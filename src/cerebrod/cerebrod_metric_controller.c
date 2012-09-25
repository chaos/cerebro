/*****************************************************************************\
 *  $Id: cerebrod_metric_controller.c,v 1.52 2010-02-02 01:01:20 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2011 Lawrence Livermore National Security, LLC.
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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <limits.h>

#include <sys/socket.h>

#include <assert.h>

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"

#include "cerebro/cerebro_metric_control_protocol.h"
#include "cerebro/cerebrod_message_protocol.h"

#include "cerebrod.h"
#include "cerebrod_config.h"
#include "cerebrod_listener_data.h"
#include "cerebrod_metric_controller.h"
#include "cerebrod_speaker.h"
#include "cerebrod_speaker_data.h"
#include "cerebrod_util.h"

#include "debug.h"
#include "fd.h"
#include "list.h"
#include "data_util.h"
#include "network_util.h"
#include "wrappers.h"

extern struct cerebrod_config conf;
extern pthread_mutex_t debug_output_mutex;

/* 
 * Speaker Data
 */
extern List metric_list;
extern int metric_list_size;
extern pthread_mutex_t metric_list_lock;

/* 
 * Listener Data
 */
extern hash_t listener_data;
extern pthread_mutex_t listener_data_lock;
extern hash_t metric_names;
extern pthread_mutex_t metric_names_lock;

#define CEREBROD_METRIC_CONTROLLER_BACKLOG    5

/*
 * metric_controller_init
 * metric_controller_init_cond
 * metric_controller_init_lock
 *
 * variables for synchronizing initialization between different pthreads
 * and signaling when it is complete
 */
int metric_controller_init = 0;
pthread_cond_t metric_controller_init_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t metric_controller_init_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * _metric_controller_initialize
 *
 * perform metric server initialization
 */
static void
_metric_controller_initialize(void)
{
  Pthread_mutex_lock(&metric_controller_init_lock);
  if (metric_controller_init)
    goto out;

  cerebrod_speaker_data_initialize();

  Signal(SIGPIPE, SIG_IGN);

  metric_controller_init++;
  Pthread_cond_signal(&metric_controller_init_cond);
 out:
  Pthread_mutex_unlock(&metric_controller_init_lock);
}

/*
 * _metric_controller_setup_socket
 *
 * Create and setup the controller socket.  Do not use wrappers in this
 * function.  We want to give the controller additional chances to
 * "survive" an error condition.
 *
 * Returns file descriptor on success, -1 on error
 *
 * Note: num parameter unused in this function, here only to match function prototype
 */
static int
_metric_controller_setup_socket(int num)
{
  struct sockaddr_un addr;
  int fd;
  
  if ((fd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
    {
      CEREBRO_ERR(("socket: %s", strerror(errno)));
      return -1;
    }

  if (strlen(CEREBRO_METRIC_CONTROL_PATH) >= sizeof(addr.sun_path))
    {
      CEREBRO_DBG(("path '%s' too long", CEREBRO_METRIC_CONTROL_PATH));
      goto cleanup;
    }

  /* unlink is allowed to fail in some situations */
  if (unlink(CEREBRO_METRIC_CONTROL_PATH) < 0)
    {
      if (errno != ENOENT)
        CEREBRO_EXIT(("unlink: %s", strerror(errno)));
    }
  
  memset(&addr, '\0', sizeof(struct sockaddr_un));
  addr.sun_family = AF_LOCAL;
  strncpy(addr.sun_path, 
          CEREBRO_METRIC_CONTROL_PATH, 
          sizeof(addr.sun_path));
  
  if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) < 0)
    {
      CEREBRO_ERR(("bind: %s", strerror(errno)));
      goto cleanup;
    }
  
  if (listen(fd, CEREBROD_METRIC_CONTROLLER_BACKLOG) < 0)
    {
      CEREBRO_ERR(("listen: %s", strerror(errno)));
      goto cleanup;
    }
  
  return fd;

 cleanup:
  /* ignore potential error, we're in the error path already */
  close(fd);
  return -1;
}

/*
 * _metric_control_request_check_version
 *
 * Check that the version is correct prior to unmarshalling
 *
 * Returns 0 if version is correct, -1 if not
 */
