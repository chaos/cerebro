/*****************************************************************************\
 *  $Id: cerebrod_event_server.c,v 1.1.2.6 2006-11-04 04:22:38 chu11 Exp $
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

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_event_protocol.h"

#include "cerebrod_config.h"
#include "cerebrod_event_server.h"
#include "cerebrod_util.h"

#include "debug.h"
#include "event_module.h"
#include "fd.h"
#include "hash.h"
#include "list.h"
#include "metric_util.h"
#include "network_util.h"
#include "wrappers.h"

extern struct cerebrod_config conf;
#if CEREBRO_DEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* CEREBRO_DEBUG */

#define CEREBROD_EVENT_SERVER_BACKLOG 10

/* 
 * event_server_init
 * event_server_init_cond
 * event_server_init_lock
 *
 * variables for synchronizing initialization between different pthreads
 * and signaling when it is complete
 */
int event_server_init = 0;
pthread_cond_t event_server_init_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t event_server_init_lock = PTHREAD_MUTEX_INITIALIZER;

/* 
 * event_queue_monitor_init
 * event_queue_monitor_init_cond
 * event_queue_monitor_init_lock
 *
 * variables for synchronizing initialization between different pthreads
 * and signaling when it is complete
 */
int event_queue_monitor_init = 0;
pthread_cond_t event_queue_monitor_init_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t event_queue_monitor_init_lock = PTHREAD_MUTEX_INITIALIZER;

/* 
 * event_queue
 * event_queue_cond
 * event_queue_lock
 *
 * queue of events to send out and the conditional variable and mutex
 * for exclusive access and signaling.
 */
List event_queue = NULL;
pthread_cond_t event_queue_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t event_queue_lock = PTHREAD_MUTEX_INITIALIZER;

/* 
 * event_connections
 *
 * hash of file descriptors to send event info to.
 */
hash_t event_connections = NULL;
 
extern event_modules_t event_handle;
extern hash_t event_index;
extern List event_names;

void
cerebrod_queue_event(struct cerebro_event *event, unsigned int index)
{
  struct cerebrod_event_to_send *ets;

  assert(event);
  assert(event_queue);

  Pthread_mutex_lock(&event_queue_lock);

  ets = (struct cerebrod_event_to_send *)Malloc(sizeof(struct cerebrod_event_to_send));
  ets->event_name = event->event_name;
  ets->index = index;
  ets->event = event;
  
  List_append(event_queue, ets);

  Pthread_cond_signal(&event_queue_cond);
  Pthread_mutex_unlock(&event_queue_lock);
}

/* 
 * _cerebrod_event_to_send_destroy
 */
static void
_cerebrod_event_to_send_destroy(void *x)
{
  struct cerebrod_event_to_send *ets;

  assert(x);
  assert(event_handle);

  ets = (struct cerebrod_event_to_send *)x;
  event_module_destroy(event_handle, ets->index, ets->event);
  Free(ets);
}

/*
 * _event_queue_monitor_initialize
 *
 * perform metric queue_monitor initialization
 */
static void
_event_queue_monitor_initialize(void)
{
  Pthread_mutex_lock(&event_queue_monitor_init_lock);
  if (event_queue_monitor_init)
    goto out;

  event_queue = List_create((ListDelF)_cerebrod_event_to_send_destroy);

  event_queue_monitor_init++;
  Pthread_cond_signal(&event_queue_monitor_init_cond);
 out:
  Pthread_mutex_unlock(&event_queue_monitor_init_lock);
}

/* 
 * _event_dump
 *
 * Output event debugging info
 */
