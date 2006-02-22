/*****************************************************************************\
 *  $Id: cerebrod_listener.c,v 1.123 2006-02-22 06:08:27 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2005 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <http://www.llnl.gov/linux/cerebro/>.
 *
 *  Cerebro is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  Cerebro is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with Genders; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if !WITH_CEREBROD_SPEAKER_ONLY

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
#include "cerebrod_util.h"

#include "clusterlist_module.h"
#include "debug.h"
#include "metric_util.h"
#include "wrappers.h"

extern struct cerebrod_config conf;
#if CEREBRO_DEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* CEREBRO_DEBUG */

extern clusterlist_module_t clusterlist_handle;

/* 
 * listener_init
 * listener_init_cond
 * listener_init_lock
 *
 * variables for synchronizing initialization between different pthreads
 * and signaling when it is complete
 */
int listener_init = 0;
pthread_cond_t listener_init_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t listener_init_lock = PTHREAD_MUTEX_INITIALIZER;

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
  int fd, optval = 1;

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

      memset(&imr, '\0', sizeof(struct ip_mreqn));
      memcpy(&imr.imr_multiaddr,
             &conf.heartbeat_destination_ip_in_addr,
             sizeof(struct in_addr));
      memcpy(&imr.imr_address,
             &conf.heartbeat_network_interface_in_addr,
             sizeof(struct in_addr));
      imr.imr_ifindex = conf.heartbeat_interface_index;

      optlen = sizeof(struct ip_mreqn);
      if (setsockopt(fd, SOL_IP, IP_ADD_MEMBERSHIP, &imr, optlen) < 0)
	{
	  CEREBRO_DBG(("setsockopt: %s", strerror(errno)));
          goto cleanup;
	}
    }

  /* For quick start/restart */
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) < 0)
    {
      CEREBRO_DBG(("setsockopt: %s", strerror(errno)));
      goto cleanup;
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
      goto cleanup;
    }

  return fd;

 cleanup:
  close(fd);
  return -1;
}

/*
 * _cerebrod_listener_initialize
 *
 * perform listener initialization
 */