static int
_metric_control_request_check_version(const char *buf, 
                                      unsigned int buflen, 
                                      int32_t *version)
{
  assert(buflen >= sizeof(int32_t) && version);

  if (!Unmarshall_int32(version, buf, buflen))
    {
      CEREBRO_DBG(("version could not be unmarshalled"));
      return -1;
    }

  if (*version != CEREBRO_METRIC_CONTROL_PROTOCOL_VERSION)
    return -1;

  return 0;
}

/*
 * _metric_control_request_header_unmarshall
 *
 * unmarshall contents of a metric control request packet buffer
 *
 * Returns 0 on success, -1 on error
 */
static int
_metric_control_request_header_unmarshall(struct cerebro_metric_control_request *req,
                                          const char *buf,
                                          unsigned int buflen)
{
  int bufPtrlen, c = 0;
  char *bufPtr;
  
  assert(req && buf);
  
  bufPtr = req->metric_name;
  bufPtrlen = sizeof(req->metric_name);
  c += Unmarshall_int32(&(req->version), buf + c, buflen - c);
  c += Unmarshall_int32(&(req->command), buf + c, buflen - c);
  c += Unmarshall_u_int32(&(req->flags), buf + c, buflen - c);
  c += Unmarshall_buffer(bufPtr, bufPtrlen, buf + c, buflen - c);
  c += Unmarshall_u_int32(&(req->metric_value_type), buf + c, buflen - c);
  c += Unmarshall_u_int32(&(req->metric_value_len), buf + c, buflen - c);
  
  if (c != CEREBRO_METRIC_CONTROL_REQUEST_HEADER_LEN)
    return -1;
  
  return 0;
}

/*
 * _metric_control_response_marshall
 *
 * marshall contents of a metric err response packet buffer
 *
 * Returns length written to buffer on success, -1 on error
 */
static int
_metric_control_response_marshall(struct cerebro_metric_control_response *res,
                                  char *buf,
                                  unsigned int buflen)
{
  int len = 0;

  assert(res && buf && buflen >= CEREBRO_METRIC_CONTROL_RESPONSE_LEN);

  memset(buf, '\0', buflen);
  len += Marshall_int32(res->version, buf + len, buflen - len);
  len += Marshall_u_int32(res->err_code, buf + len, buflen - len);
  return len;
}


/*  
 * _metric_control_request_dump
 *
 * dump contents of a metric controller request
 */
static void
_metric_control_request_dump(struct cerebro_metric_control_request *req)
{
  char metric_name_buf[CEREBRO_MAX_METRIC_NAME_LEN+1];

  assert(req);

  if (!(conf.debug && conf.metric_controller_debug))
    return;
  
  Pthread_mutex_lock(&debug_output_mutex);
  fprintf(stderr, "**************************************\n");
  fprintf(stderr, "* Metric Controller Request Received:\n");
  fprintf(stderr, "* ------------------------\n");
  fprintf(stderr, "* Version: %d\n", req->version);
  fprintf(stderr, "* Command: %d\n", req->command);
  fprintf(stderr, "* Flags: %X\n", req->flags);
  /* Guarantee ending '\0' character */
  memset(metric_name_buf, '\0', CEREBRO_MAX_METRIC_NAME_LEN+1);
  memcpy(metric_name_buf, req->metric_name, CEREBRO_MAX_METRIC_NAME_LEN);
  fprintf(stderr, "* Metric_name: %s\n", metric_name_buf);
  fprintf(stderr, "* Metric_value_type: %x\n", req->metric_value_type);
  fprintf(stderr, "* metric_value_len: %d\n", req->metric_value_len);
  fprintf(stderr, "**************************************\n");
  Pthread_mutex_unlock(&debug_output_mutex);
}

/* 
 * _send_metric_control_response
 *
 * Send metric control responses with the appropriate err code
 *
 * Returns 0 on success, -1 on error
 */
