/*****************************************************************************\
 *  $Id: cerebrod_listener.c,v 1.94 2005-06-24 23:53:30 achu Exp $
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
#include "cerebro/cerebro_error.h"
#include "cerebro/cerebrod_heartbeat_protocol.h"

#include "cerebrod_config.h"
#include "cerebrod_heartbeat.h"
#include "cerebrod_listener.h"
#include "cerebrod_listener_data.h"
#include "cerebrod_metric.h"

#include "clusterlist_module.h"

#include "wrappers.h"

extern struct cerebrod_config conf;
#if CEREBRO_DEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* CEREBRO_DEBUG */

extern clusterlist_module_t clusterlist_handle;

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
 * clusterlist_handle
 *
 * Handle for clusterlist module
 */
clusterlist_module_t clusterlist_handle;

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

  if (!(clusterlist_handle = clusterlist_module_load()))
    cerebro_err_exit("%s(%s:%d): clusterlist_module_load",
                     __FILE__, __FUNCTION__, __LINE__);
  
  if (clusterlist_module_setup(clusterlist_handle) < 0)
    cerebro_err_exit("%s(%s:%d): clusterlist_module_setup",
                     __FILE__, __FUNCTION__, __LINE__);

  cerebrod_listener_data_initialize();

  cerebrod_listener_initialization_complete++;
  Pthread_cond_signal(&cerebrod_listener_initialization_complete_cond);
 out:
  Pthread_mutex_unlock(&cerebrod_listener_initialization_complete_lock);
}

/*
 * _cerebrod_heartbeat_unmarshall_and_create
 *
 * unmarshall contents of a heartbeat packet buffer and
 * return in an allocated heartbeat
 *
 * Returns heartbeat data on success, NULL on error
 */
