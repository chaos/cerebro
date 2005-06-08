/*****************************************************************************\
 *  $Id: cerebrod_listener.c,v 1.76 2005-06-08 16:35:14 achu Exp $
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
#include "cerebro_module.h"
#include "cerebrod_heartbeat_protocol.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_error.h"

#include "cerebrod.h"
#include "cerebrod_config.h"
#include "cerebrod_heartbeat.h"
#include "cerebrod_listener.h"
#include "cerebrod_metric.h"
#include "cerebrod_node_data.h"
#include "wrappers.h"

extern struct cerebrod_config conf;
#if CEREBRO_DEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* CEREBRO_DEBUG */

/* 
 * cerebrod_listener_initialization_complete
 * cerebrod_listener_initialization_complete_cond
 * cerebrod_listener_initialization_complete_lock
 *
 * variables for synchronizing initialization between different pthreads
 * and signaling when it is complete
 */
int cerebrod_listener_initialization_complete = 0;
pthread_cond_t cerebrod_listener_initialization_complete_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t cerebrod_listener_initialization_complete_lock = PTHREAD_MUTEX_INITIALIZER;

/* 
 * listener_fd
 * listener_fd_lock
 *
 * listener file descriptor and lock to protect concurrent access
 */
int listener_fd = 0;
pthread_mutex_t listener_fd_lock = PTHREAD_MUTEX_INITIALIZER;

/* 
 * packet_buflen_max
 *
 * Maximum packet size discovered
 */
int packet_buflen_max = CEREBRO_PACKET_BUFLEN;
pthread_mutex_t packet_buflen_max_lock = PTHREAD_MUTEX_INITIALIZER;


/* 
 * _cerebrod_listener_create_and_setup_socket
 *
 * Create and setup the listener socket.  Do not use wrappers in this
 * function.  We want to give the daemon additional chances to
 * "survive" an error condition.
 *
 * Returns file descriptor on success, -1 on error
 */
