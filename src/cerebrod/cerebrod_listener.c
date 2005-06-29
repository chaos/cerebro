/*****************************************************************************\
 *  $Id: cerebrod_listener.c,v 1.101 2005-06-29 17:03:52 achu Exp $
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
#include "cerebrod_listener.h"
#include "cerebrod_listener_data.h"
#include "cerebrod_metric.h"
#include "cerebrod_util.h"

#include "clusterlist_module.h"
#include "debug.h"
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
 * _listener_setup_socket
 *
 * Create and setup the listener socket.  Do not use wrappers in this
 * function.  We want to give the daemon additional chances to
 * "survive" an error condition.
 *
 * Returns file descriptor on success, -1 on error
 */
static int
_listener_setup_socket(void)
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
                     IP_ADD_MEMBERSHIP, 
                     &imr, 
                     sizeof(struct ip_mreqn)) < 0)
	{
	  CEREBRO_DBG(("setsockopt: %s", strerror(errno)));
	  return -1;
	}
    }

  /* Configuration checks ensure destination ip is on this machine if
   * it is a non-multicast address.
   */
  memset(&addr, '\0', sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(conf.heartbeat_destination_port);
  memcpy(&addr.sin_addr, 
         &conf.heartbeat_destination_ip_in_addr, 
         sizeof(struct in_addr));
  if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
    {
      CEREBRO_DBG(("bind: %s", strerror(errno)));
      return -1;
    }

  return fd;
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
  if ((listener_fd = _listener_setup_socket()) < 0)
    CEREBRO_EXIT(("listener_fd setup failed"));
  Pthread_mutex_unlock(&listener_fd_lock);

  if (!(clusterlist_handle = clusterlist_module_load()))
    CEREBRO_EXIT(("clusterlist_module_load"));
  
  if (clusterlist_module_setup(clusterlist_handle) < 0)
    CEREBRO_EXIT(("clusterlist_module_setup"));

  cerebrod_listener_data_initialize();

  cerebrod_listener_initialization_complete++;
  Pthread_cond_signal(&cerebrod_listener_initialization_complete_cond);
 out:
  Pthread_mutex_unlock(&cerebrod_listener_initialization_complete_lock);
}

/* 
 * _cerebrod_heartbeat_check_version
 *
 * Check that the version is correct prior to unmarshalling
 *
 * Returns 0 if version is correct, -1 if not
 */
static int
_cerebrod_heartbeat_check_version(const char *buf, unsigned int buflen)
{
  int32_t version;

  if (!Unmarshall_int32(&version, buf, buflen))
    return -1;

  if (version != CEREBROD_HEARTBEAT_PROTOCOL_VERSION)
    return -1;
  
  return 0;
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

      if (hb->metrics_len > conf.metric_max)
        {
          CEREBRO_DBG(("reducing metrics_len: len=%d", hb->metrics_len));
          hb->metrics_len = conf.metric_max;
        }

      hb->metrics = Malloc(sizeof(struct cerebrod_heartbeat_metric *)*(hb->metrics_len + 1));
      memset(hb->metrics, 
             '\0', 
             sizeof(struct cerebrod_heartbeat_metric *)*(hb->metrics_len + 1));
      
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
                  CEREBRO_DBG(("metric value len > 0 for type NONE"));
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
                  CEREBRO_DBG(("invalid type %d", hd->metric_value_type));
                  goto cleanup;
                }
            }
          hb->metrics[i] = hd;
        }
      hd = NULL;
    }
  
  return hb;

 bad_len_cleanup:
  CEREBRO_DBG(("packet buffer length invalid"));
  
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
  char *buf;
  int buflen;

  _cerebrod_listener_initialize();

  /* 
   * Determine max buffer length since packets are variable length
   */
  buflen = CEREBROD_HEARTBEAT_HEADER_LEN + conf.metric_max * (CEREBROD_HEARTBEAT_METRIC_HEADER_LEN + CEREBRO_MAX_METRIC_STRING_LEN);
  
  if (buflen < CEREBRO_MAX_PACKET_LEN)
    buflen = CEREBRO_MAX_PACKET_LEN;

  buf = Malloc(buflen);

  for (;;)
    {
      struct cerebrod_heartbeat *hb;
      char nodename_buf[CEREBRO_MAX_NODENAME_LEN+1];
      char nodename_key[CEREBRO_MAX_NODENAME_LEN+1];
      struct timeval tv;
      int recv_len, flag;
      
      Pthread_mutex_lock(&listener_fd_lock);
      if ((recv_len = recvfrom(listener_fd, 
			       buf, 
			       buflen, 
			       0, 
			       NULL, 
			       NULL)) < 0)
        listener_fd = cerebrod_reinitialize_socket(listener_fd,
                                                   _listener_setup_socket,
                                                   "listener: recvfrom");
      Pthread_mutex_unlock(&listener_fd_lock);

      /* No packet read */
      if (recv_len <= 0)
	continue;

      if (_cerebrod_heartbeat_check_version(buf, recv_len) < 0)
        continue;

      if (!(hb = _cerebrod_heartbeat_unmarshall_and_create(buf, recv_len)))
	continue;

      _cerebrod_heartbeat_dump(hb);
      
      /* Guarantee ending '\0' character */
      memset(nodename_buf, '\0', CEREBRO_MAX_NODENAME_LEN+1);
      memcpy(nodename_buf, hb->nodename, CEREBRO_MAX_NODENAME_LEN);

      if ((flag = clusterlist_module_node_in_cluster(clusterlist_handle,
						     nodename_buf)) < 0)
	CEREBRO_EXIT(("clusterlist_module_node_in_cluster: %s", nodename_buf));
      
      if (!flag)
	{
	  CEREBRO_DBG(("received non-cluster packet: %s", nodename_buf));
	  continue;
	}
      
      memset(nodename_key, '\0', CEREBRO_MAX_NODENAME_LEN+1);

      if (clusterlist_module_get_nodename(clusterlist_handle,
					  nodename_buf,
					  nodename_key, 
					  CEREBRO_MAX_NODENAME_LEN+1) < 0)
	{
	  CEREBRO_DBG(("clusterlist_module_get_nodename: %s", nodename_buf));
	  continue;
	}

      Gettimeofday(&tv, NULL);
      cerebrod_listener_data_update(nodename_key, hb, tv.tv_sec);
      cerebrod_heartbeat_destroy(hb);
    }

  return NULL;			/* NOT REACHED */
}
