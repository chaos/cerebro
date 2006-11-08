/*****************************************************************************\
 *  $Id: cerebrod_event_server.c,v 1.1.2.14 2006-11-08 00:19:02 chu11 Exp $
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

#include "cerebrod.h"
#include "cerebrod_config.h"
#include "cerebrod_event_server.h"
#include "cerebrod_util.h"

#include "debug.h"
#include "event_module.h"
#include "fd.h"
#include "hash.h"
#include "list.h"
#include "data_util.h"
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
 * event_connections_index
 * event_connections_lock
 *
 * hash of file descriptors to send event info to.
 */
List event_connections = NULL;
hash_t event_connections_index = NULL;
pthread_mutex_t event_connections_lock = PTHREAD_MUTEX_INITIALIZER;
 
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
        case CEREBRO_DATA_VALUE_TYPE_NONE:
          break;
        case CEREBRO_DATA_VALUE_TYPE_INT32:
          fprintf(stderr, "* Value = %d",
                  *((int32_t *)event->event_value));
          break;
        case CEREBRO_DATA_VALUE_TYPE_U_INT32:
          fprintf(stderr, "* Value = %u",
                  *((u_int32_t *)event->event_value));
          break;
        case CEREBRO_DATA_VALUE_TYPE_FLOAT:
          fprintf(stderr, "* Value = %f",
                  *((float *)event->event_value));
          break;
        case CEREBRO_DATA_VALUE_TYPE_DOUBLE:
          fprintf(stderr, "* Value = %f",
                  *((double *)event->event_value));
          break;
        case CEREBRO_DATA_VALUE_TYPE_STRING:
          /* Watch for NUL termination */
          buf = Malloc(event->event_value_len + 1);
          memset(buf, '\0', event->event_value_len + 1);
          memcpy(buf,
                 event->event_value,
                 event->event_value_len);
          fprintf(stderr, "* Value = %s", buf);
          Free(buf);
          break;
#if SIZEOF_LONG == 4
        case CEREBRO_DATA_VALUE_TYPE_INT64:
          fprintf(stderr, "* Value = %lld",
                  *((int64_t *)event->event_value));
          break;
        case CEREBRO_DATA_VALUE_TYPE_U_INT64:
          fprintf(stderr, "* Value = %llu",
                  *((u_int64_t *)event->event_value));
          break;
#else  /* SIZEOF_LONG == 8 */
        case CEREBRO_DATA_VALUE_TYPE_INT64:
          fprintf(stderr, "* Value = %ld",
                  *((int64_t *)event->event_value));
          break;
        case CEREBRO_DATA_VALUE_TYPE_U_INT64:
          fprintf(stderr, "* Value = %lu",
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
  c += Marshall_u_int32(event->err_code, buf + c, buflen - c);
  bufPtr = event->nodename;
  bufPtrlen = sizeof(event->nodename);
  c += Marshall_buffer(bufPtr, bufPtrlen, buf + c, buflen - c);
  bufPtr = event->event_name;
  bufPtrlen = sizeof(event->event_name);
  c += Marshall_buffer(bufPtr, bufPtrlen, buf + c, buflen - c);

  if ((n = marshall_data(event->event_value_type,
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
          
          Pthread_mutex_lock(&event_connections_lock);
          if ((connections = Hash_find(event_connections_index, ets->event_name)))
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
                            {
#if CEREBRO_DEBUG
                              if (conf.event_server_debug)
                                {
                                  Pthread_mutex_lock(&debug_output_mutex);
                                  fprintf(stderr, "**************************************\n");
                                  fprintf(stderr, "* Event Connection Died: errno = %d\n", errno);
                                  fprintf(stderr, "**************************************\n");
                                  Pthread_mutex_unlock(&debug_output_mutex);
                                }
#endif /* CEREBRO_DEBUG */
                              List_delete(citr);
                            }
                          continue;
                        }
                    }
                  List_iterator_destroy(citr);
                }
            }
          Pthread_mutex_unlock(&event_connections_lock);

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

  /* achu:
   *
   * This hash must be created in this initialize and not the
   * queue_monitor initialize, b/c the event names list is generated
   * during the listener initialization (which is done after the
   * queue_monitor initialization but before the server
   * initialization).
   */
  assert(event_names);
  event_names_count = List_count(event_names);
  event_connections = List_create((ListDelF)free);
  event_connections_index = Hash_create(event_names_count,
                                        (hash_key_f)hash_key_string,
                                        (hash_cmp_f)strcmp,
                                        (hash_del_f)list_destroy);

  Signal(SIGPIPE, SIG_IGN);

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