static void
_event_dump(struct cerebro_event *event)
{
#if CEREBRO_DEBUG
  if (conf.event_server_debug)
    {
      char *buf;

      Pthread_mutex_lock(&debug_output_mutex);
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Cerebrod Event:\n");
      fprintf(stderr, "* ---------------\n");
      fprintf(stderr, "* Version: %d\n", event->version);
      fprintf(stderr, "* Nodename: %s\n", event->nodename);
      fprintf(stderr, "* Event_Name: %s\n", event->event_name);
      fprintf(stderr, "* Value Type: %d\n", event->event_value_type);
      fprintf(stderr, "* Value Len: %d\n", event->event_value_len);

      switch(event->event_value_type)
        {
        case CEREBRO_METRIC_VALUE_TYPE_NONE:
          break;
        case CEREBRO_METRIC_VALUE_TYPE_INT32:
          fprintf(stderr, "value = %d",
                  *((int32_t *)event->event_value));
          break;
        case CEREBRO_METRIC_VALUE_TYPE_U_INT32:
          fprintf(stderr, "value = %u",
                  *((u_int32_t *)event->event_value));
          break;
        case CEREBRO_METRIC_VALUE_TYPE_FLOAT:
          fprintf(stderr, "value = %f",
                  *((float *)event->event_value));
          break;
        case CEREBRO_METRIC_VALUE_TYPE_DOUBLE:
          fprintf(stderr, "value = %f",
                  *((double *)event->event_value));
          break;
        case CEREBRO_METRIC_VALUE_TYPE_STRING:
          /* Watch for NUL termination */
          buf = Malloc(event->event_value_len + 1);
          memset(buf, '\0', event->event_value_len + 1);
          memcpy(buf,
                 event->event_value,
                 event->event_value_len);
          fprintf(stderr, "value = %s", buf);
          Free(buf);
          break;
#if SIZEOF_LONG == 4
        case CEREBRO_METRIC_VALUE_TYPE_INT64:
          fprintf(stderr, "value = %lld",
                  *((int64_t *)event->event_value));
          break;
        case CEREBRO_METRIC_VALUE_TYPE_U_INT64:
          fprintf(stderr, "value = %llu",
                  *((u_int64_t *)event->event_value));
          break;
#else  /* SIZEOF_LONG == 8 */
        case CEREBRO_METRIC_VALUE_TYPE_INT64:
          fprintf(stderr, "value = %ld",
                  *((int64_t *)event->event_value));
          break;
        case CEREBRO_METRIC_VALUE_TYPE_U_INT64:
          fprintf(stderr, "value = %lu",
                  *((u_int64_t *)event->event_value));
          break;
#endif /* SIZEOF_LONG == 8 */
        default:
          break;
        }
      fprintf(stderr, "\n");
      fprintf(stderr, "**************************************\n");
      Pthread_mutex_unlock(&debug_output_mutex);
    }
#endif /* CEREBRO_DEBUG */
}

/* 
 * _event_marshall
 *
 * marshall event packet
 *
 * Returns length written to buffer on success, -1 on error
 */
static int
_event_marshall(struct cerebro_event *event,
                char *buf,
                unsigned int buflen)
{
  int bufPtrlen, n, c = 0;
  char *bufPtr;
  
  assert(event && buf && buflen >= CEREBRO_EVENT_HEADER_LEN);

  memset(buf, '\0', buflen);

  c += Marshall_int32(event->version, buf + c, buflen - c);

  bufPtr = event->nodename;
  bufPtrlen = sizeof(event->nodename);
  c += Marshall_buffer(bufPtr, bufPtrlen, buf + c, buflen - c);
  bufPtr = event->event_name;
  bufPtrlen = sizeof(event->event_name);
  c += Marshall_buffer(bufPtr, bufPtrlen, buf + c, buflen - c);

  c += Marshall_u_int32(event->event_value_type, buf + c, buflen - c);
  c += Marshall_u_int32(event->event_value_len, buf + c, buflen - c);

  if ((n = marshall_metric(event->event_value_type,
                           event->event_value_len,
                           event->event_value,
                           buf + c,
                           buflen - c,
                           NULL)) < 0)
    goto cleanup;
  c += n;

  return c;

 cleanup:
  return -1;
}