static int
_send_metric_control_response(int fd, int32_t version, u_int32_t err_code)
{
  struct cerebro_metric_control_response res;
  char buf[CEREBRO_MAX_PACKET_LEN];
  int res_len, buflen;

  assert(fd >= 0
         && err_code >= CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_SUCCESS
         && err_code <= CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_INTERNAL_ERROR);

  memset(&res, '\0', CEREBRO_METRIC_CONTROL_RESPONSE_LEN);
  res.version = version;
  res.err_code = err_code;

  buflen = CEREBRO_MAX_PACKET_LEN;
  if ((res_len = _metric_control_response_marshall(&res, buf, buflen)) < 0)
    return -1;
  
  if (fd_write_n(fd, buf, res_len) < 0)
    {
      CEREBRO_ERR(("fd_write_n: %s", strerror(errno)));
      return -1;
    }

  return 0;
}

/* 
 * _find_speaker_metric_info
 *
 * Find metric_info for 'metric_name' in the metric_list
 *
 * Returns metric_info on success, NULL if not found
 */
static struct cerebrod_speaker_metric_info *
_find_speaker_metric_info(const char *metric_name)
{
  struct cerebrod_speaker_metric_info *metric_info = NULL;
  ListIterator itr = NULL;
#if CEREBRO_DEBUG
  int rv;
#endif /* CEREBRO_DEBUG */

  assert(metric_name);

#if CEREBRO_DEBUG
  /* Should be called with lock already set */
  rv = Pthread_mutex_trylock(&metric_list_lock);
  if (rv != EBUSY)
    CEREBRO_EXIT(("mutex not locked: rv=%d", rv));
#endif /* CEREBRO_DEBUG */

  itr = List_iterator_create(metric_list);
  while ((metric_info = list_next(itr)))
    {
      if (!strcmp(metric_info->metric_name, metric_name))
        break;
    }
  List_iterator_destroy(itr);
  
  return metric_info;
}

/* 
 * _find_metric_name
 *
 * Find the metric name in the metric list
 *
 * Returns 1 if it matches, 0 otherwise
 */
static int
_find_metric_name(void *x, void *key)
{
  struct cerebrod_speaker_metric_info *metric_info = NULL;

  assert(x);

  metric_info = (struct cerebrod_speaker_metric_info *)x;
  
  return (!strcmp(metric_info->metric_name, (char *)key)) ? 1 : 0;
}

/* 
 * _register_metric
 *
 * Register a new metric
 *
 * Returns 0 on success, -1 on error
 */
static int
_register_metric(int fd, int32_t version, const char *metric_name)
{
  struct cerebrod_speaker_metric_info *metric_info;
  int rv = -1;

  assert(fd >= 0 && metric_name && conf.speak);

  Pthread_mutex_lock(&metric_list_lock);

  if (_find_speaker_metric_info(metric_name))
    {
      _send_metric_control_response(fd,
                                    version,
                                    CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_METRIC_INVALID);
      goto cleanup;
    }

  metric_info = Malloc(sizeof(struct cerebrod_speaker_metric_info));
  metric_info->metric_name = Strdup(metric_name);
  metric_info->metric_origin = CEREBROD_METRIC_SPEAKER_ORIGIN_USERSPACE;
  /* 
   * Setting next_call_time to UINT_MAX means the data will never
   * be sent.
   */
  metric_info->next_call_time = UINT_MAX;
  metric_info->metric_value_type = CEREBRO_DATA_VALUE_TYPE_NONE;
  metric_info->metric_value_len = 0;
  metric_info->metric_value = NULL;
  List_append(metric_list, metric_info);
  metric_list_size++;
  cerebrod_speaker_data_metric_list_sort();

  rv = 0;
 cleanup:
  Pthread_mutex_unlock(&metric_list_lock);
  return rv;
}

/* 
 * _unregister_metric
 *
 * Unregister a existing metric
 *
 * Returns 0 on success, -1 on error
 */