static void
_cerebrod_listener_initialize(void)
{
  Pthread_mutex_lock(&listener_init_lock);
  if (listener_init)
    goto out;

  Pthread_mutex_lock(&listener_fd_lock);
  if ((listener_fd = _listener_setup_socket()) < 0)
    CEREBRO_EXIT(("listener fd setup failed"));
  Pthread_mutex_unlock(&listener_fd_lock);

  if (!(clusterlist_handle = clusterlist_module_load()))
    CEREBRO_EXIT(("clusterlist_module_load"));
  
  if (clusterlist_module_setup(clusterlist_handle) < 0)
    CEREBRO_EXIT(("clusterlist_module_setup"));

#if CEREBRO_DEBUG
  if (conf.debug && conf.listen_debug)
    {
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Cerebro Clusterlist\n");
      fprintf(stderr, "* -----------------------\n");
      fprintf(stderr, "* Using Clusterlist: %s\n", 
              clusterlist_module_name(clusterlist_handle));
      fprintf(stderr, "**************************************\n");
    }
#endif /* CEREBRO_DEBUG */

  cerebrod_listener_data_initialize();

  listener_init++;
  Pthread_cond_signal(&listener_init_cond);
 out:
  Pthread_mutex_unlock(&listener_init_lock);
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
  unsigned int size;
  char *bufPtr;
  int i, n, bufPtrlen, c = 0;

  assert(buf);
  
  hb = Malloc(sizeof(struct cerebrod_heartbeat));

  memset(hb, '\0', sizeof(struct cerebrod_heartbeat));
  
  if (!(n = Unmarshall_int32(&(hb->version), buf + c, buflen - c)))
    goto cleanup;
  c += n;

  bufPtr = hb->nodename;
  bufPtrlen = sizeof(hb->nodename);
  if (!(n = Unmarshall_buffer(bufPtr, bufPtrlen, buf + c, buflen - c)))
    goto cleanup;
  c += n;
  
  if (!(n = Unmarshall_u_int32(&(hb->metrics_len), buf + c, buflen - c)))
    goto cleanup;
  c += n;

  /* If no metrics in this packet, just return with the header */
  if (!hb->metrics_len)
    {
      if (buflen != CEREBROD_HEARTBEAT_HEADER_LEN)
        {
          CEREBRO_DBG(("invalid packet length for no metrics"));
          goto cleanup;
        }
      hb->metrics = NULL;
      return hb;
    }
  
  if (hb->metrics_len > conf.metric_max)
    {
      CEREBRO_DBG(("reducing metrics_len: len=%d", hb->metrics_len));
      hb->metrics_len = conf.metric_max;
    }
  
  size = sizeof(struct cerebrod_heartbeat_metric *)*(hb->metrics_len + 1);
  hb->metrics = Malloc(size);
  memset(hb->metrics, '\0', size);
      
  for (i = 0; i < hb->metrics_len; i++)
    {
      char *mname;
      int mnamelen;

      hd = Malloc(sizeof(struct cerebrod_heartbeat_metric));
      memset(hd, '\0', sizeof(struct cerebrod_heartbeat_metric));
      
      mname = hd->metric_name;
      mnamelen = sizeof(hd->metric_name);
      
      if (!(n = Unmarshall_buffer(mname, mnamelen, buf + c, buflen - c)))
        goto cleanup;
      c += n;
      
      if ((n = unmarshall_metric_type_len(&(hd->metric_value_type),
                                          &(hd->metric_value_len),
                                          buf + c, 
                                          buflen - c,
                                          NULL)) < 0)
        goto cleanup;
      c += n;
      
      if (check_metric_type_len(hd->metric_value_type, hd->metric_value_len) < 0)
        goto cleanup;

      hd->metric_value = NULL;
      if (hd->metric_value_len)
        {
          hd->metric_value = Malloc(hd->metric_value_len);
          if ((n = unmarshall_metric_value(hd->metric_value_type, 
                                           hd->metric_value_len,
                                           hd->metric_value,
                                           hd->metric_value_len,
                                           buf + c,
                                           buflen - c,
                                           NULL)) < 0)
            goto cleanup;
          c += n;
        }

      hb->metrics[i] = hd;
    }
  hd = NULL;

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
              if (hb->metrics[i]->metric_value)
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

  if (!(conf.debug && conf.listen_debug))
    return;

  Pthread_mutex_lock(&debug_output_mutex);
  fprintf(stderr, "**************************************\n");
  fprintf(stderr, "* Received Heartbeat\n");
  fprintf(stderr, "* -----------------------\n");
  cerebrod_heartbeat_dump(hb);
  fprintf(stderr, "**************************************\n");
  Pthread_mutex_unlock(&debug_output_mutex);
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
      if ((recv_len = recvfrom(listener_fd, buf, buflen, 0, NULL, NULL)) < 0)
        listener_fd = cerebrod_reinit_socket(listener_fd, 
                                             _listener_setup_socket, 
                                             "listener: recvfrom");
      Pthread_mutex_unlock(&listener_fd_lock);

      /* No packet read */
      if (recv_len <= 0)
	continue;

      if (_cerebrod_heartbeat_check_version(buf, recv_len) < 0)
        {
	  CEREBRO_DBG(("received invalid version packet"));
          continue;
        }

      if (!(hb = _cerebrod_heartbeat_unmarshall(buf, recv_len)))
        {
	  CEREBRO_DBG(("received unmarshallable packet"));
          continue;
        }

      _cerebrod_heartbeat_dump(hb);
      
      /* Guarantee ending '\0' character */
      memset(nodename_buf, '\0', CEREBRO_MAX_NODENAME_LEN+1);
      memcpy(nodename_buf, hb->nodename, CEREBRO_MAX_NODENAME_LEN);

      if (!strlen(nodename_buf))
        {
          CEREBRO_DBG(("received null nodename"));
          continue;
        }

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

#endif /* !WITH_CEREBROD_SPEAKER_ONLY */