void *
cerebrod_event_queue_monitor(void *arg)
{
  _event_queue_monitor_initialize();

  /* 
   * achu: The listener and thus event update initialization is
   * started after this thread is started.  So the and event_index may
   * not be set up the first time this loop is reached.  
   *
   * However, it must be set after the condition is signaled, b/c the
   * listener (and thus event update code) and event node timeout
   * thread begin after the listener is setup.
   *
   * Thus, we put the event_queue assert inside the loop.
   */
  for (;;)
    {
      struct cerebrod_event_to_send *ets;
      ListIterator eitr;

      Pthread_mutex_lock(&event_queue_lock);
      assert(event_queue);
      while (list_count(event_queue) == 0)
        Pthread_cond_wait(&event_queue_cond, &event_queue_lock);

      eitr = List_iterator_create(event_queue);
      while ((ets = list_next(eitr)))
        {
          List connections;

          _event_dump(ets->event);
          
          if ((connections = Hash_find(event_connections, ets->event_name)))
            {
              char buf[CEREBRO_MAX_PACKET_LEN];
              int elen;

              if ((elen = _event_marshall(ets->event, 
                                          buf, 
                                          CEREBRO_MAX_PACKET_LEN)) > 0)
                {
                  ListIterator citr;
                  int *fd;

                  citr = List_iterator_create(connections);
                  while ((fd = list_next(citr)))
                    {
                      if (fd_write_n(*fd, buf, elen) < 0)
                        {
                          CEREBRO_DBG(("fd_write_n: %s", strerror(errno)));
                          if (errno == EPIPE
                              || errno == EINVAL
                              || errno == EBADF
                              || errno == ENODEV
                              || errno == ENETDOWN
                              || errno == ENETUNREACH)
                            List_delete(citr);
                          continue;
                        }
                    }
                  List_iterator_destroy(citr);
                }
            }
          List_delete(eitr);
        }
      List_iterator_destroy(eitr);

      Pthread_mutex_unlock(&event_queue_lock);
    }

  return NULL;			/* NOT REACHED */
}

/*
 * _event_server_initialize
 *
 * perform metric server initialization
 */
static void
_event_server_initialize(void)
{
  int event_names_count;

  Pthread_mutex_lock(&event_server_init_lock);
  if (event_server_init)
    goto out;

  Signal(SIGPIPE, SIG_IGN);

  /* achu:
   *
   * This hash must be created in this initialize and not the queue
   * initialize, b/c the event names list is generated during the
   * listener initialization (which is done after the queue
   * initialization).
   */
  event_names_count = List_count(event_names);
  event_connections = Hash_create(event_names_count,
                                  (hash_key_f)hash_key_string,
                                  (hash_cmp_f)strcmp,
                                  (hash_del_f)list_destroy);

  event_server_init++;
  Pthread_cond_signal(&event_server_init_cond);
 out:
  Pthread_mutex_unlock(&event_server_init_lock);
}

/*
 * _event_server_setup_socket
 *
 * Create and setup the server socket.  Do not use wrappers in this
 * function.  We want to give the server additional chances to
 * "survive" an error condition.
 *
 * Returns file descriptor on success, -1 on error
 *
 * Note: num parameter unused in this function, here only to match function prototype
 */
static int
_event_server_setup_socket(int num)
{
  struct sockaddr_in addr;
  int fd, optval = 1;

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      CEREBRO_DBG(("socket: %s", strerror(errno)));
      goto cleanup;
    }

  /* For quick start/restart */
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) < 0)
    {
      CEREBRO_DBG(("setsockopt: %s", strerror(errno)));
      goto cleanup;
    }

  memset(&addr, '\0', sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(conf.event_server_port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
    {
      CEREBRO_DBG(("bind: %s", strerror(errno)));
      goto cleanup;
    }

  if (listen(fd, CEREBROD_EVENT_SERVER_BACKLOG) < 0)
    {
      CEREBRO_DBG(("listen: %s", strerror(errno)));
      goto cleanup;
    }

  return fd;

 cleanup:
  close(fd);
  return -1;
}

void *
cerebrod_event_server(void *arg)
{
  int server_fd;

  _event_server_initialize();

  if ((server_fd = _event_server_setup_socket(0)) < 0)
    CEREBRO_EXIT(("event server fd setup failed"));

  for (;;)
    {
      int fd, client_addr_len;
      struct sockaddr_in client_addr;
      
      client_addr_len = sizeof(struct sockaddr_in);
      if ((fd = accept(server_fd,
                       (struct sockaddr *)&client_addr,
                       &client_addr_len)) < 0)
        server_fd = cerebrod_reinit_socket(server_fd,
                                           0,
                                           _event_server_setup_socket,
                                           "event_server: accept");
      if (fd < 0)
        continue;
      
    }

  return NULL;			/* NOT REACHED */
}

#endif /* !WITH_CEREBROD_SPEAKER_ONLY */
