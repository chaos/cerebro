/*****************************************************************************\
 *  $Id: cerebrod_listener.c,v 1.150 2010-02-02 01:01:20 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2018 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2005-2007 The Regents of the University of California.
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
 *  with Cerebro.  If not, see <http://www.gnu.org/licenses/>.
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
#include "cerebrod_debug.h"
#include "cerebrod_message.h"
#include "cerebrod_listener.h"
#include "cerebrod_listener_data.h"
#include "cerebrod_util.h"

#include "clusterlist_module.h"
#include "debug.h"
#include "data_util.h"
#include "wrappers.h"

extern struct cerebrod_config conf;
extern pthread_mutex_t debug_output_mutex;

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
int listener_fds[CEREBRO_CONFIG_LISTEN_MESSAGE_CONFIG_MAX];
pthread_mutex_t listener_fds_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * struct forwarding_info
 *
 * forwarding information and a lock to protect concurrent access
 */
struct forwarding_info
{
  int fd;
  hostlist_t hosts;
  pthread_mutex_t lock;
};
struct forwarding_info forwarding_info[CEREBRO_CONFIG_FORWARD_MESSAGE_CONFIG_MAX];

/*
 * clusterlist_handle
 *
 * Handle for clusterlist module
 */
clusterlist_module_t clusterlist_handle;
int found_clusterlist_module = 0;

/*
 * _listener_setup_socket
 *
 * Create and setup the listener socket.  Do not use wrappers in this
 * function.  We want to give the daemon additional chances to
 * "survive" an error condition.
 *
 * In this socket setup function, 'num' is used as the message config
 * and file descriptor index.
 *
 * Returns file descriptor on success, -1 on error
 */
static int
_listener_setup_socket(int num)
{
  struct sockaddr_in addr;
  unsigned int optlen;
  int fd, optval = 1;

  assert(num >= 0 && num < conf.listen_message_config_len);

  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      CEREBROD_ERR(("socket: %s", strerror(errno)));
      goto cleanup;
    }

  if (conf.listen_message_config[num].ip_is_multicast)
     {
      /* XXX: Probably lots of portability problems here */
      struct ip_mreqn imr;

      memset(&imr, '\0', sizeof(struct ip_mreqn));
      memcpy(&imr.imr_multiaddr,
             &conf.listen_message_config[num].ip_in_addr,
             sizeof(struct in_addr));
      memcpy(&imr.imr_address,
             &conf.listen_message_config[num].network_interface_in_addr,
             sizeof(struct in_addr));
      imr.imr_ifindex = conf.listen_message_config[num].network_interface_index;

      optlen = sizeof(struct ip_mreqn);
      if (setsockopt(fd, SOL_IP, IP_ADD_MEMBERSHIP, &imr, optlen) < 0)
	{
	  CEREBROD_ERR(("setsockopt: %s", strerror(errno)));
          goto cleanup;
	}
    }

  /* For quick start/restart */
  optval = 1;
  optlen = sizeof(optval);
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, optlen) < 0)
    {
      CEREBROD_ERR(("setsockopt: %s", strerror(errno)));
      goto cleanup;
    }

  memset(&addr, '\0', sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(conf.listen_message_config[num].port);
  memcpy(&addr.sin_addr,
         &conf.listen_message_config[num].ip_in_addr,
         sizeof(struct in_addr));
  if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
    {
      CEREBROD_ERR(("bind: %s", strerror(errno)));
      goto cleanup;
    }

  return fd;

 cleanup:
  /* ignore potential error, we're in the error path already */
  close(fd);
  return -1;
}

/*
 * _forwarding_setup_socket
 *
 * Create and setup the forwarding socket.  Do not use wrappers in
 * this function.  We want to give the daemon additional chances to
 * "survive" an error condition.
 *
 * In this socket setup function, 'num' is used as the message config
 * and forwarding info index.
 *
 * Returns file descriptor on success, -1 on error
 */
