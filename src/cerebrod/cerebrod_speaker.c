/*****************************************************************************\
 *  $Id: cerebrod_speaker.c,v 1.73 2005-07-11 20:35:34 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <errno.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebrod_heartbeat_protocol.h"

#include "cerebrod_config.h"
#include "cerebrod_heartbeat.h"
#include "cerebrod_speaker.h"
#include "cerebrod_speaker_data.h"
#include "cerebrod_util.h"

#include "debug.h"
#include "metric_module.h"
#include "wrappers.h"

extern struct cerebrod_config conf;
#if CEREBRO_DEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* CEREBRO_DEBUG */

/*
 * cerebrod_nodename
 *
 * cached system nodename
 */
static char cerebrod_nodename[CEREBRO_MAX_NODENAME_LEN+1];

/*
 * speaker_init
 * speaker_init_cond
 * speaker_init_lock
 *
 * variables for synchronizing initialization between different pthreads
 * and signaling when it is complete
 */
int speaker_init = 0;
pthread_cond_t speaker_init_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t speaker_init_lock = PTHREAD_MUTEX_INITIALIZER;

/* 
 * _speaker_socket_setup
 *
 * Create and setup the speaker socket.  Do not use wrappers in this
 * function.  We want to give the daemon additional chances to
 * "survive" an error condition.
 *
 * Returns file descriptor on success, -1 on error
 */
static int
_speaker_socket_setup(void)
{
  struct sockaddr_in addr;
  int fd;

  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      CEREBRO_DBG(("socket: %s", strerror(errno)));
      goto cleanup;
    }

  if (conf.multicast)
    {
      /* XXX: Probably lots of portability problems here */
      struct ip_mreqn imr;
      unsigned int optlen;
      int optval;

      memset(&imr, '\0', sizeof(struct ip_mreqn));
      memcpy(&imr.imr_multiaddr, 
	     &conf.heartbeat_destination_ip_in_addr,
	     sizeof(struct in_addr));
      memcpy(&imr.imr_address, 
	     &conf.heartbeat_network_interface_in_addr,
	     sizeof(struct in_addr));
      imr.imr_ifindex = conf.heartbeat_interface_index;
      
      optlen = sizeof(struct ip_mreqn);
      if (setsockopt(fd, SOL_IP, IP_MULTICAST_IF, &imr, optlen) < 0)
	{
	  CEREBRO_DBG(("setsockopt: %s", strerror(errno)));
          goto cleanup;
	}

      optval = 1;
      optlen = sizeof(optval);
      if (setsockopt(fd, SOL_IP, IP_MULTICAST_LOOP, &optval, optlen) < 0)
	{
	  CEREBRO_DBG(("setsockopt: %s", strerror(errno)));
          goto cleanup;
	}

      optval = conf.heartbeat_ttl;
      optlen = sizeof(optval);
      if (setsockopt(fd, SOL_IP, IP_MULTICAST_TTL, &optval, optlen) < 0)
	{
	  CEREBRO_DBG(("setsockopt: %s", strerror(errno)));
          goto cleanup;
	}
    }

  memset(&addr, '\0', sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(conf.heartbeat_source_port);
  memcpy(&addr.sin_addr,
	 &conf.heartbeat_network_interface_in_addr,
	 sizeof(struct in_addr));
  if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0) 
    {
      CEREBRO_DBG(("bind: %s", strerror(errno)));
      goto cleanup;
    }

  return fd;

 cleanup:
  close(fd);
  return -1;
}

/* 
 * _speaker_initialize
 *
 * perform speaker initialization
 */
static void
_speaker_initialize(void)
{
  unsigned int seed;
  struct timeval tv;
  int i, len;

  Pthread_mutex_lock(&speaker_init_lock);
  if (speaker_init)
    goto out;

  /* Cache Nodename */
  memset(cerebrod_nodename, '\0', CEREBRO_MAX_NODENAME_LEN+1);
  Gethostname(cerebrod_nodename, CEREBRO_MAX_NODENAME_LEN);

  /* Seed random number generator */
  Gettimeofday(&tv, NULL);
  seed = tv.tv_sec;

  /* If a cluster is re-booted at the same time, each cluster node
   * could be seeded with the same time.  In order to avoid this,
   * we'll add the cluster nodename to the seed to give every cluster
   * node atleast a constant different offset.
   */
  len = strlen(cerebrod_nodename);
  for (i = 0; i < len; i++)
    seed += (int)cerebrod_nodename[i];

  srand(seed);

  cerebrod_speaker_data_initialize();

  speaker_init++;
  Pthread_cond_signal(&speaker_init_cond);
 out:
  Pthread_mutex_unlock(&speaker_init_lock);
}

/*
 * _cerebrod_heartbeat_create
 *
 * construct a heartbeat packet
 */
static struct cerebrod_heartbeat *
_cerebrod_heartbeat_create(unsigned int *heartbeat_len)
{
  struct cerebrod_heartbeat *hb = NULL;

  assert(heartbeat_len);

  *heartbeat_len = 0;

  hb = Malloc(sizeof(struct cerebrod_heartbeat));

  hb->version = CEREBROD_HEARTBEAT_PROTOCOL_VERSION;
  memcpy(hb->nodename, cerebrod_nodename, CEREBRO_MAX_NODENAME_LEN);
  *heartbeat_len += CEREBROD_HEARTBEAT_HEADER_LEN;

  cerebrod_speaker_data_get_metric_data(hb, heartbeat_len);

  return hb;
}

