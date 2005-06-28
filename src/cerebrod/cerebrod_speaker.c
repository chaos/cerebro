/*****************************************************************************\
 *  $Id: cerebrod_speaker.c,v 1.65 2005-06-28 00:32:12 achu Exp $
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
 * _speaker_setup_socketet
 *
 * Create and setup the speaker socket.  Do not use wrappers in this
 * function.  We want to give the daemon additional chances to
 * "survive" an error condition.
 *
 * Returns file descriptor on success, -1 on error
 */
static int
_speaker_setup_socketet(void)
{
  struct sockaddr_in addr;
  int fd;

  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      CEREBRO_DBG(("socket: %s", strerror(errno)));
      return -1;
    }

  if (conf.multicast)
    {
      /* XXX: Probably lots of portability problems here */
      struct ip_mreqn imr;
      int optval;

      memset(&imr, '\0', sizeof(struct ip_mreqn));
      memcpy(&imr.imr_multiaddr, 
	     &conf.heartbeat_destination_ip_in_addr,
	     sizeof(struct in_addr));
      memcpy(&imr.imr_address, 
	     &conf.heartbeat_network_interface_in_addr,
	     sizeof(struct in_addr));
      imr.imr_ifindex = conf.heartbeat_interface_index;
      
      if (setsockopt(fd,
		     SOL_IP,
		     IP_MULTICAST_IF,
		     &imr,
		     sizeof(struct ip_mreqn)) < 0)
	{
	  CEREBRO_DBG(("setsockopt: %s", strerror(errno)));
	  return -1;
	}

      optval = 1;
      if (setsockopt(fd, 
		     SOL_IP,
		     IP_MULTICAST_LOOP,
		     &optval, 
		     sizeof(optval)) < 0)
	{
	  CEREBRO_DBG(("setsockopt: %s", strerror(errno)));
	  return -1;
	}

      optval = conf.heartbeat_ttl;
      if (setsockopt(fd,
		     SOL_IP,
		     IP_MULTICAST_TTL,
		     &optval,
		     sizeof(optval)) < 0)
	{
	  CEREBRO_DBG(("setsockopt: %s", strerror(errno)));
	  return -1;
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
      return -1;
    }

  return fd;
}

/* 
 * _cerebrod_speaker_initialize
 *
 * perform speaker initialization
 */