static int
_forwarding_setup_socket(int num)
{
  struct sockaddr_in addr;
  unsigned int optlen;
  int fd, optval = 1;

  assert(num >= 0 && num < conf.forward_message_config_len);

  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      CEREBROD_ERR(("socket: %s", strerror(errno)));
      goto cleanup;
    }

  if (conf.forward_message_config[num].ip_is_multicast)
    {
      /* XXX: Probably lots of portability problems here */
      struct ip_mreqn imr;

      memset(&imr, '\0', sizeof(struct ip_mreqn));
      memcpy(&imr.imr_multiaddr,
             &conf.forward_message_config[num].ip_in_addr,
             sizeof(struct in_addr));
      memcpy(&imr.imr_address,
             &conf.forward_message_config[num].network_interface_in_addr,
             sizeof(struct in_addr));
      imr.imr_ifindex = conf.forward_message_config[num].network_interface_index;

      optlen = sizeof(struct ip_mreqn);
      if (setsockopt(fd, SOL_IP, IP_MULTICAST_IF, &imr, optlen) < 0)
        {
          CEREBROD_ERR(("setsockopt: %s", strerror(errno)));
          goto cleanup;
        }

      /* Turn off loopback for forwarding, since we already
       * have the data.
       */
      optval = 0;
      optlen = sizeof(optval);
      if (setsockopt(fd, SOL_IP, IP_MULTICAST_LOOP, &optval, optlen) < 0)
        {
          CEREBROD_ERR(("setsockopt: %s", strerror(errno)));
          goto cleanup;
        }

      optval = conf.forward_message_ttl;
      optlen = sizeof(optval);
      if (setsockopt(fd, SOL_IP, IP_MULTICAST_TTL, &optval, optlen) < 0)
        {
          CEREBROD_ERR(("setsockopt: %s", strerror(errno)));
          goto cleanup;
        }
    }

  /* For quick start/restart */
  optval = 1;
  optlen = sizeof(optval);
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, optlen) < 0)
    {
      CEREBROD_ERR(("setsockopt: %s", strerror(errno)));
      goto cleanup;
    }

  memset(&addr, '\0', sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(conf.forward_message_config[num].source_port);
  memcpy(&addr.sin_addr,
         &conf.forward_message_config[num].network_interface_in_addr,
         sizeof(struct in_addr));
  if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
    {
      CEREBROD_ERR(("bind: %s", strerror(errno)));
      goto cleanup;
    }

  return fd;

 cleanup:
  /* ignore potential error, we're in the error path already */
  close(fd);
  return -1;
}

/*
 * _forwarding_setup
 *
 * 'index' is the index of the message config and forwarding info.
 *
 * Returns 0 success, -1 on error
 */