static void
_delete_event_connection_fd(int fd)
{
  struct cerebrod_event_connection_data *ecd;
  ListIterator eitr;
#if CEREBRO_DEBUG
  int rv;
#endif /* CEREBRO_DEBUG */

  assert(fd >= 0);

#if CEREBRO_DEBUG
  /* Should be called with lock already set */
  rv = Pthread_mutex_trylock(&event_connections_lock);
  if (rv != EBUSY)
    CEREBRO_EXIT(("mutex not locked: rv=%d", rv));
#endif /* CEREBRO_DEBUG */
  
  eitr = List_iterator_create(event_connections);
  while ((ecd = list_next(eitr)))
    {
      if (ecd->fd == fd)
        {
          List connections;
          if ((connections = Hash_find(event_connections_index, 
                                       ecd->event_name)))
            {
              ListIterator citr;
              int *fdPtr;

              citr = List_iterator_create(connections);
              while ((fdPtr = list_next(citr)))
                {
                  if (*fdPtr == fd) 
                    {
                      List_delete(citr);
                      break;
                    }
                }
              List_iterator_destroy(citr);
            }
          List_delete(eitr);
          break;
        }
    }
  List_iterator_destroy(eitr);
}

/*
 * _event_server_response_marshall
 *
 * marshall contents of a event response packet buffer
 *
 * Returns length written to buffer on success, -1 on error
 */
static int
_event_server_response_marshall(struct cerebro_event_server_response *res,
                                char *buf,
                                unsigned int buflen)
{
  int len = 0;
  
  assert(res && buf && buflen >= CEREBRO_EVENT_SERVER_RESPONSE_LEN);
  
  memset(buf, '\0', buflen);
  len += Marshall_int32(res->version, buf + len, buflen - len);
  len += Marshall_u_int32(res->err_code, buf + len, buflen - len);
  return len;
}

/*
 * _event_server_response
 *
 * respond to the event_server_request with an error
 *
 * Return 0 on success, -1 on error
 */
static int
_event_server_response(int fd, int32_t version, u_int32_t err_code)
{
  struct cerebro_event_server_response res;
  char buf[CEREBRO_MAX_PACKET_LEN];
  int res_len;

  assert(fd >= 0
         && err_code >= CEREBRO_EVENT_SERVER_PROTOCOL_ERR_SUCCESS
         && err_code <= CEREBRO_EVENT_SERVER_PROTOCOL_ERR_INTERNAL_ERROR);

  memset(&res, '\0', CEREBRO_EVENT_SERVER_RESPONSE_LEN);
  res.version = version;
  res.err_code = err_code;

  if ((res_len = _event_server_response_marshall(&res, 
                                                 buf, 
                                                 CEREBRO_MAX_PACKET_LEN)) < 0)
    return -1;

  if (fd_write_n(fd, buf, res_len) < 0)
    {
      CEREBRO_DBG(("fd_write_n: %s", strerror(errno)));
      return -1;
    }

  return 0;
}

/*
 * _event_server_request_check_version
 *
 * Check that the version is correct prior to unmarshalling
 *
 * Returns 0 if version is correct, -1 if not
 */