static int
_unregister_metric(int fd, int32_t version, const char *metric_name)
{
  struct cerebrod_speaker_metric_info *metric_info;
  int rv = -1;

  assert(fd >= 0 && metric_name && conf.speak);

  Pthread_mutex_lock(&metric_list_lock);
  if (!(metric_info = _find_speaker_metric_info(metric_name)))
    {
      _send_metric_control_response(fd,
                                    version,
                                    CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_METRIC_INVALID);
      goto cleanup;
    }

  if (!(metric_info->metric_origin & CEREBROD_METRIC_SPEAKER_ORIGIN_USERSPACE))
    {
      _send_metric_control_response(fd,
                                    version,
                                    CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_METRIC_INVALID);
      goto cleanup;
    }

  if (List_delete_all(metric_list, _find_metric_name, (void *)metric_name) != 1)
    CEREBRO_DBG(("invalid list_delete_all"));

  metric_list_size--;

  rv = 0;
 cleanup:
  Pthread_mutex_unlock(&metric_list_lock);
  return rv;
}

/* 
 * _receive_metric_value
 *
 * Receive the metric value from the fd
 *
 * Returns 0 on success, -1 on error
 */
static int
_receive_metric_value(int fd, 
                      int32_t version, 
                      struct cerebro_metric_control_request *req)
{
  char *vbuf = NULL, *mvalue = NULL;
  int n, vbytes_read, rv = -1;
  u_int32_t mtype, mlen;

  assert(fd >= 0 && req && req->metric_value_len);

  vbuf = Malloc(req->metric_value_len);
  
  if ((vbytes_read = receive_data(fd,
                                  req->metric_value_len,
                                  vbuf,
                                  req->metric_value_len,
                                  CEREBRO_METRIC_CONTROL_PROTOCOL_CLIENT_TIMEOUT_LEN,
                                  NULL)) < 0)
    {
      _send_metric_control_response(fd,
                                    version,
                                    CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_PACKET_INVALID);
      goto cleanup;
    }
  
  if (vbytes_read != req->metric_value_len)
    {
      _send_metric_control_response(fd,
                                    version,
                                    CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_PACKET_INVALID);
      goto cleanup;
    }
  
  mvalue = Malloc(req->metric_value_len);

  mtype = req->metric_value_type;
  mlen = req->metric_value_len;

  if ((n = unmarshall_data_value(mtype,
                                 mlen,
                                 mvalue,
                                 req->metric_value_len,
                                 vbuf,
                                 vbytes_read,
                                 NULL)) < 0)
    goto cleanup;
  
  req->metric_value = mvalue;
  
  rv = 0;
 cleanup:
  Free(vbuf);
  return rv;
}

/* 
 * _send_message_now
 *
 * Send the metric info now rather than waiting for the next message
 *
 * Returns 0 on success, -1 on error
 */
static int
_send_message_now(int fd, 
                  int32_t version,
                  const char *metric_name,
                  u_int32_t metric_value_type,
                  u_int32_t metric_value_len,
                  void *metric_value)
{
  struct cerebrod_message *msg = NULL;
  struct cerebrod_message_metric *mm = NULL;
  char nodename[CEREBRO_MAX_NODENAME_LEN+1];
  int rv = -1;

  assert(fd >= 0 
         && metric_name 
         && conf.speak);

  if (!(msg = (struct cerebrod_message *)malloc(sizeof(struct cerebrod_message))))
    {
      CEREBRO_ERR(("malloc: %s", strerror(errno)));
      _send_metric_control_response(fd,
                                    version,
                                    CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_INTERNAL_ERROR);
      goto cleanup;
    }
  
  memset(nodename, '\0', CEREBRO_MAX_NODENAME_LEN+1);
  if (gethostname(nodename, CEREBRO_MAX_NODENAME_LEN) < 0)
    {
      CEREBRO_ERR(("gethostname: %s", strerror(errno)));
      _send_metric_control_response(fd,
                                    version,
                                    CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_INTERNAL_ERROR);
      goto cleanup;
    }
  
  msg->version = CEREBROD_MESSAGE_PROTOCOL_VERSION;
  memcpy(msg->nodename, nodename, CEREBRO_MAX_NODENAME_LEN);

  msg->metrics_len = 1;
  if (!(msg->metrics = (struct cerebrod_message_metric **)malloc(sizeof(struct cerebrod_message_metric *)*(msg->metrics_len + 1))))
    {
      CEREBRO_ERR(("malloc: %s", strerror(errno)));
      _send_metric_control_response(fd,
                                    version,
                                    CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_INTERNAL_ERROR);
      goto cleanup;
    }
  memset(msg->metrics, '\0', sizeof(struct cerebrod_message_metric *)*(msg->metrics_len + 1));

  if (!(mm = (struct cerebrod_message_metric *)malloc(sizeof(struct cerebrod_message_metric))))
    {
      CEREBRO_ERR(("malloc: %s", strerror(errno)));
      _send_metric_control_response(fd,
                                    version,
                                    CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_INTERNAL_ERROR);
      goto cleanup;
    }
  memset(mm, '\0', sizeof(struct cerebrod_message_metric));

  /* need not overflow */
  strncpy(mm->metric_name, metric_name, CEREBRO_MAX_METRIC_NAME_LEN);
  
  mm->metric_value_type = metric_value_type;
  mm->metric_value_len = metric_value_len;
  mm->metric_value = metric_value;

  msg->metrics[0] = mm;

  if (cerebrod_send_message(msg) < 0)
    {
      CEREBRO_DBG(("cerebrod_send_message"));
      _send_metric_control_response(fd,
                                    version,
                                    CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_INTERNAL_ERROR);
      goto cleanup;
    }

  rv = 0;
 cleanup:
  if (msg)
    {
      if (msg->metrics)
        free(msg->metrics);
      free(msg);
    }
  if (mm)
    free(mm);
  return rv;
}