static struct cerebrod_heartbeat *
_cerebrod_heartbeat_unmarshall_and_create(const char *buf, unsigned int buflen)
{
  struct cerebrod_heartbeat *hb = NULL;
  struct cerebrod_heartbeat_metric *hd = NULL;
  int i, n, len = 0;

  assert(buf);
  
  hb = Malloc(sizeof(struct cerebrod_heartbeat));

  memset(hb, '\0', sizeof(struct cerebrod_heartbeat));
  
  if (!(n = Unmarshall_int32(&(hb->version), buf + len, buflen - len)))
    goto bad_len_cleanup;
  len += n;

  if (!(n = Unmarshall_buffer(hb->nodename,
                              sizeof(hb->nodename),
                              buf + len,
                              buflen - len)))
    goto bad_len_cleanup;
  len += n;
  
  if (!(n = Unmarshall_u_int32(&(hb->metrics_len), buf + len, buflen - len)))
    goto bad_len_cleanup;
  len += n;
  
  if (hb->metrics_len)
    {
      hb->metrics = Malloc(sizeof(struct cerebrod_heartbeat_metric *)*(hb->metrics_len + 1));
      memset(hb->metrics, '\0', sizeof(struct cerebrod_heartbeat_metric *)*(hb->metrics_len + 1));
      
      for (i = 0; i < hb->metrics_len; i++)
        {
          hd = Malloc(sizeof(struct cerebrod_heartbeat_metric));
          memset(hd, '\0', sizeof(struct cerebrod_heartbeat_metric));
          
          if (!(n = Unmarshall_buffer(hd->metric_name,
                                      sizeof(hd->metric_name),
                                      buf + len,
                                      buflen - len)))
            goto bad_len_cleanup;
          len += n;
          
          if (!(n = Unmarshall_u_int32(&(hd->metric_value_type),
                                       buf + len,
                                       buflen - len)))
            goto bad_len_cleanup;
          len += n;
          
          if (!(n = Unmarshall_u_int32(&(hd->metric_value_len),
                                       buf + len,
                                       buflen - len)))
            goto bad_len_cleanup;
          len += n;
          
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
                  if (!(n = Unmarshall_int32((int32_t *)hd->metric_value,
                                             buf + len,
                                             buflen - len)))
                    goto bad_len_cleanup;
                  len += n;
                  break;
                case CEREBRO_METRIC_VALUE_TYPE_U_INT32:
                  if (!(n = Unmarshall_u_int32((u_int32_t *)hd->metric_value,
                                               buf + len,
                                               buflen - len)))
                    goto bad_len_cleanup;
                  len += n;
                  break;
                case CEREBRO_METRIC_VALUE_TYPE_FLOAT:
                  if (!(n = Unmarshall_float((float *)hd->metric_value,
                                             buf + len,
                                             buflen - len)))
                    goto bad_len_cleanup;
                  len += n;
                  break;
                case CEREBRO_METRIC_VALUE_TYPE_DOUBLE:
                  if (!(n = Unmarshall_double((double *)hd->metric_value,
                                              buf + len,
                                              buflen - len)))
                    goto bad_len_cleanup;
                  len += n;
                  break;
                case CEREBRO_METRIC_VALUE_TYPE_STRING:
                  if (!(n = Unmarshall_buffer((char *)hd->metric_value,
                                              hd->metric_value_len,
                                              buf + len,
                                              buflen - len)))
                    goto bad_len_cleanup;
                  len += n;
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

 bad_len_cleanup:
  cerebro_err_debug("%s(%s:%d): packet buffer length invalid",
                    __FILE__, __FUNCTION__, __LINE__);
  
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
      Pthread_mutex_lock(&debug_output_mutex);
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Received Heartbeat\n");
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
      char nodename_buf[CEREBRO_MAX_NODENAME_LEN+1];
      char nodename_key[CEREBRO_MAX_NODENAME_LEN+1];
      int recv_len, flag;
      char buf[CEREBRO_MAX_PACKET_LEN];
      
      Pthread_mutex_lock(&listener_fd_lock);
      if ((recv_len = recvfrom(listener_fd, 
			       buf, 
			       CEREBRO_MAX_PACKET_LEN, 
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
	continue;

      if (recv_len < CEREBROD_HEARTBEAT_HEADER_LEN)
        {
          cerebro_err_debug("%s(%s:%d): received buf length too small, "
                            "expect header %d, recv_len %d",
                            __FILE__, __FUNCTION__, __LINE__,
                            CEREBROD_HEARTBEAT_HEADER_LEN, recv_len);
          continue;
        }

      if (!(hb = _cerebrod_heartbeat_unmarshall_and_create(buf, recv_len)))
	continue;

      _cerebrod_heartbeat_dump(hb);

      if (hb->version != CEREBROD_HEARTBEAT_PROTOCOL_VERSION)
	{
	  cerebro_err_debug("%s(%s:%d): invalid cerebrod packet version read:"
                            "expect %d, version %d",
                            __FILE__, __FUNCTION__, __LINE__,
                            CEREBROD_HEARTBEAT_PROTOCOL_VERSION, hb->version);
	  continue;
	}
      
      if ((flag = clusterlist_module_node_in_cluster(clusterlist_handle,
						     hb->nodename)) < 0)
	cerebro_err_exit("%s(%s:%d): clusterlist_module_node_in_cluster: %s",
			 __FILE__, __FUNCTION__, __LINE__, hb->nodename);
      
      if (!flag)
	{
	  cerebro_err_debug("%s(%s:%d): received non-cluster packet from: %s",
			    __FILE__, __FUNCTION__, __LINE__,
			    hb->nodename);
	  continue;
	}
      
      /* Guarantee ending '\0' character */
      memset(nodename_buf, '\0', CEREBRO_MAX_NODENAME_LEN+1);
      memcpy(nodename_buf, hb->nodename, CEREBRO_MAX_NODENAME_LEN);

      memset(nodename_key, '\0', CEREBRO_MAX_NODENAME_LEN+1);

      if (clusterlist_module_get_nodename(clusterlist_handle,
					  nodename_buf,
					  nodename_key, 
					  CEREBRO_MAX_NODENAME_LEN+1) < 0)
	{
	  cerebro_err_debug("%s(%s:%d): clusterlist_module_get_nodename: %s",
			    __FILE__, __FUNCTION__, __LINE__,
			    hb->nodename);
	  continue;
	}

      Gettimeofday(&tv, NULL);
      cerebrod_listener_data_update(nodename_key, hb, tv.tv_sec);
      cerebrod_heartbeat_destroy(hb);
    }

  return NULL;			/* NOT REACHED */
}