static int
_event_server_request_check_version(const char *buf,
                                    unsigned int buflen,
                                    int32_t *version)
{
  assert(buflen >= sizeof(int32_t) && version);

  if (!Unmarshall_int32(version, buf, buflen))
    {
      CEREBRO_DBG(("version could not be unmarshalled"));
      return -1;
    }

  if (*version != CEREBRO_EVENT_SERVER_PROTOCOL_VERSION)
    return -1;
  
  return 0;
}

/*
 * _event_server_request_unmarshall
 *
 * unmarshall contents of a event server request packet buffer
 *
 * Returns 0 on success, -1 on error
 */
static int
_event_server_request_unmarshall(struct cerebro_event_server_request *req,
                                 const char *buf,
                                 unsigned int buflen)
{
  int bufPtrlen, c = 0;
  char *bufPtr;
  
  assert(req && buf);

  bufPtr = req->event_name;
  bufPtrlen = sizeof(req->event_name);
  c += Unmarshall_int32(&(req->version), buf + c, buflen - c);
  c += Unmarshall_buffer(bufPtr, bufPtrlen, buf + c, buflen - c);
  c += Unmarshall_u_int32(&(req->flags), buf + c, buflen - c);

  if (c != CEREBRO_EVENT_SERVER_REQUEST_PACKET_LEN)
    return -1;

  return 0;
}

/*
 * _event_server_request_dump
 *
 * dump contents of a event server request
 */
static void
_event_server_request_dump(struct cerebro_event_server_request *req)
{
#if CEREBRO_DEBUG
  char event_name_buf[CEREBRO_MAX_EVENT_NAME_LEN+1];

  assert(req);

  if (!(conf.debug && conf.event_server_debug))
    return;

  Pthread_mutex_lock(&debug_output_mutex);
  fprintf(stderr, "**************************************\n");
  fprintf(stderr, "* Event Server Request Received:\n");
  fprintf(stderr, "* ------------------------\n");
  fprintf(stderr, "* Version: %d\n", req->version);
  /* Guarantee ending '\0' character */
  memset(event_name_buf, '\0', CEREBRO_MAX_EVENT_NAME_LEN+1);
  memcpy(event_name_buf, req->event_name, CEREBRO_MAX_EVENT_NAME_LEN);
  fprintf(stderr, "* Event_name: %s\n", event_name_buf);
  fprintf(stderr, "* Flags: %x\n", req->flags);
  fprintf(stderr, "**************************************\n");
  Pthread_mutex_unlock(&debug_output_mutex);
#endif /* CEREBRO_DEBUG */
}

/* 
 * _event_names_compare
 *
 * Comparison for list_find
 *
 * Return 1 if key matches, 0 if not
 */
static int
_event_names_compare(void *x, void *key)
{
  assert(x);
  assert(key);

  if (!strcmp((char *)x, (char *)key))
    return 1;
  return 0;
}

/*
 * _event_server_service_connection
 *
 * Service a connection from a client to receive event packets.  Use
 * wrapper functions minimally, b/c we want to return errors to the
 * user instead of exitting with errors.
 *
 */