/* 
 * _update_metric
 *
 * Update the metric type, len, and data for a metric
 *
 * Returns 0 on success, -1 on error
 */
static int
_update_metric(int fd, 
               int32_t version, 
               const char *metric_name,
               struct cerebro_metric_control_request *req)
{
  struct cerebrod_speaker_metric_info *metric_info;
  u_int32_t mtype, mlen;

  assert(fd >= 0 && metric_name && req && conf.speak);

  req->metric_value = NULL;

  if (req->metric_value_type == CEREBRO_DATA_VALUE_TYPE_STRING &&
      !req->metric_value_len)
    {
      CEREBRO_DBG(("adjusting metric type to none"));
      req->metric_value_type = CEREBRO_DATA_VALUE_TYPE_NONE;
    }

  mtype = req->metric_value_type;
  mlen = req->metric_value_len;
  if (check_data_type_len(mtype, mlen) < 0)
    {
      _send_metric_control_response(fd,
                                    version,
                                    CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_PARAMETER_INVALID);
      goto cleanup;
    }

  if (req->metric_value_len)
    {
      if (_receive_metric_value(fd, version, req) < 0)
        goto cleanup;
    }
 
  Pthread_mutex_lock(&metric_list_lock);
  
  if (!(metric_info = _find_speaker_metric_info(metric_name)))
    {
      _send_metric_control_response(fd,
                                    version,
                                    CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_METRIC_INVALID);
      Pthread_mutex_unlock(&metric_list_lock);
      goto cleanup;
    }
  
  if (!(metric_info->metric_origin & CEREBROD_METRIC_SPEAKER_ORIGIN_USERSPACE))
    {
      _send_metric_control_response(fd,
                                    version,
                                    CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_METRIC_INVALID);
      Pthread_mutex_unlock(&metric_list_lock);
      goto cleanup;
    } 

  if (metric_info->metric_value)
    Free(metric_info->metric_value);

  /* If we have to send it now, don't bother waiting for a message
   * to come along later by setting next_call_time to UINT_MAX.
   */
  if (req->flags & CEREBRO_METRIC_CONTROL_FLAGS_SEND_NOW)
    metric_info->next_call_time = UINT_MAX;
  else
    metric_info->next_call_time = 0;
  metric_info->metric_value_type = req->metric_value_type;
  metric_info->metric_value_len = req->metric_value_len;
  metric_info->metric_value = Malloc(req->metric_value_len);
  memcpy(metric_info->metric_value,
         req->metric_value,
         req->metric_value_len);
  cerebrod_speaker_data_metric_list_sort();
  Pthread_mutex_unlock(&metric_list_lock);