static void
_cerebrod_speaker_initialize(void)
{
  unsigned int seed;
  struct timeval tv;
  int i, len;

  /* 
   * Setup Nodename
   */

  memset(cerebrod_nodename, '\0', CEREBRO_MAX_NODENAME_LEN+1);
  Gethostname(cerebrod_nodename, CEREBRO_MAX_NODENAME_LEN);

  /* 
   * Seed random number generator
   */

  Gettimeofday(&tv, NULL);
  seed = tv.tv_sec;

  /* If an entire cluster is re-booted at the same time, each cluster
   * node could potentially be seeded with the same time.  In order to
   * avoid this, we'll add the cluster nodename to the seed to give
   * every cluster node a constant different offset.
   */
  len = strlen(cerebrod_nodename);
  for (i = 0; i < len; i++)
    seed += (int)cerebrod_nodename[i];

  srand(seed);

  cerebrod_speaker_data_initialize();
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
 * _cerebrod_heartbeat_marshall
 *
 * marshall contents of a heartbeat packet.
 *
 * Returns length written to buffer on success, -1 on error
 */
int
_cerebrod_heartbeat_marshall(struct cerebrod_heartbeat *hb,
			     char *buf,
			     unsigned int buflen)
{
  int i, len = 0;
 
  assert(hb);
  assert(buf);
  assert(buflen >= CEREBROD_HEARTBEAT_HEADER_LEN);
  
  memset(buf, '\0', buflen);
  len += Marshall_int32(hb->version, buf + len, buflen - len);
  len += Marshall_buffer(hb->nodename, 
                         sizeof(hb->nodename),
                         buf + len,
                         buflen - len);
  len += Marshall_u_int32(hb->metrics_len, buf + len, buflen - len);
  
  if (hb->metrics_len)
    {
      for (i = 0; i < hb->metrics_len; i++)
        {
          len += Marshall_buffer(hb->metrics[i]->metric_name,
                                 sizeof(hb->metrics[i]->metric_name),
                                 buf + len,
                                 buflen - len);
          
          len += Marshall_u_int32(hb->metrics[i]->metric_value_type,
                                  buf + len,
                                  buflen - len);
          
          len += Marshall_u_int32(hb->metrics[i]->metric_value_len,
                                  buf + len,
                                  buflen - len);
          
          switch(hb->metrics[i]->metric_value_type)
            {
            case CEREBRO_METRIC_VALUE_TYPE_NONE:
              CEREBRO_DBG(("metric value len > 0 for type NONE"));
              break;
            case CEREBRO_METRIC_VALUE_TYPE_INT32:
              len += Marshall_int32(*((int32_t *)hb->metrics[i]->metric_value),
                                    buf + len,
                                    buflen - len);
              break;
            case CEREBRO_METRIC_VALUE_TYPE_U_INT32:
              len += Marshall_u_int32(*((u_int32_t *)hb->metrics[i]->metric_value),
                                      buf + len,
                                      buflen - len);
              break;
            case CEREBRO_METRIC_VALUE_TYPE_FLOAT:
              len += Marshall_float(*((float *)hb->metrics[i]->metric_value),
                                    buf + len,
                                    buflen - len);
              break;
            case CEREBRO_METRIC_VALUE_TYPE_DOUBLE:
              len += Marshall_double(*((double *)hb->metrics[i]->metric_value),
                                     buf + len,
                                     buflen - len);
              break;
            case CEREBRO_METRIC_VALUE_TYPE_STRING:
              len += Marshall_buffer((char *)hb->metrics[i]->metric_value,
                                     hb->metrics[i]->metric_value_len,
                                     buf + len,
                                     buflen - len);
              break;
            default:
              CEREBRO_EXIT(("invalid type %d", hb->metrics[i]->metric_value_type));
              break;
            }
        }
    }
  
  return len;
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

  if (conf.debug && conf.speak_debug)
    {
      Pthread_mutex_lock(&debug_output_mutex);
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Sending Heartbeat\n");     
      fprintf(stderr, "* -----------------------\n");
      cerebrod_heartbeat_dump(hb);
      fprintf(stderr, "**************************************\n");
      Pthread_mutex_unlock(&debug_output_mutex);
    }
#endif /* CEREBRO_DEBUG */
}

void *
cerebrod_speaker(void *arg)
{
  int fd;

  _cerebrod_speaker_initialize();
  if ((fd = _speaker_setup_socketet()) < 0)
    CEREBRO_EXIT(("fd setup failed"));

  while (1)
    {
      struct sockaddr_in heartbeat_destination_addr;
      struct cerebrod_heartbeat* hb;
      int send_len, heartbeat_len, sleep_time;
      unsigned int buflen;
      char *buf = NULL;

      /* Algorithm from srand(3) manpage */
      if (conf.heartbeat_frequency_ranged)
	sleep_time = conf.heartbeat_frequency_min + ((((double)(conf.heartbeat_frequency_max - conf.heartbeat_frequency_min))*rand())/(RAND_MAX+1.0));
      else
	sleep_time = conf.heartbeat_frequency_min;

      hb = _cerebrod_heartbeat_create(&buflen);
  
      buf = Malloc(buflen + 1);

      heartbeat_len = _cerebrod_heartbeat_marshall(hb, 
						   buf, 
						   buflen);

      _cerebrod_heartbeat_dump(hb);
      
      memset(&heartbeat_destination_addr, '\0', sizeof(struct sockaddr_in));
      heartbeat_destination_addr.sin_family = AF_INET;
      heartbeat_destination_addr.sin_port = htons(conf.heartbeat_destination_port);
      memcpy(&heartbeat_destination_addr.sin_addr,
             &conf.heartbeat_destination_ip_in_addr,
             sizeof(struct in_addr));

      if ((send_len = sendto(fd, 
			     buf, 
			     heartbeat_len, 
			     0, 
			     (struct sockaddr *)&heartbeat_destination_addr,
			     sizeof(struct sockaddr_in))) != heartbeat_len)
        {
          if (send_len < 0)
            fd = cerebrod_reinitialize_socket(fd,
                                              _speaker_setup_socketet,
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