static int
_cerebrod_listener_create_and_setup_socket(void)
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
		     IP_ADD_MEMBERSHIP,
		     &imr,
		     sizeof(struct ip_mreqn)) < 0)
	{
	  cerebro_err_debug("%s(%s:%d): setsockopt: %s",
                            __FILE__, __FUNCTION__, __LINE__,
                            strerror(errno));
	  return -1;
	}
    }

  /* Configuration checks ensure destination ip is on this machine if
   * it is a non-multicast address.
   */
  memset(&heartbeat_addr, '\0', sizeof(struct sockaddr_in));
  heartbeat_addr.sin_family = AF_INET;
  heartbeat_addr.sin_port = htons(conf.heartbeat_destination_port);
  memcpy(&heartbeat_addr.sin_addr,
         &conf.heartbeat_destination_ip_in_addr,
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
 * _cerebrod_listener_initialize
 *
 * perform listener initialization
 */
static void
_cerebrod_listener_initialize(void)
{
  Pthread_mutex_lock(&cerebrod_listener_initialization_complete_lock);
  if (cerebrod_listener_initialization_complete)
    goto out;

  Pthread_mutex_lock(&listener_fd_lock);
  if ((listener_fd = _cerebrod_listener_create_and_setup_socket()) < 0)
    cerebro_err_exit("%s(%s:%d): listener_fd setup failed",
                     __FILE__, __FUNCTION__, __LINE__);
  Pthread_mutex_unlock(&listener_fd_lock);
  
  cerebrod_listener_initialization_complete++;
  Pthread_cond_signal(&cerebrod_listener_initialization_complete_cond);
 out:
  Pthread_mutex_unlock(&cerebrod_listener_initialization_complete_lock);
}

/*
 * _cerebrod_heartbeat_unmarshall
 *
 * unmarshall contents of a heartbeat packet buffer and
 * return in an allocated heartbeat
 *
 * Returns heartbeat data on success, NULL on error
 */
static struct cerebrod_heartbeat *
_cerebrod_heartbeat_unmarshall(const char *buf, unsigned int buflen)
{
  struct cerebrod_heartbeat *hb = NULL;
  struct cerebrod_heartbeat_metric *hd = NULL;
  int i, len, count = 0;

  assert(buf);
  
  hb = Malloc(sizeof(struct cerebrod_heartbeat));

  memset(hb, '\0', sizeof(struct cerebrod_heartbeat));
  
  if ((len = _cerebro_unmarshall_int32(&(hb->version),
                                       buf + count,
                                       buflen - count)) < 0)
    cerebro_err_exit("%s(%s:%d): _cerebro_unmarshall_int32",
		     __FILE__, __FUNCTION__, __LINE__);
  if (!len)
    {
      cerebro_err_debug("%s(%s:%d): packet buffer length invalid",
                        __FILE__, __FUNCTION__, __LINE__);
      goto cleanup;
    }
  
  count += len;

  if ((len = _cerebro_unmarshall_buffer(hb->nodename,
                                        sizeof(hb->nodename),
                                        buf + count,
                                        buflen - count)) < 0)
    cerebro_err_exit("%s(%s:%d): _cerebro_unmarshall_buffer",
                     __FILE__, __FUNCTION__, __LINE__);
  if (!len)
    {
      cerebro_err_debug("%s(%s:%d): packet buffer length invalid",
                        __FILE__, __FUNCTION__, __LINE__);
      goto cleanup;
    }
  
  count += len;
  
  if ((len = _cerebro_unmarshall_unsigned_int32(&(hb->metrics_len),
                                                buf + count,
                                                buflen - count)) < 0)
    cerebro_err_exit("%s(%s:%d): _cerebro_unmarshall_unsigned_int32",
                     __FILE__, __FUNCTION__, __LINE__);
  if (!len)
    {
      cerebro_err_debug("%s(%s:%d): packet buffer length invalid",
                        __FILE__, __FUNCTION__, __LINE__);
      goto cleanup;
    }

  count += len;
  
  if (hb->metrics_len)
    {
      hb->metrics = Malloc(sizeof(struct cerebrod_heartbeat_metric *)*(hb->metrics_len + 1));
      memset(hb->metrics, '\0', sizeof(struct cerebrod_heartbeat_metric *)*(hb->metrics_len + 1));
      
      for (i = 0; i < hb->metrics_len; i++)
        {
          hd = Malloc(sizeof(struct cerebrod_heartbeat_metric));
          memset(hd, '\0', sizeof(struct cerebrod_heartbeat_metric));
          
          if ((len = _cerebro_unmarshall_buffer(hd->metric_name,
                                                sizeof(hd->metric_name),
                                                buf + count,
                                                buflen - count)) < 0)
            cerebro_err_exit("%s(%s:%d): _cerebro_unmarshall_buffer",
                             __FILE__, __FUNCTION__, __LINE__);
          if (!len)
            {
              cerebro_err_debug("%s(%s:%d): packet buffer length invalid",
                                __FILE__, __FUNCTION__, __LINE__);
              goto cleanup;
            }
          
          count += len;
          
          if ((len = _cerebro_unmarshall_unsigned_int32(&(hd->metric_value_type),
                                                        buf + count,
                                                        buflen - count)) < 0)
            cerebro_err_exit("%s(%s:%d): _cerebro_unmarshall_unsigned_int32",
                             __FILE__, __FUNCTION__, __LINE__);
          if (!len)
            {
              cerebro_err_debug("%s(%s:%d): packet buffer length invalid",
                                __FILE__, __FUNCTION__, __LINE__);
              goto cleanup;
            }
          
          count += len;
          
          if ((len = _cerebro_unmarshall_unsigned_int32(&(hd->metric_value_len),
                                                        buf + count,
                                                        buflen - count)) < 0)
            cerebro_err_exit("%s(%s:%d): _cerebro_unmarshall_unsigned_int32",
                             __FILE__, __FUNCTION__, __LINE__);
          if (!len)
            {
              cerebro_err_debug("%s(%s:%d): packet buffer length invalid",
                                __FILE__, __FUNCTION__, __LINE__);
              goto cleanup;
            }
      
          count += len;
          
          if (hd->metric_value_len)
            {
              hd->metric_value = Malloc(hd->metric_value_len);
              
              switch(hd->metric_value_type)
                {
                case CEREBRO_METRIC_VALUE_TYPE_NONE:
                  cerebro_err_debug("%s(%s:%d): packet metric_value_len > 0 "
                                    "for metric_value_type NONE",
                                    __FILE__, __FUNCTION__, __LINE__);
                  break;
                case CEREBRO_METRIC_VALUE_TYPE_INT32:
                  if ((len = _cerebro_unmarshall_int32((int32_t *)hd->metric_value,
                                                       buf + count,
                                                       buflen - count)) < 0)
                    cerebro_err_exit("%s(%s:%d): _cerebro_unmarshall_int32",
                                     __FILE__, __FUNCTION__, __LINE__);
                  if (!len)
                    {
                      cerebro_err_debug("%s(%s:%d): packet buffer length invalid",
                                        __FILE__, __FUNCTION__, __LINE__);
                      goto cleanup;
                    }
                  count += len;
                  break;
                case CEREBRO_METRIC_VALUE_TYPE_UNSIGNED_INT32:
                  if ((len = _cerebro_unmarshall_unsigned_int32((u_int32_t *)hd->metric_value,
                                                                buf + count,
                                                                buflen - count)) < 0)
                    cerebro_err_exit("%s(%s:%d): _cerebro_unmarshall_unsigned_int32",
                                     __FILE__, __FUNCTION__, __LINE__);
                  if (!len)
                    {
                      cerebro_err_debug("%s(%s:%d): packet buffer length invalid",
                                        __FILE__, __FUNCTION__, __LINE__);
                      goto cleanup;
                    }
                  count += len;
                  break;
                case CEREBRO_METRIC_VALUE_TYPE_FLOAT:
                  if ((len = _cerebro_unmarshall_float((float *)hd->metric_value,
                                                       buf + count,
                                                       buflen - count)) < 0)
                    cerebro_err_exit("%s(%s:%d): _cerebro_unmarshall_float",
                                     __FILE__, __FUNCTION__, __LINE__);
                  if (!len)
                    {
                      cerebro_err_debug("%s(%s:%d): packet buffer length invalid",
                                        __FILE__, __FUNCTION__, __LINE__);
                      goto cleanup;
                    }
                  count += len;
                  break;
                case CEREBRO_METRIC_VALUE_TYPE_DOUBLE:
                  if ((len = _cerebro_unmarshall_double((double *)hd->metric_value,
                                                        buf + count,
                                                        buflen - count)) < 0)
                    cerebro_err_exit("%s(%s:%d): _cerebro_unmarshall_double",
                                     __FILE__, __FUNCTION__, __LINE__);
                  if (!len)
                    {
                      cerebro_err_debug("%s(%s:%d): packet buffer length invalid",
                                        __FILE__, __FUNCTION__, __LINE__);
                      goto cleanup;
                    }
                  count += len;
                  break;
                case CEREBRO_METRIC_VALUE_TYPE_STRING:
                case CEREBRO_METRIC_VALUE_TYPE_RAW:
                  if ((len = _cerebro_unmarshall_buffer((char *)hd->metric_value,
                                                        hd->metric_value_len,
                                                        buf + count,
                                                        buflen - count)) < 0)
                    cerebro_err_exit("%s(%s:%d): _cerebro_unmarshall_buffer",
                                     __FILE__, __FUNCTION__, __LINE__);
                  if (!len)
                    {
                      cerebro_err_debug("%s(%s:%d): packet buffer length invalid",
                                        __FILE__, __FUNCTION__, __LINE__);
                      goto cleanup;
                    }
                  count += len;
                  break;
                default:
                  cerebro_err_debug("%s(%s:%d): packet metric_value_type invalid: %d",
                                    __FILE__, __FUNCTION__, __LINE__,
                                    hd->metric_value_type);
                  goto cleanup;
                }
            }
          hb->metrics[i] = hd;
        }
      hd = NULL;
    }

  return hb;

 cleanup:
  if (hd)
    {
      if (hd->metric_value)
        Free(hd->metric_value);
      Free(hd);
    }

  if (hb)
    {
      if (hb->metrics)
        {
          i = 0;
          while (hb->metrics[i])
            {
              Free(hb->metrics[i]->metric_value);
              Free(hb->metrics[i]);
              i++;
            }
          Free(hb->metrics);
        }
      Free(hb);
    }
  return NULL;
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

  if (conf.debug && conf.listen_debug)
    {
      time_t t;
      struct tm tm;
      char strbuf[CEREBROD_STRING_BUFLEN];

      t = Time(NULL);
      Localtime_r(&t, &tm);
      strftime(strbuf, CEREBROD_STRING_BUFLEN, "%H:%M:%S", &tm);

      Pthread_mutex_lock(&debug_output_mutex);
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Received Heartbeat: %s\n", strbuf);     
      fprintf(stderr, "* -----------------------\n");
      cerebrod_heartbeat_dump(hb);
      fprintf(stderr, "**************************************\n");
      Pthread_mutex_unlock(&debug_output_mutex);
    }
#endif /* CEREBRO_DEBUG */
}

void *
cerebrod_listener(void *arg)
{
  struct timeval tv;

  _cerebrod_listener_initialize();

  for (;;)
    {
      struct cerebrod_heartbeat *hb;
      char nodename_buf[CEREBRO_MAXNODENAMELEN+1];
      char nodename_key[CEREBRO_MAXNODENAMELEN+1];
      int recv_len, heartbeat_len, flag, buflen;
      char *buf = NULL;
      
      Pthread_mutex_lock(&packet_buflen_max_lock);
      buflen = packet_buflen_max;
      Pthread_mutex_unlock(&packet_buflen_max_lock);
      buf = Malloc(buflen);

      Pthread_mutex_lock(&listener_fd_lock);
      if ((recv_len = recvfrom(listener_fd, 
			       buf, 
			       buflen, 
			       0, 
			       NULL, 
			       NULL)) < 0)
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
	      || listener_fd < 0)
            {
              if (!(listener_fd < 0))
		close(listener_fd);	/* no-wrapper, make best effort */

              if ((listener_fd = _cerebrod_listener_create_and_setup_socket()) < 0)
		{
		  cerebro_err_debug("%s(%s:%d): error re-initializing socket",
                                    __FILE__, __FUNCTION__, __LINE__);
                  
		  /* Wait a bit, so we don't spin */
		  sleep(CEREBROD_LISTENER_REINITIALIZE_WAIT);
		}
              else
                cerebro_err_debug("%s(%s:%d): success re-initializing socket",
                                  __FILE__, __FUNCTION__, __LINE__);
            }
          else if (errno == EINTR)
            cerebro_err_debug("%s(%s:%d): recvfrom: %s", 
                              __FILE__, __FUNCTION__, __LINE__,
                              strerror(errno));
          else
            cerebro_err_exit("%s(%s:%d): recvfrom: %s", 
                             __FILE__, __FUNCTION__, __LINE__,
                             strerror(errno));
	}
      Pthread_mutex_unlock(&listener_fd_lock);

      /* No packet read */
      if (recv_len <= 0)
	goto cleanup_continue;

      /* 
       * Packet is lost this time, oh well
       */
      if (recv_len >= buflen)
        {
          buflen += CEREBRO_PACKET_BUFLEN;
          Pthread_mutex_lock(&packet_buflen_max_lock);
          if (packet_buflen_max < buflen)
            packet_buflen_max = buflen;
          Pthread_mutex_unlock(&packet_buflen_max_lock);
          goto cleanup_continue;
        }

      if (recv_len < CEREBROD_HEARTBEAT_HEADER_LEN)
        {
          cerebro_err_debug("%s(%s:%d): received buf length "
                            "unexpected size: expect %d, heartbeat_len %d",
                            __FILE__, __FUNCTION__, __LINE__,
                            CEREBROD_HEARTBEAT_HEADER_LEN, heartbeat_len);
          goto cleanup_continue;
        }

      if (!(hb = _cerebrod_heartbeat_unmarshall(buf, recv_len)))
	goto cleanup_continue;

      _cerebrod_heartbeat_dump(hb);

      if (hb->version != CEREBROD_HEARTBEAT_PROTOCOL_VERSION)
	{
	  cerebro_err_debug("%s(%s:%d): invalid cerebrod packet version read:"
                            "expect %d, version %d",
                            __FILE__, __FUNCTION__, __LINE__,
                            CEREBROD_HEARTBEAT_PROTOCOL_VERSION, hb->version);
	  goto cleanup_continue;
	}
      
      if ((flag = _cerebro_clusterlist_module_node_in_cluster(hb->nodename)) < 0)
	cerebro_err_exit("%s(%s:%d): _cerebro_clusterlist_module_node_in_cluster: %s",
			 __FILE__, __FUNCTION__, __LINE__, hb->nodename);
      
      if (!flag)
	{
	  cerebro_err_debug("%s(%s:%d): received non-cluster packet from: %s",
			    __FILE__, __FUNCTION__, __LINE__,
			    hb->nodename);
	  goto cleanup_continue;
	}
      
      /* Guarantee ending '\0' character */
      memset(nodename_buf, '\0', CEREBRO_MAXNODENAMELEN+1);
      memcpy(nodename_buf, hb->nodename, CEREBRO_MAXNODENAMELEN);

      memset(nodename_key, '\0', CEREBRO_MAXNODENAMELEN+1);

      if (_cerebro_clusterlist_module_get_nodename(nodename_buf,
                                                   nodename_key, 
                                                   CEREBRO_MAXNODENAMELEN+1) < 0)
	{
	  cerebro_err_debug("%s(%s:%d): _cerebro_clusterlist_module_get_nodename: %s",
			    __FILE__, __FUNCTION__, __LINE__,
			    hb->nodename);
	  goto cleanup_continue;
	}

      Gettimeofday(&tv, NULL);
      cerebrod_node_data_update(nodename_key, hb, tv.tv_sec);
      cerebrod_heartbeat_destroy(hb);
    cleanup_continue:
      Free(buf);
    }

  return NULL;			/* NOT REACHED */
}