  if (req->flags & CEREBRO_METRIC_CONTROL_FLAGS_SEND_NOW)
    {
      if (_send_message_now(fd, 
                            version,
                            metric_name,
                            req->metric_value_type,
                            req->metric_value_len,
                            req->metric_value) < 0)
        goto cleanup;
    }

  return 0;

 cleanup:
  Free(req->metric_value);
  return -1;
}

/* 
 * _resend_metric
 *
 * Resend a metric by setting the next call time to 0
 *
 * Returns 0 on success, -1 on error
 */
static int
_resend_metric(int fd, 
               int32_t version, 
               const char *metric_name,
               struct cerebro_metric_control_request *req)
{
  struct cerebrod_speaker_metric_info *metric_info;
  u_int32_t metric_value_type = 0, metric_value_len = 0;
  void *metric_value_copy = NULL;
    
  int rv = -1;

  assert(fd >= 0 && metric_name && req && conf.speak);

  Pthread_mutex_lock(&metric_list_lock);
  
  if (!(metric_info = _find_speaker_metric_info(metric_name)))
    {
      _send_metric_control_response(fd,
                                    version,
                                    CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_METRIC_INVALID);
      Pthread_mutex_unlock(&metric_list_lock);
      goto cleanup;
    }
  
  if (!(metric_info->metric_origin & CEREBROD_METRIC_SPEAKER_ORIGIN_MODULE
        || metric_info->metric_origin & CEREBROD_METRIC_SPEAKER_ORIGIN_USERSPACE))
    {
      _send_metric_control_response(fd,
                                    version,
                                    CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_METRIC_INVALID);
      Pthread_mutex_unlock(&metric_list_lock);
      goto cleanup;
    }
  
  /* If we have to send it now, don't bother waiting for a message
   * to come along later by setting next_call_time to UINT_MAX.
   */
  if (req->flags & CEREBRO_METRIC_CONTROL_FLAGS_SEND_NOW)
    {
      metric_info->next_call_time = UINT_MAX;

      /* Copy data so we don't have to send data w/n a mutex section */
      metric_value_type = metric_info->metric_value_type;
      metric_value_len = metric_info->metric_value_len;
      metric_value_copy = (void *)Malloc(metric_info->metric_value_len);
      memcpy(metric_value_copy, 
             metric_info->metric_value,
             metric_info->metric_value_len);
    }
  else
    metric_info->next_call_time = 0;

  cerebrod_speaker_data_metric_list_sort();
  Pthread_mutex_unlock(&metric_list_lock);

  if (req->flags & CEREBRO_METRIC_CONTROL_FLAGS_SEND_NOW)
    {
      if (_send_message_now(fd, 
                            version,
                            metric_name,
                            metric_value_type,
                            metric_value_len,
                            metric_value_copy) < 0)
        goto cleanup;
    }

  rv = 0;
 cleanup:
  if (metric_value_copy)
    Free(metric_value_copy);
  return rv;
}

/* 
 * _flush_metric_data
 *
 * Flush metric data from the node
 *
 * Returns 0 on success, -1 on error
 */
static int
_flush_metric_data(void *x, const void *key, void *arg)
{
  struct cerebrod_node_data *nd;
  char *metric_name;
#if CEREBRO_DEBUG
  int rv;
#endif /* CEREBRO_DEBUG */

  assert(x && arg);
  
  nd = (struct cerebrod_node_data *)x;
  metric_name = (char *)arg;

#if CEREBRO_DEBUG
  /* Should be called with lock already set */
  rv = Pthread_mutex_trylock(&listener_data_lock);
  if (rv != EBUSY)
    CEREBRO_EXIT(("mutex not locked: rv=%d", rv));
#endif /* CEREBRO_DEBUG */
  
  Pthread_mutex_lock(&(nd->node_data_lock));
  if (Hash_find(nd->metric_data, metric_name))
    {
      struct cerebrod_metric_data *md;

      if ((md = Hash_remove(nd->metric_data, metric_name)))
        {
          metric_data_destroy(md);
          nd->metric_data_count--;
        }
      else
        CEREBRO_DBG(("illogical delete"));
    }
  Pthread_mutex_unlock(&(nd->node_data_lock));

  return 0;
}

