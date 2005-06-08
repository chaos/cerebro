/*****************************************************************************\
 *  $Id: cerebrod_speaker.c,v 1.41 2005-06-08 15:32:01 achu Exp $
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
#include "cerebro_marshalling.h"
#include "cerebrod_heartbeat_protocol.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_error.h"

#include "cerebrod.h"
#include "cerebrod_config.h"
#include "cerebrod_data.h"
#include "cerebrod_heartbeat.h"
#include "cerebrod_speaker.h"
#include "wrappers.h"

extern struct cerebrod_config conf;
#if CEREBRO_DEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* CEREBRO_DEBUG */

/* 
 * _cerebrod_speaker_create_and_setup_socket
 *
 * Create and setup the speaker socket.  Do not use wrappers in this
 * function.  We want to give the daemon additional chances to
 * "survive" an error condition.
 *
 * Returns file descriptor on success, -1 on error
 */
static int
_cerebrod_speaker_create_and_setup_socket(void)
{
  struct sockaddr_in heartbeat_addr;
  int temp_fd;

  if ((temp_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): socket: %s", 
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
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
      
      if (setsockopt(temp_fd,
		     SOL_IP,
		     IP_MULTICAST_IF,
		     &imr,
		     sizeof(struct ip_mreqn)) < 0)
	{
	  cerebro_err_debug("%s(%s:%d): setsockopt: %s", 
                            __FILE__, __FUNCTION__, __LINE__,
                            strerror(errno));
	  return -1;
	}

      optval = 1;
      if (setsockopt(temp_fd, 
		     SOL_IP,
		     IP_MULTICAST_LOOP,
		     &optval, 
		     sizeof(optval)) < 0)
	{
	  cerebro_err_debug("%s(%s:%d): setsockopt: %s", 
                            __FILE__, __FUNCTION__, __LINE__,
                            strerror(errno));
	  return -1;
	}

      optval = conf.heartbeat_ttl;
      if (setsockopt(temp_fd,
		     SOL_IP,
		     IP_MULTICAST_TTL,
		     &optval,
		     sizeof(optval)) < 0)
	{
	  cerebro_err_debug("%s(%s:%d): setsockopt: %s", 
                            __FILE__, __FUNCTION__, __LINE__,
                            strerror(errno));
	  return -1;
	}
    }

  memset(&heartbeat_addr, '\0', sizeof(struct sockaddr_in));
  heartbeat_addr.sin_family = AF_INET;
  heartbeat_addr.sin_port = htons(conf.heartbeat_source_port);
  memcpy(&heartbeat_addr.sin_addr,
	 &conf.heartbeat_network_interface_in_addr,
	 sizeof(struct in_addr));
  if (bind(temp_fd, 
	   (struct sockaddr *)&heartbeat_addr, 
	   sizeof(struct sockaddr_in)) < 0) 
    {
      cerebro_err_debug("%s(%s:%d): bind: %s", 
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      return -1;
    }

  return temp_fd;
}

/* 
 * _cerebrod_speaker_initialize
 *
 * perform speaker initialization
 */
static void
_cerebrod_speaker_initialize(void)
{
  char nodename[CEREBRO_MAXNODENAMELEN+1];
  unsigned int seed;;
  int i, len;

  seed = Time(NULL);

  /* If an entire cluster is re-booted at the same time, each cluster
   * node could potentially be seeded with the same time.  In order to
   * avoid this, we'll add the cluster nodename to the seed to give
   * every cluster node a constant different offset.
   */
  memset(nodename, '\0', CEREBRO_MAXNODENAMELEN+1);
  cerebrod_get_nodename(nodename, CEREBRO_MAXNODENAMELEN+1);
  
  len = strlen(nodename);
  for (i = 0; i < len; i++)
    seed += (int)nodename[i];

  srand(seed);
}

/*
 * _cerebrod_heartbeat_create
 *
 * construct a heartbeat packet
 */
static struct cerebrod_heartbeat *
_cerebrod_heartbeat_create(int *heartbeat_len)
{
  struct cerebrod_heartbeat *hb = NULL;
  struct cerebrod_heartbeat_metric *hd = NULL;
  u_int32_t boottime;

  assert(heartbeat_len);

  *heartbeat_len = 0;

  /* XXX needs cleaning up */

  hb = Malloc(sizeof(struct cerebrod_heartbeat));

  hb->version = CEREBROD_HEARTBEAT_PROTOCOL_VERSION;
  cerebrod_get_nodename(hb->nodename, CEREBRO_MAXNODENAMELEN);

  /* XXX need clean way to determine this */
  hb->metrics_len = 1;

  hb->metrics = Malloc(sizeof(struct cerebrod_heartbeat_metric *)*(hb->metrics_len + 1));
  memset(hb->metrics, '\0', sizeof(struct cerebrod_heartbeat_metric *)*(hb->metrics_len + 1));

  *heartbeat_len += CEREBROD_HEARTBEAT_HEADER_LEN;

  /* 
   * Store boottime
   */

  hd = Malloc(sizeof(struct cerebrod_heartbeat_metric));
  memset(hd, '\0', sizeof(struct cerebrod_heartbeat_metric));

  /* need not overflow */
  /* XXX cleanup */
  strncpy(hd->metric_name, "boottime", CEREBRO_METRIC_NAME_MAXLEN);
  /* XXX need a function to calculate this here instead */
  hd->metric_value_type = CEREBRO_METRIC_VALUE_TYPE_UNSIGNED_INT32;
  hd->metric_value_len = sizeof(u_int32_t);
  
  hd->metric_value = Malloc(hd->metric_value_len);
  boottime = cerebrod_get_boottime();
  memcpy(hd->metric_value, &boottime, hd->metric_value_len);

  *heartbeat_len += CEREBROD_HEARTBEAT_METRIC_HEADER_LEN + hd->metric_value_len;

  hb->metrics[0] = hd;

  /* 
   * store something else later ??
   */

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
  int i, len, count = 0;
 
  assert(hb);
  assert(buf);
  assert(buflen >= CEREBROD_HEARTBEAT_HEADER_LEN);
  
  memset(buf, '\0', buflen);
  if ((len = _cerebro_marshall_int32(hb->version,
                                     buf + count,
                                     buflen - count)) < 0)
    cerebro_err_exit("%s(%s:%d): _cerebro_marshall_int32",
                     __FILE__, __FUNCTION__, __LINE__);
  count += len;
  
  if ((len = _cerebro_marshall_buffer(hb->nodename,
                                      sizeof(hb->nodename),
                                      buf + count,
                                      buflen - count)) < 0)
    cerebro_err_exit("%s(%s:%d): _cerebro_marshall_buffer",
                     __FILE__, __FUNCTION__, __LINE__);
  count += len;
  
  if ((len = _cerebro_marshall_unsigned_int32(hb->metrics_len,
                                              buf + count,
                                              buflen - count)) < 0)
    cerebro_err_exit("%s(%s:%d): _cerebro_marshall_unsigned_int32",
                     __FILE__, __FUNCTION__, __LINE__);
  count += len;
  
  if (hb->metrics_len)
    {
      for (i = 0; i < hb->metrics_len; i++)
        {
          if ((len = _cerebro_marshall_buffer(hb->metrics[i]->metric_name,
                                              sizeof(hb->metrics[i]->metric_name),
                                              buf + count,
                                              buflen - count)) < 0)
            cerebro_err_exit("%s(%s:%d): _cerebro_marshall_buffer",
                             __FILE__, __FUNCTION__, __LINE__);
          count += len;
          
          if ((len = _cerebro_marshall_unsigned_int32(hb->metrics[i]->metric_value_type,
                                                      buf + count,
                                                      buflen - count)) < 0)
            cerebro_err_exit("%s(%s:%d): _cerebro_marshall_unsigned_int32",
                             __FILE__, __FUNCTION__, __LINE__);
          count += len;
          
          if ((len = _cerebro_marshall_unsigned_int32(hb->metrics[i]->metric_value_len,
                                                      buf + count,
                                                      buflen - count)) < 0)
            cerebro_err_exit("%s(%s:%d): _cerebro_marshall_unsigned_int32",
                             __FILE__, __FUNCTION__, __LINE__);
          count += len;
          
          switch(hb->metrics[i]->metric_value_type)
            {
            case CEREBRO_METRIC_VALUE_TYPE_NONE:
              cerebro_err_debug("%s(%s:%d): packet metric_value_len > 0 for metric_value_type NONE",
                                __FILE__, __FUNCTION__, __LINE__);
              break;
            case CEREBRO_METRIC_VALUE_TYPE_INT32:
              if ((len = _cerebro_marshall_int32(*((int32_t *)hb->metrics[i]->metric_value),
                                                 buf + count,
                                                 buflen - count)) < 0)
                cerebro_err_exit("%s(%s:%d): _cerebro_marshall_int32",
                                 __FILE__, __FUNCTION__, __LINE__);
              count += len;
              break;
            case CEREBRO_METRIC_VALUE_TYPE_UNSIGNED_INT32:
              if ((len = _cerebro_marshall_unsigned_int32(*((u_int32_t *)hb->metrics[i]->metric_value),
                                                          buf + count,
                                                          buflen - count)) < 0)
                cerebro_err_exit("%s(%s:%d): _cerebro_marshall_unsigned_int32",
                                 __FILE__, __FUNCTION__, __LINE__);
              count += len;
              break;
            case CEREBRO_METRIC_VALUE_TYPE_FLOAT:
              if ((len = _cerebro_marshall_float(*((float *)hb->metrics[i]->metric_value),
                                                 buf + count,
                                                 buflen - count)) < 0)
                cerebro_err_exit("%s(%s:%d): _cerebro_marshall_float",
                                 __FILE__, __FUNCTION__, __LINE__);
              count += len;
              break;
            case CEREBRO_METRIC_VALUE_TYPE_DOUBLE:
              if ((len = _cerebro_marshall_double(*((double *)hb->metrics[i]->metric_value),
                                                  buf + count,
                                                  buflen - count)) < 0)
                cerebro_err_exit("%s(%s:%d): _cerebro_marshall_double",
                                 __FILE__, __FUNCTION__, __LINE__);
              count += len;
              break;
            case CEREBRO_METRIC_VALUE_TYPE_STRING:
            case CEREBRO_METRIC_VALUE_TYPE_RAW:
              if ((len = _cerebro_marshall_buffer((char *)hb->metrics[i]->metric_value,
                                                  hb->metrics[i]->metric_value_len,
                                                  buf + count,
                                                  buflen - count)) < 0)
                cerebro_err_exit("%s(%s:%d): _cerebro_marshall_buffer",
                                 __FILE__, __FUNCTION__, __LINE__);
              count += len;
              break;
            default:
              cerebro_err_exit("%s(%s:%d): invalid metric type: %d",
                               __FILE__, __FUNCTION__, __LINE__,
                               hb->metrics[i]->metric_value_type);
              break;
            }
        }
    }
  
  return count;
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
      time_t t;
      struct tm tm;
      char strbuf[CEREBROD_STRING_BUFLEN];

      t = Time(NULL);
      Localtime_r(&t, &tm);
      strftime(strbuf, CEREBROD_STRING_BUFLEN, "%H:%M:%S", &tm);

      Pthread_mutex_lock(&debug_output_mutex);
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Sending Heartbeat: %s\n", strbuf);     
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
  if ((fd = _cerebrod_speaker_create_and_setup_socket()) < 0)
    cerebro_err_exit("%s(%s:%d): fd setup failed",
                     __FILE__, __FUNCTION__, __LINE__);

  while (1)
    {
      struct sockaddr_in heartbeat_destination_addr;
      struct cerebrod_heartbeat* hb;
      int send_len, heartbeat_len, sleep_time, buflen;
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
            {
              /* For errnos EINVAL, EBADF, ENODEV, assume the device has
               * been temporarily brought down then back up.  For example,
               * this can occur if the administrator runs
               * '/etc/init.d/network restart'.  We just need to re-setup
               * the socket.
               * 
               * If fd < 0, the network device just isn't back up yet from
               * the previous time we got an errno EINVAL, EBADF, or
               * ENODEV.
               */
              if (errno == EINVAL 
                  || errno == EBADF
                  || errno == ENODEV
                  || fd < 0)
                {
                  if (!(fd < 0))
                    close(fd);	/* no-wrapper, make best effort */
                  
                  if ((fd = _cerebrod_speaker_create_and_setup_socket()) < 0)
                    cerebro_err_debug("%s(%s:%d): error re-initializing socket",
                                      __FILE__, __FUNCTION__, __LINE__);
                  else
                    cerebro_err_debug("%s(%s:%d): success re-initializing "
                                      "socket",
                                      __FILE__, __FUNCTION__, __LINE__);
                }
              else if (errno == EINTR)
                cerebro_err_debug("%s(%s:%d): sendto: %s", 
                                  __FILE__, __FUNCTION__, __LINE__,
                                  strerror(errno));
              else
                cerebro_err_exit("%s(%s:%d): sendto: %s", 
                                 __FILE__, __FUNCTION__, __LINE__,
                                 strerror(errno));
            }
          else
            cerebro_err_debug("%s(%s:%d): sendto: invalid bytes sent: %d", 
                              __FILE__, __FUNCTION__, __LINE__, send_len);
        }

      cerebrod_heartbeat_destroy(hb);
      Free(buf);
      sleep(sleep_time);
    }

  return NULL;			/* NOT REACHED */
}
