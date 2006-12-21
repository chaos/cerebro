/*****************************************************************************\
 *  $Id: cerebrod_listener.c,v 1.132 2006-12-21 01:42:17 chu11 Exp $
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
#include <sys/select.h>
#include <netinet/in.h>

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebrod_message_protocol.h"

#include "cerebrod_config.h"
#include "cerebrod_message.h"
#include "cerebrod_listener.h"
#include "cerebrod_listener_data.h"
#include "cerebrod_util.h"

#include "clusterlist_module.h"
#include "debug.h"
#include "data_util.h"
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
 * listener_fds
 * listener_fds_lock
 *
 * listener file descriptor and lock to protect concurrent access
 */
int listener_fds[CEREBRO_MAX_LISTENERS] = {0, 0, 0, 0};
pthread_mutex_t listener_fds_lock = PTHREAD_MUTEX_INITIALIZER;

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
 * In this socket setup function, 'num' is used as the file descriptor
 * index.
 * 
 * Returns file descriptor on success, -1 on error
 */
static int
_listener_setup_socket(int num)
{
  struct sockaddr_in addr;
  unsigned int optlen;
  int fd, optval = 1;

  assert(num >= 0 && num < conf.listen_len);

  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      CEREBRO_DBG(("socket: %s", strerror(errno)));
      goto cleanup;
    }

  if (conf.listen_ips_is_multicast[num])
     {
      /* XXX: Probably lots of portability problems here */
      struct ip_mreqn imr;

      memset(&imr, '\0', sizeof(struct ip_mreqn));
      memcpy(&imr.imr_multiaddr,
             &conf.listen_ips_in_addr[num],
             sizeof(struct in_addr));
      memcpy(&imr.imr_address,
             &conf.listen_network_interfaces_in_addr[num],
             sizeof(struct in_addr));
      imr.imr_ifindex = conf.listen_network_interfaces_index[num];

      optlen = sizeof(struct ip_mreqn);
      if (setsockopt(fd, SOL_IP, IP_ADD_MEMBERSHIP, &imr, optlen) < 0)
	{
	  CEREBRO_DBG(("setsockopt: %s", strerror(errno)));
          goto cleanup;
	}
    }

  /* For quick start/restart */
  optval = 1;
  optlen = sizeof(optval);
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, optlen) < 0)
    {
      CEREBRO_DBG(("setsockopt: %s", strerror(errno)));
      goto cleanup;
    }

  /* Configuration checks ensure destination ip is on this machine if
   * it is a non-multicast address.
   */
  memset(&addr, '\0', sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(conf.listen_ports[num]);
  memcpy(&addr.sin_addr, 
         &conf.listen_ips_in_addr[num],
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
  int i;

  Pthread_mutex_lock(&listener_init_lock);
  if (listener_init)
    goto out;

  Pthread_mutex_lock(&listener_fds_lock);
  for (i = 0; i < conf.listen_len; i++)
    {
      if ((listener_fds[i] = _listener_setup_socket(i)) < 0)
        CEREBRO_EXIT(("listener fd setup failed"));
    }
  Pthread_mutex_unlock(&listener_fds_lock);

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
 * _cerebrod_message_check_version
 *
 * Check that the version is correct prior to unmarshalling
 *
 * Returns 0 if version is correct, -1 if not
 */
static int
_cerebrod_message_check_version(const char *buf, unsigned int buflen)
{
  int32_t version;

  if (!Unmarshall_int32(&version, buf, buflen))
    return -1;

  if (version != CEREBROD_MESSAGE_PROTOCOL_VERSION)
    return -1;
  
  return 0;
}

/*
 * _cerebrod_message_unmarshall
 *
 * unmarshall contents of a message packet buffer and
 * return in an allocated message
 *
 * Returns message data on success, NULL on error
 */
static struct cerebrod_message *
_cerebrod_message_unmarshall(const char *buf, unsigned int buflen)
{
  struct cerebrod_message *msg = NULL;
  struct cerebrod_message_metric *mm = NULL;
  unsigned int size;
  char *bufPtr;
  int i, n, bufPtrlen, c = 0;

  assert(buf);
  
  msg = Malloc(sizeof(struct cerebrod_message));

  memset(msg, '\0', sizeof(struct cerebrod_message));
  
  if (!(n = Unmarshall_int32(&(msg->version), buf + c, buflen - c)))
    goto cleanup;
  c += n;

  bufPtr = msg->nodename;
  bufPtrlen = sizeof(msg->nodename);
  if (!(n = Unmarshall_buffer(bufPtr, bufPtrlen, buf + c, buflen - c)))
    goto cleanup;
  c += n;
  
  if (!(n = Unmarshall_u_int32(&(msg->metrics_len), buf + c, buflen - c)))
    goto cleanup;
  c += n;

  /* If no metrics in this packet, just return with the header */
  if (!msg->metrics_len)
    {
      if (buflen != CEREBROD_MESSAGE_HEADER_LEN)
        {
          CEREBRO_DBG(("invalid packet length for no metrics"));
          goto cleanup;
        }
      msg->metrics = NULL;
      return msg;
    }
  
  size = sizeof(struct cerebrod_message_metric *)*(msg->metrics_len + 1);
  msg->metrics = Malloc(size);
  memset(msg->metrics, '\0', size);
      
  for (i = 0; i < msg->metrics_len; i++)
    {
      char *mname;
      int mnamelen;

      mm = Malloc(sizeof(struct cerebrod_message_metric));
      memset(mm, '\0', sizeof(struct cerebrod_message_metric));
      
      mname = mm->metric_name;
      mnamelen = sizeof(mm->metric_name);
      
      if (!(n = Unmarshall_buffer(mname, mnamelen, buf + c, buflen - c)))
        goto cleanup;
      c += n;
      
      if ((n = unmarshall_data_type_len(&(mm->metric_value_type),
                                        &(mm->metric_value_len),
                                        buf + c, 
                                        buflen - c,
                                        NULL)) < 0)
        goto cleanup;
      c += n;
      
      if (check_data_type_len(mm->metric_value_type, mm->metric_value_len) < 0)
        goto cleanup;

      mm->metric_value = NULL;
      if (mm->metric_value_len)
        {
          mm->metric_value = Malloc(mm->metric_value_len);
          if ((n = unmarshall_data_value(mm->metric_value_type, 
                                         mm->metric_value_len,
                                         mm->metric_value,
                                         mm->metric_value_len,
                                         buf + c,
                                         buflen - c,
                                         NULL)) < 0)
            goto cleanup;
          c += n;
        }

      msg->metrics[i] = mm;
    }
  mm = NULL;

  return msg;

 cleanup:
  if (mm)
    {
      if (mm->metric_value)
        Free(mm->metric_value);
      Free(mm);
    }

  if (msg)
    {
      if (msg->metrics)
        {
          i = 0;
          while (msg->metrics[i])
            {
              if (msg->metrics[i]->metric_value)
                Free(msg->metrics[i]->metric_value);
              Free(msg->metrics[i]);
              i++;
            }
          Free(msg->metrics);
        }
      Free(msg);
    }
  return NULL;
}

/*
 * _cerebrod_message_dump
 *
 * Dump contents of message packet
 */
static void
_cerebrod_message_dump(struct cerebrod_message *msg)
{
#if CEREBRO_DEBUG
  assert(msg);

  if (!(conf.debug && conf.listen_debug))
    return;

  Pthread_mutex_lock(&debug_output_mutex);
  fprintf(stderr, "**************************************\n");
  fprintf(stderr, "* Received Message\n");
  fprintf(stderr, "* -----------------------\n");
  cerebrod_message_dump(msg);
  fprintf(stderr, "**************************************\n");
  Pthread_mutex_unlock(&debug_output_mutex);
#endif /* CEREBRO_DEBUG */
}

void *
cerebrod_listener(void *arg)
{
  char buf[CEREBRO_MAX_PACKET_LEN];

  _cerebrod_listener_initialize();

  for (;;)
    {
      struct cerebrod_message *msg;
      char nodename_buf[CEREBRO_MAX_NODENAME_LEN+1];
      char nodename_key[CEREBRO_MAX_NODENAME_LEN+1];
      struct timeval tv;
      int flag, i, count;
      fd_set readfds;
      int recv_len = 0;
      int maxfd = 0;

      FD_ZERO(&readfds);
      Pthread_mutex_lock(&listener_fds_lock);
      for (i = 0; i < conf.listen_len; i++)
        {
          if (listener_fds[i] > maxfd)
            maxfd = listener_fds[i];
          FD_SET(listener_fds[i], &readfds);
        }

      count = Select(maxfd + 1, &readfds, NULL, NULL, NULL);

      for (i = 0; i < conf.listen_len; i++)
        {
          if (FD_ISSET(listener_fds[i], &readfds))
            {
              if ((recv_len = recvfrom(listener_fds[i], 
                                       buf, 
                                       CEREBRO_MAX_PACKET_LEN, 
                                       0, 
                                       NULL, 
                                       NULL)) < 0)
                listener_fds[i] = cerebrod_reinit_socket(listener_fds[i], 
                                                         i,
                                                         _listener_setup_socket, 
                                                         "listener: recvfrom");
              break;
            }
        }
      Pthread_mutex_unlock(&listener_fds_lock);

      /* No packet read */
      if (recv_len <= 0)
	continue;

      if (recv_len >= CEREBRO_MAX_PACKET_LEN)
        {
	  CEREBRO_DBG(("received truncated packet"));
          continue;
        }

      if (_cerebrod_message_check_version(buf, recv_len) < 0)
        {
	  CEREBRO_DBG(("received invalid version packet"));
          continue;
        }

      if (!(msg = _cerebrod_message_unmarshall(buf, recv_len)))
        {
	  CEREBRO_DBG(("received unmarshallable packet"));
          continue;
        }

      _cerebrod_message_dump(msg);
      
      /* Guarantee ending '\0' character */
      memset(nodename_buf, '\0', CEREBRO_MAX_NODENAME_LEN+1);
      memcpy(nodename_buf, msg->nodename, CEREBRO_MAX_NODENAME_LEN);

      if (!strlen(nodename_buf))
        {
          CEREBRO_DBG(("received null nodename"));
          cerebrod_message_destroy(msg);
          continue;
        }

      if ((flag = clusterlist_module_node_in_cluster(clusterlist_handle,
						     nodename_buf)) < 0)
	CEREBRO_EXIT(("clusterlist_module_node_in_cluster: %s", nodename_buf));
      
      if (!flag)
	{
	  CEREBRO_DBG(("received non-cluster packet: %s", nodename_buf));
          cerebrod_message_destroy(msg);
	  continue;
	}
      
      memset(nodename_key, '\0', CEREBRO_MAX_NODENAME_LEN+1);

      if (clusterlist_module_get_nodename(clusterlist_handle,
					  nodename_buf,
					  nodename_key, 
					  CEREBRO_MAX_NODENAME_LEN+1) < 0)
	{
	  CEREBRO_DBG(("clusterlist_module_get_nodename: %s", nodename_buf));
          cerebrod_message_destroy(msg);
	  continue;
	}

      Gettimeofday(&tv, NULL);
      cerebrod_listener_data_update(nodename_key, msg, tv.tv_sec);
      cerebrod_message_destroy(msg);
    }

  return NULL;			/* NOT REACHED */
}

#endif /* !WITH_CEREBROD_SPEAKER_ONLY */