/* 
 * _flush_metric
 *
 * Flush a metric 
 *
 * Returns 0 on success, -1 on error
 */
static int
_flush_metric(int fd, int32_t version, const char *metric_name)
{
  struct cerebrod_metric_name_data *mnd;
  
  /* 
   * Algorithm note, flushing means flushing the *current* known
   * contents.  It does not stop any update attempts currently in
   * progress.
   *
   * Note that there is some racing here with the metric server, but
   * it doesn't matter.  The metric server simply responds with
   * whatever it has at the moment.
   */

  Pthread_mutex_lock(&metric_names_lock);
  if (!(mnd = Hash_find(metric_names, metric_name)))
    {
      Pthread_mutex_unlock(&metric_names_lock);
      _send_metric_control_response(fd,
                                    version,
                                    CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_METRIC_INVALID);
      goto cleanup;
    }

  if (!(mnd->metric_origin & CEREBROD_METRIC_LISTENER_ORIGIN_MONITORED))
    {
      Pthread_mutex_unlock(&metric_names_lock);
      _send_metric_control_response(fd,
                                    version,
                                    CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_METRIC_INVALID);
      goto cleanup;
    }

  if ((mnd = Hash_remove(metric_names, metric_name)))
    metric_name_data_destroy(mnd);
  else
    CEREBRO_DBG(("illogical delete"));

  Pthread_mutex_unlock(&metric_names_lock);

  Pthread_mutex_lock(&listener_data_lock);
  if (hash_for_each(listener_data, _flush_metric_data, (void *)metric_name) < 0)
    {
      Pthread_mutex_unlock(&listener_data_lock);
      goto cleanup;
    }
  Pthread_mutex_unlock(&listener_data_lock);

  return 0;

 cleanup:
  return -1;
}

/* 
 * _speaker_metric_names_dump
 *
 * Dump the currently known/registered metric names
 */
static void
_speaker_metric_names_dump(void)
{
  struct cerebrod_speaker_metric_info *metric_info = NULL;
  ListIterator itr = NULL;

  if (!(conf.debug && conf.metric_controller_debug))
    return;

  Pthread_mutex_lock(&metric_list_lock);
  Pthread_mutex_lock(&debug_output_mutex);

  fprintf(stderr, "**************************************\n");
  fprintf(stderr, "* Speaker Data Metric Names\n");
  fprintf(stderr, "* -----------------------\n");
  itr = List_iterator_create(metric_list);
  while ((metric_info = list_next(itr)))
    fprintf(stderr, "* %s\n", metric_info->metric_name);
  fprintf(stderr, "**************************************\n");
  List_iterator_destroy(itr);
  Pthread_mutex_unlock(&debug_output_mutex);
  Pthread_mutex_unlock(&metric_list_lock);
}

/*
 * _metric_controller_service_connection
 *
 * Thread to service a connection from a client to retrieve metric
 * data.  Use wrapper functions minimally, b/c we want to return
 * errors to the user instead of exitting with errors.
 *
 * Passed int * pointer to client TCP socket file descriptor
 *
 * Executed in detached state, no return value.
 */