static void
_event_server_service_connection(int fd)
{
  int recv_len;
  struct cerebro_event_server_request req;
  struct cerebrod_event_connection_data *ecd = NULL;
  char buf[CEREBRO_MAX_PACKET_LEN];
  char event_name_buf[CEREBRO_MAX_EVENT_NAME_LEN+1];
  char *event_name_ptr = NULL;
  int32_t version;
  int *fdptr;
  List connections = NULL;

  assert(fd >= 0);

  memset(&req, '\0', sizeof(struct cerebro_event_server_request));
  if ((recv_len = receive_data(fd,
                               CEREBRO_EVENT_SERVER_REQUEST_PACKET_LEN,
                               buf,
                               CEREBRO_MAX_PACKET_LEN,
                               CEREBRO_EVENT_SERVER_PROTOCOL_CLIENT_TIMEOUT_LEN,
                               NULL)) < 0)
    goto cleanup;

  if (recv_len < sizeof(version))
    goto cleanup;

  if (_event_server_request_check_version(buf, recv_len, &version) < 0)
    {
      _event_server_response(fd,
                             version,
                             CEREBRO_EVENT_SERVER_PROTOCOL_ERR_VERSION_INVALID);
      goto cleanup;
    }

  if (recv_len != CEREBRO_EVENT_SERVER_REQUEST_PACKET_LEN)
    {
      _event_server_response(fd,
                             version,
                             CEREBRO_EVENT_SERVER_PROTOCOL_ERR_PACKET_INVALID);
      goto cleanup;
    }

  if (_event_server_request_unmarshall(&req, buf, recv_len) < 0)
    {
      _event_server_response(fd,
                             version,
                             CEREBRO_EVENT_SERVER_PROTOCOL_ERR_PACKET_INVALID);
      goto cleanup;
    }

  _event_server_request_dump(&req);

  /* Guarantee ending '\0' character */
  memset(event_name_buf, '\0', CEREBRO_MAX_EVENT_NAME_LEN+1);
  memcpy(event_name_buf, req.event_name, CEREBRO_MAX_EVENT_NAME_LEN);

  if (!strlen(event_name_buf))
    {
      _event_server_response(fd,
                             req.version,
                             CEREBRO_EVENT_SERVER_PROTOCOL_ERR_EVENT_INVALID);
      goto cleanup;
    }

  /* Event names is not changeable - so no need for a lock */
  if (!(event_name_ptr = list_find_first(event_names, 
                                         _event_names_compare,
                                         event_name_buf)))
    {
      _event_server_response(fd,
                             req.version,
                             CEREBRO_EVENT_SERVER_PROTOCOL_ERR_EVENT_INVALID);
      goto cleanup;
    }
  
  if (!(ecd = (struct cerebrod_event_connection_data *)malloc(sizeof(struct cerebrod_event_connection_data))))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      _event_server_response(fd,
                             req.version,
                             CEREBRO_EVENT_SERVER_PROTOCOL_ERR_INTERNAL_ERROR);
      goto cleanup;
    }
  
  ecd->event_name = event_name_ptr;
  ecd->fd = fd;

  if (!list_append(event_connections, ecd))
    {
      CEREBRO_DBG(("list_append: %s", strerror(errno)));
      _event_server_response(fd,
                             req.version,
                             CEREBRO_EVENT_SERVER_PROTOCOL_ERR_INTERNAL_ERROR);
      goto cleanup;
    }

  if (!(connections = Hash_find(event_connections_index, 
                                ecd->event_name)))
    {
      if (!(connections = list_create((ListDelF)free)))
        {
          CEREBRO_DBG(("list_create: %s", strerror(errno)));
          _event_server_response(fd,
                                 req.version,
                                 CEREBRO_EVENT_SERVER_PROTOCOL_ERR_INTERNAL_ERROR);
          goto cleanup;
        }

      if (!Hash_insert(event_connections_index, ecd->event_name, connections))
        {
          CEREBRO_DBG(("list_create: %s", strerror(errno)));
          _event_server_response(fd,
                                 req.version,
                                 CEREBRO_EVENT_SERVER_PROTOCOL_ERR_INTERNAL_ERROR);
          list_destroy(connections);
          goto cleanup;
        }
    }

  if (!(fdptr = (int *)malloc(sizeof(int))))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      _event_server_response(fd,
                             req.version,
                             CEREBRO_EVENT_SERVER_PROTOCOL_ERR_INTERNAL_ERROR);
      goto cleanup;
    }

  *fdptr = fd;
  Pthread_mutex_lock(&event_connections_lock);
  if (!list_append(connections, fdptr))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      _event_server_response(fd,
                             req.version,
                             CEREBRO_EVENT_SERVER_PROTOCOL_ERR_INTERNAL_ERROR);
      goto cleanup;
      
    }
  Pthread_mutex_unlock(&event_connections_lock);
  /* Clear this pointer so we know it's stored away in a list */
  fdptr = NULL;

  _event_server_response(fd,
                         req.version,
                         CEREBRO_EVENT_SERVER_PROTOCOL_ERR_SUCCESS);

  return;
  
 cleanup:
  if (ecd)
    free(ecd);
  if (fdptr)
    free(fdptr);
  close(fd);
  return;
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
      ListIterator eitr;
      struct cerebrod_event_connection_data *ecd;
      struct pollfd *pfds;
      int pfdslen;
      int i;

       /* Note that the list_count won't grow larger after the first
        * mutex block, b/c the cerebrod_event_queue_monitor thread can
        * never add to the event_connections.  It can only shrink it.
        */
      Pthread_mutex_lock(&event_connections_lock);
      pfdslen = List_count(event_connections);
      Pthread_mutex_unlock(&event_connections_lock);

      /* The + 1 is b/c of the server_fd. */
      pfdslen++;

      pfds = Malloc(sizeof(struct pollfd) * pfdslen);
      memset(pfds, '\0', sizeof(struct pollfd) * pfdslen);

      pfds[0].fd = server_fd;
      pfds[0].events = POLLIN;
      pfds[0].revents = 0;

      i = 1;
      Pthread_mutex_lock(&event_connections_lock);
      eitr = List_iterator_create(event_connections);
      while ((ecd = list_next(eitr)))
        {
          pfds[i].fd = ecd->fd;
          pfds[i].events = POLLIN;
          pfds[i].revents = 0;
          i++;
        }
      List_iterator_destroy(eitr);
      Pthread_mutex_unlock(&event_connections_lock);
      
      Poll(pfds, pfdslen, -1);

      /* Deal with the server fd first */
      if (pfds[0].revents & POLLERR)
        CEREBRO_DBG(("server_fd POLLERR"));
      else if (pfds[0].revents & POLLIN)
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
          if (fd >= 0)
            _event_server_service_connection(fd);
        }

      /* Deal with the connecting fds */
      for (i = 1; i < pfdslen; i++)
        {
          if (pfds[i].revents & POLLERR)
            {
              CEREBRO_DBG(("fd = %d POLLERR", pfds[i].fd));
              Pthread_mutex_lock(&event_connections_lock);
              _delete_event_connection_fd(pfds[i].fd);
              Pthread_mutex_unlock(&event_connections_lock);
              continue;
            }

          if (pfds[i].revents & POLLIN)
            {
              char buf[CEREBRO_MAX_PACKET_LEN];
              int n;

              /* We should not expect any actual data.  If
               * we get some, just eat it and move on.
               *
               * The common situation is that the client
               * closes the connection.  So we need to delete
               * our fd.
               */

              n = fd_read_n(pfds[i].fd, 
                            buf,
                            CEREBRO_MAX_PACKET_LEN);
              if (n < 0)
                CEREBRO_DBG(("fd_read_n = %s", strerror(errno)));

              if (n <= 0)
                {
#if CEREBRO_DEBUG
                  if (conf.debug && conf.event_server_debug)
                    {
                      Pthread_mutex_lock(&debug_output_mutex);
                      fprintf(stderr, "**************************************\n");
                      fprintf(stderr, "* Event Server Close Fd: %d\n", pfds[i].fd);
                      fprintf(stderr, "**************************************\n");
                      Pthread_mutex_unlock(&debug_output_mutex);
                    }
#endif /* CEREBRO_DEBUG */
                  Pthread_mutex_lock(&event_connections_lock);
                  _delete_event_connection_fd(pfds[i].fd);
                  Pthread_mutex_unlock(&event_connections_lock);
                }
            }
        }
      
      Free(pfds);
    }

  return NULL;			/* NOT REACHED */
}

#endif /* !WITH_CEREBROD_SPEAKER_ONLY */