static int
_forwarding_setup(int index)
{
  hostlist_iterator_t itr = NULL;
  char *node;
  int rv = -1;

  assert(index >= 0 && index < conf.forward_message_config_len);

  /* We require a separate hostlist here b/c the hosts input by the
   * user and/or received by the remote hosts need to be mapped to a
   * single hostname.
   */
  if ((forwarding_info[index].fd = _forwarding_setup_socket(index)) < 0)
    goto cleanup;
  if (conf.forward_message_config[index].hosts)
    {
      forwarding_info[index].hosts = Hostlist_create(NULL);
      itr = Hostlist_iterator_create(conf.forward_message_config[index].hosts);
      while ((node = Hostlist_next(itr)))
        {
          char nodebuf[CEREBRO_MAX_NODENAME_LEN+1];
          char *nodeptr;

          if (found_clusterlist_module)
            {
              if (clusterlist_module_get_nodename(clusterlist_handle,
                                                  node,
                                                  nodebuf,
                                                  CEREBRO_MAX_NODENAME_LEN+1) < 0)
                {
                  CEREBROD_DBG(("clusterlist_module_get_nodename: %s", nodebuf));
                  Hostlist_destroy(forwarding_info[index].hosts);
                  goto cleanup;
                }
              nodeptr = nodebuf;
            }
          else
            nodeptr = node;

          Hostlist_push(forwarding_info[index].hosts, nodeptr);

          free(node);
        }
    }
  else
    forwarding_info[index].hosts = NULL;
  Pthread_mutex_init(&(forwarding_info[index].lock), NULL);
  rv = 0;
 cleanup:
  if (itr)
    Hostlist_iterator_destroy(itr);
  return rv;
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
  for (i = 0; i < conf.listen_message_config_len; i++)
    {
      if ((listener_fds[i] = _listener_setup_socket(i)) < 0)
        CEREBROD_EXIT(("listener fd setup failed"));
    }
  Pthread_mutex_unlock(&listener_fds_lock);

  if (!(clusterlist_handle = clusterlist_module_load()))
    CEREBROD_EXIT(("clusterlist_module_load"));

  if ((found_clusterlist_module = clusterlist_module_found(clusterlist_handle)) < 0)
    CEREBROD_EXIT(("clusterlist_module_found"));

  if (found_clusterlist_module)
    {
      if (clusterlist_module_setup(clusterlist_handle) < 0)
        CEREBROD_EXIT(("clusterlist_module_setup"));

      if (conf.debug && conf.listen_debug)
        {
          fprintf(stderr, "**************************************\n");
          fprintf(stderr, "* Cerebro Clusterlist\n");
          fprintf(stderr, "* -----------------------\n");
          fprintf(stderr, "* Using Clusterlist: %s\n",
                  clusterlist_module_name(clusterlist_handle));
          fprintf(stderr, "**************************************\n");
        }
    }

  cerebrod_listener_data_initialize();

  for (i = 0; i < conf.forward_message_config_len; i++)
    {
      /* if the forward destination is local to the machine, don't forward */
      if (conf.forward_message_config[i].ip_is_local)
        continue;
      if (_forwarding_setup(i) < 0)
        CEREBROD_EXIT(("forwarding setup failed"));
    }

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
          CEREBROD_DBG(("invalid packet length for no metrics"));
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
_cerebrod_message_dump(struct cerebrod_message *msg, char *hdr)
{
  assert(msg && hdr);

  if (!(conf.debug && conf.listen_debug))
    return;

  Pthread_mutex_lock(&debug_output_mutex);
  fprintf(stderr, "**************************************\n");
  fprintf(stderr, "* %s\n", hdr);
  fprintf(stderr, "* -----------------------\n");
  cerebrod_message_dump(msg);
  fprintf(stderr, "**************************************\n");
  Pthread_mutex_unlock(&debug_output_mutex);
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
      int in_cluster_flag, i, count;
      fd_set readfds;
      int recv_len = 0;
      int maxfd = 0;

      FD_ZERO(&readfds);
      Pthread_mutex_lock(&listener_fds_lock);
      for (i = 0; i < conf.listen_message_config_len; i++)
        {
          if (listener_fds[i] > maxfd)
            maxfd = listener_fds[i];
          FD_SET(listener_fds[i], &readfds);
        }

      count = Select(maxfd + 1, &readfds, NULL, NULL, NULL);

      for (i = 0; i < conf.listen_message_config_len; i++)
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
	  CEREBROD_DBG(("received truncated packet"));
          continue;
        }

      if (_cerebrod_message_check_version(buf, recv_len) < 0)
        {
	  CEREBROD_DBG(("received invalid version packet"));
          continue;
        }

      if (!(msg = _cerebrod_message_unmarshall(buf, recv_len)))
        {
	  CEREBROD_DBG(("received unmarshallable packet"));
          continue;
        }

      _cerebrod_message_dump(msg, "Received Message");

      /* Guarantee ending '\0' character */
      memset(nodename_buf, '\0', CEREBRO_MAX_NODENAME_LEN+1);
      memcpy(nodename_buf, msg->nodename, CEREBRO_MAX_NODENAME_LEN);

      if (!strlen(nodename_buf))
        {
          CEREBROD_DBG(("received null nodename"));
          cerebrod_message_destroy(msg);
          continue;
        }

      if (found_clusterlist_module)
        {
          if ((in_cluster_flag = clusterlist_module_node_in_cluster(clusterlist_handle,
                                                                    nodename_buf)) < 0)
            CEREBROD_EXIT(("clusterlist_module_node_in_cluster: %s", nodename_buf));

          /* Second chance, is this data being forwarded from another host */
          if (!in_cluster_flag)
            {
              if (Hostlist_find(conf.forward_host_accept, nodename_buf) >= 0)
                in_cluster_flag++;
            }
        }
      else
        /* must assume it is in the cluster */
        /* Note, there is no need to handle 'forward_host_accept' under this case,
         * since we don't know if it is in the cluster or not anyways.
         */
        in_cluster_flag = 1;

      if (!in_cluster_flag)
	{
	  CEREBROD_DBG(("received non-cluster packet: %s", nodename_buf));
          cerebrod_message_destroy(msg);
	  continue;
	}

      memset(nodename_key, '\0', CEREBRO_MAX_NODENAME_LEN+1);

      if (found_clusterlist_module)
        {
          if (clusterlist_module_get_nodename(clusterlist_handle,
                                              nodename_buf,
                                              nodename_key,
                                              CEREBRO_MAX_NODENAME_LEN+1) < 0)
            {
              CEREBROD_DBG(("clusterlist_module_get_nodename: %s", nodename_buf));
              cerebrod_message_destroy(msg);
              continue;
            }
        }
      else
        memcpy(nodename_key, nodename_buf, CEREBRO_MAX_NODENAME_LEN+1);

      Gettimeofday(&tv, NULL);
      cerebrod_listener_data_update(nodename_key, msg, tv.tv_sec);

      /* Forward data as necessary.  Note, there is no need to
       * marshall data, it should already be marshalled when we
       * read it earlier.
       */
      for (i = 0; i < conf.forward_message_config_len; i++)
        {
          /* if the forward destination is local to the machine, don't forward */
          if (conf.forward_message_config[i].ip_is_local)
            continue;

          if (!forwarding_info[i].hosts
              || hostlist_find(forwarding_info[i].hosts, nodename_key) >= 0)
            {
              struct sockaddr *addr;
              struct sockaddr_in msgaddr;
              unsigned int addrlen;
              int rv;

              memset(&msgaddr, '\0', sizeof(struct sockaddr_in));
              msgaddr.sin_family = AF_INET;
              msgaddr.sin_port = htons(conf.forward_message_config[i].destination_port);
              memcpy(&msgaddr.sin_addr,
                     &conf.forward_message_config[i].ip_in_addr,
                     sizeof(struct in_addr));

              addr = (struct sockaddr *)&msgaddr;
              addrlen = sizeof(struct sockaddr_in);

              _cerebrod_message_dump(msg, "Forwarding Message");

              Pthread_mutex_lock(&forwarding_info[i].lock);

              if ((rv = sendto(forwarding_info[i].fd,
                               buf,
                               recv_len,
                               0,
                               addr,
                               addrlen)) != recv_len)
                {
                  if (rv < 0)
                    forwarding_info[i].fd = cerebrod_reinit_socket(forwarding_info[i].fd,
                                                                   i,
                                                                   _forwarding_setup_socket,
                                                                   "forwarding: sendto");
                  else
                    CEREBROD_ERR(("sendto: invalid bytes sent"));
                }

              Pthread_mutex_unlock(&forwarding_info[i].lock);
            }
        }

      cerebrod_message_destroy(msg);
    }

  return NULL;			/* NOT REACHED */
}

#endif /* !WITH_CEREBROD_SPEAKER_ONLY */