static void *
_metric_controller_service_connection(void *arg)
{
  int fd, recv_len;
  struct cerebro_metric_control_request req;
  char metric_name_buf[CEREBRO_MAX_METRIC_NAME_LEN+1];
  char buf[CEREBRO_MAX_PACKET_LEN];
  int32_t version;

  fd = *((int *)arg);
  
  memset(&req, '\0', sizeof(struct cerebro_metric_control_request));
  
  if ((recv_len = receive_data(fd,
                               CEREBRO_METRIC_CONTROL_REQUEST_HEADER_LEN,
                               buf,
                               CEREBRO_MAX_PACKET_LEN,
                               CEREBRO_METRIC_CONTROL_PROTOCOL_CLIENT_TIMEOUT_LEN,
                               NULL)) < 0)
    goto cleanup;
  
  if (recv_len < sizeof(version))
    goto cleanup;

  if (_metric_control_request_check_version(buf, recv_len, &version) < 0)
    {
      _send_metric_control_response(fd,
                                    version,
                                    CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_VERSION_INVALID);
      goto cleanup;
    }
  
  if (recv_len < CEREBRO_METRIC_CONTROL_REQUEST_HEADER_LEN)
    {
      _send_metric_control_response(fd,
                                    version,
                                    CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_PACKET_INVALID);
      goto cleanup;
    }
  
  if (_metric_control_request_header_unmarshall(&req, buf, recv_len) < 0)
    {
      _send_metric_control_response(fd,
                                    version,
                                    CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_PACKET_INVALID);
      goto cleanup;
    }

  _metric_control_request_dump(&req);

  /* Guarantee ending '\0' character */
  memset(metric_name_buf, '\0', CEREBRO_MAX_METRIC_NAME_LEN+1);
  memcpy(metric_name_buf, req.metric_name, CEREBRO_MAX_METRIC_NAME_LEN);
  
  if (!strlen(metric_name_buf))
    {
      _send_metric_control_response(fd,
                                    version,
                                    CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_METRIC_INVALID);
      goto cleanup;
    }

  if (req.command == CEREBRO_METRIC_CONTROL_PROTOCOL_CMD_REGISTER 
      && conf.speak)
    {
      if (_register_metric(fd, version, metric_name_buf) < 0)
        goto cleanup;
      _speaker_metric_names_dump();
    }
  else if (req.command == CEREBRO_METRIC_CONTROL_PROTOCOL_CMD_UNREGISTER 
           && conf.speak)
    {
      if (_unregister_metric(fd, version, metric_name_buf) < 0)
        goto cleanup;
      _speaker_metric_names_dump();
    }
  else if (req.command == CEREBRO_METRIC_CONTROL_PROTOCOL_CMD_UPDATE 
           && conf.speak)
    {
      if (_update_metric(fd, version, metric_name_buf, &req) < 0)
        goto cleanup;
      _speaker_metric_names_dump();
    }
  else if (req.command == CEREBRO_METRIC_CONTROL_PROTOCOL_CMD_RESEND 
           && conf.speak)
    {
      if (_resend_metric(fd, version, metric_name_buf, &req) < 0)
        goto cleanup;
      _speaker_metric_names_dump();
    }
  else if (req.command == CEREBRO_METRIC_CONTROL_PROTOCOL_CMD_FLUSH
           && conf.listen)
    {
      if (_flush_metric(fd, version, metric_name_buf) < 0)
        goto cleanup;
    }
  else
    {
      _send_metric_control_response(fd,
                                    version,
                                    CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_COMMAND_INVALID);
      goto cleanup;
    }


  _send_metric_control_response(fd,
                                version,
                                CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_SUCCESS);

 cleanup:
  if (req.metric_value)
    Free(req.metric_value);
  Free(arg);
  /* ignore potential error, we're done sendin */
  close(fd);
  return NULL;
}

void *
cerebrod_metric_controller(void *arg)
{
  int controller_fd;

  _metric_controller_initialize();

  if ((controller_fd = _metric_controller_setup_socket(0)) < 0)
    CEREBRO_EXIT(("metric controller fd setup failed"));

  for (;;)
    {
      pthread_t thread;
      pthread_attr_t attr;
      unsigned int client_addr_len;
      int fd, *arg;
      struct sockaddr_un client_addr;
      
      client_addr_len = sizeof(struct sockaddr_un);
      if ((fd = accept(controller_fd,
                       (struct sockaddr *)&client_addr,
                       &client_addr_len)) < 0)
        controller_fd = cerebrod_reinit_socket(controller_fd,
                                               0,
                                               _metric_controller_setup_socket,
                                               "metric_controller: accept");
      
      if (fd < 0)
        continue;

      /* Pass off connection to thread */
      Pthread_attr_init(&attr);
      Pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      Pthread_attr_setstacksize(&attr, CEREBROD_THREAD_STACKSIZE);
      arg = Malloc(sizeof(int));
      *arg = fd;
      Pthread_create(&thread,
                     &attr,
                     _metric_controller_service_connection,
                     (void *)arg);
      Pthread_attr_destroy(&attr);
    }

  return NULL;			/* NOT REACHED */
}

#endif /* !WITH_CEREBROD_SPEAKER_ONLY */