/*
 * _heartbeat_marshall
 *
 * marshall contents of a heartbeat packet.
 *
 * Returns length written to buffer on success, -1 on error
 */
int
_heartbeat_marshall(struct cerebrod_heartbeat *hb, 
                    char *buf, 
                    unsigned int buflen)
{
  char *bufPtr;
  int i, bufPtrlen, c = 0;
 
  assert(hb && buf && buflen >= CEREBROD_HEARTBEAT_HEADER_LEN);
  
  bufPtr = hb->nodename;
  bufPtrlen = sizeof(hb->nodename);
  memset(buf, '\0', buflen);
  c += Marshall_int32(hb->version, buf + c, buflen - c);
  c += Marshall_buffer(bufPtr, bufPtrlen, buf + c, buflen - c);
  c += Marshall_u_int32(hb->metrics_len, buf + c, buflen - c);
  
  if (!hb->metrics_len)
    return c;

  for (i = 0; i < hb->metrics_len; i++)
    {
      char *mname;
      u_int32_t mtype, mlen;
      void *mvalue;
      int mnamelen;

      mname = hb->metrics[i]->metric_name;
      mnamelen = sizeof(hb->metrics[i]->metric_name);
      mtype = hb->metrics[i]->metric_value_type;
      mlen = hb->metrics[i]->metric_value_len;
      mvalue = hb->metrics[i]->metric_value;

      c += Marshall_buffer(mname, mnamelen, buf + c, buflen - c);
      c += Marshall_u_int32(mtype, buf + c, buflen - c);
      c += Marshall_u_int32(mlen, buf + c, buflen - c);
      
      if (!mlen)
        continue;
      
      if (mtype == CEREBRO_METRIC_VALUE_TYPE_INT32)
        c += Marshall_int32(*((int32_t *)mvalue), buf + c, buflen - c);
      else if (mtype == CEREBRO_METRIC_VALUE_TYPE_U_INT32)
        c += Marshall_u_int32(*((u_int32_t *)mvalue), buf + c, buflen - c);
      else if (mtype == CEREBRO_METRIC_VALUE_TYPE_FLOAT)
        c += Marshall_float(*((float *)mvalue), buf + c, buflen - c);
      else if (mtype == CEREBRO_METRIC_VALUE_TYPE_DOUBLE)
        c += Marshall_double(*((double *)mvalue), buf + c, buflen - c);
      else if (mtype == CEREBRO_METRIC_VALUE_TYPE_STRING)
        c += Marshall_buffer((char *)mvalue, mlen, buf + c, buflen - c);
      else
        CEREBRO_EXIT(("invalid type %d", mtype));
    }
  
  return c;
}

/* 
 * _cerebrod_heartbeat_dump
 *
 * Dump contents of heartbeat packet
 */
static void
_cerebrod_heartbeat_dump(struct cerebrod_heartbeat *hb)
{
#if CEREBRO_DEBUG
  assert(hb);

  if (!(conf.debug && conf.speak_debug))
    return;

  Pthread_mutex_lock(&debug_output_mutex);
  fprintf(stderr, "**************************************\n");
  fprintf(stderr, "* Sending Heartbeat\n");     
  fprintf(stderr, "* -----------------------\n");
  cerebrod_heartbeat_dump(hb);
  fprintf(stderr, "**************************************\n");
  Pthread_mutex_unlock(&debug_output_mutex);
#endif /* CEREBRO_DEBUG */
}

void *
cerebrod_speaker(void *arg)
{
  int speaker_fd;

  _speaker_initialize();
  if ((speaker_fd = _speaker_socket_setup()) < 0)
    CEREBRO_EXIT(("speaker_fd setup failed"));

  while (1)
    {
      struct cerebrod_heartbeat* hb;
      struct sockaddr *addr;
      struct sockaddr_in hbaddr;
      int rv, hblen, sleep_time;
      unsigned int buflen, addrlen;
      char *buf = NULL;

      /* Algorithm from srand(3) manpage */
      if (conf.heartbeat_frequency_ranged)
	sleep_time = conf.heartbeat_frequency_min + ((((double)(conf.heartbeat_frequency_max - conf.heartbeat_frequency_min))*rand())/(RAND_MAX+1.0));
      else
	sleep_time = conf.heartbeat_frequency_min;

      hb = _cerebrod_heartbeat_create(&buflen);
  
      buf = Malloc(buflen + 1);

      hblen = _heartbeat_marshall(hb, buf, buflen);

      _cerebrod_heartbeat_dump(hb);
      
      memset(&hbaddr, '\0', sizeof(struct sockaddr_in));
      hbaddr.sin_family = AF_INET;
      hbaddr.sin_port = htons(conf.heartbeat_destination_port);
      memcpy(&hbaddr.sin_addr, 
             &conf.heartbeat_destination_ip_in_addr, 
             sizeof(struct in_addr));

      addr = (struct sockaddr *)&hbaddr;
      addrlen = sizeof(struct sockaddr_in);
      if ((rv = sendto(speaker_fd, buf, hblen, 0, addr, addrlen)) != hblen)
        {
          if (rv < 0)
            speaker_fd = cerebrod_reinit_socket(speaker_fd, 
                                                _speaker_socket_setup, 
                                                "speaker: sendto");
          else
            CEREBRO_DBG(("sendto: invalid bytes sent"));
        }

      cerebrod_heartbeat_destroy(hb);
      Free(buf);
      sleep(sleep_time);
    }

  return NULL;			/* NOT REACHED */
}
