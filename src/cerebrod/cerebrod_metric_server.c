/*****************************************************************************\
 *  $Id: cerebrod_metric_server.c,v 1.37 2006-10-31 04:32:22 chu11 Exp $
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

#include "cerebro/cerebro_metric_server_protocol.h"

#include "cerebrod.h"
#include "cerebrod_config.h"
#include "cerebrod_listener_data.h"
#include "cerebrod_metric_server.h"
#include "cerebrod_util.h"

#include "debug.h"
#include "fd.h"
#include "list.h"
#include "metric_util.h"
#include "network_util.h"
#include "wrappers.h"

extern struct cerebrod_config conf;
#if CEREBRO_DEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* CEREBRO_DEBUG */

extern hash_t listener_data;
extern hash_t metric_names;
extern pthread_mutex_t listener_data_lock;
extern pthread_mutex_t metric_names_lock;

#define CEREBROD_METRIC_SERVER_BACKLOG 10

/*
 * metric_server_init
 * metric_server_init_cond
 * metric_server_init_lock
 *
 * variables for synchronizing initialization between different pthreads
 * and signaling when it is complete
 */
int metric_server_init = 0;
pthread_cond_t metric_server_init_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t metric_server_init_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * _metric_server_initialize
 *
 * perform metric server initialization
 */
static void
_metric_server_initialize(void)
{
  Pthread_mutex_lock(&metric_server_init_lock);
  if (metric_server_init)
    goto out;

  Signal(SIGPIPE, SIG_IGN);

  metric_server_init++;
  Pthread_cond_signal(&metric_server_init_cond);
 out:
  Pthread_mutex_unlock(&metric_server_init_lock);
}

/*
 * _metric_server_request_unmarshall
 *
 * unmarshall contents of a metric server request packet buffer
 *
 * Returns 0 on success, -1 on error
 */
static int
_metric_server_request_unmarshall(struct cerebro_metric_server_request *req,
                                  const char *buf, 
                                  unsigned int buflen)
{
  int bufPtrlen, c = 0;
  char *bufPtr;
  
  assert(req && buf);

  bufPtr = req->metric_name;
  bufPtrlen = sizeof(req->metric_name);
  c += Unmarshall_int32(&(req->version), buf + c, buflen - c);
  c += Unmarshall_buffer(bufPtr, bufPtrlen, buf + c, buflen - c);
  c += Unmarshall_u_int32(&(req->timeout_len), buf + c, buflen - c);
  c += Unmarshall_u_int32(&(req->flags), buf + c, buflen - c);

  if (c != CEREBRO_METRIC_SERVER_REQUEST_PACKET_LEN)
    return -1;

  return 0;
}
     
/*  
 * _metric_server_request_dump
 *
 * dump contents of a metric server request
 */
static void
_metric_server_request_dump(struct cerebro_metric_server_request *req)
{
#if CEREBRO_DEBUG
  char metric_name_buf[CEREBRO_MAX_METRIC_NAME_LEN+1];

  assert(req);

  if (!(conf.debug && conf.metric_server_debug))
    return;
  
  Pthread_mutex_lock(&debug_output_mutex);
  fprintf(stderr, "**************************************\n");
  fprintf(stderr, "* Metric Server Request Received:\n");
  fprintf(stderr, "* ------------------------\n");
  fprintf(stderr, "* Version: %d\n", req->version);
  /* Guarantee ending '\0' character */
  memset(metric_name_buf, '\0', CEREBRO_MAX_METRIC_NAME_LEN+1);
  memcpy(metric_name_buf, req->metric_name, CEREBRO_MAX_METRIC_NAME_LEN);
  fprintf(stderr, "* Metric_name: %s\n", metric_name_buf);
  fprintf(stderr, "* Flags: %x\n", req->flags);
  fprintf(stderr, "* Timeout_len: %d\n", req->timeout_len);
  fprintf(stderr, "**************************************\n");
  Pthread_mutex_unlock(&debug_output_mutex);
#endif /* CEREBRO_DEBUG */
}

/* 
 * _metric_server_request_check_version
 *
 * Check that the version is correct prior to unmarshalling
 *
 * Returns 0 if version is correct, -1 if not
 */
static int
_metric_server_request_check_version(const char *buf, 
                                     unsigned int buflen, 
                                     int32_t *version)
{
  assert(buflen >= sizeof(int32_t) && version);
                                       
  if (!Unmarshall_int32(version, buf, buflen))
    {
      CEREBRO_DBG(("version could not be unmarshalled"));
      return -1;
    }

  if (*version != CEREBRO_METRIC_SERVER_PROTOCOL_VERSION)
    return -1;

  return 0;
}

/*
 * _metric_server_response_marshall
 *
 * marshall contents of a metric server response packet
 *
 * Returns length written to buffer on success, -1 on error
 */
static int
_metric_server_response_marshall(struct cerebro_metric_server_response *res,
                                 char *buf, 
                                 unsigned int buflen)
{
  int bufPtrlen, n, c = 0;
  char *bufPtr;

  assert(res && buf && buflen >= CEREBRO_METRIC_SERVER_RESPONSE_HEADER_LEN);

  memset(buf, '\0', buflen);

  bufPtr = res->name;
  bufPtrlen = sizeof(res->name);
  c += Marshall_int32(res->version, buf + c, buflen - c);
  c += Marshall_u_int32(res->err_code, buf + c, buflen - c);
  c += Marshall_u_int8(res->end, buf + c, buflen - c);
  c += Marshall_buffer(bufPtr, bufPtrlen, buf + c, buflen - c);
  c += Marshall_u_int32(res->metric_value_received_time, buf + c, buflen - c);

  if ((n = marshall_metric(res->metric_value_type, 
                           res->metric_value_len,
                           res->metric_value,
                           buf + c,
                           buflen - c,
                           NULL)) < 0)
    goto cleanup;
  c += n;

  return c;

 cleanup:
  return -1;
}

/*
 * _metric_server_err_response_marshall
 *
 * marshall contents of a metric err response packet buffer
 *
 * Returns length written to buffer on success, -1 on error
 */
static int
_metric_server_err_response_marshall(struct cerebro_metric_server_err_response *err_res,
                                     char *buf, 
                                     unsigned int buflen)
{
  int len = 0;
 
  assert(err_res && buf && buflen >= CEREBRO_METRIC_SERVER_ERR_RESPONSE_LEN);

  memset(buf, '\0', buflen);
  len += Marshall_int32(err_res->version, buf + len, buflen - len);
  len += Marshall_u_int32(err_res->err_code, buf + len, buflen - len);
  return len;
}

/*
 * _metric_server_response_send
 *
 * send a metric server response packet to the client
 *
 * Return 0 on success, -1 on error
 */
static int
_metric_server_response_send(int fd, struct cerebro_metric_server_response *res)
{
  char *buf = NULL;
  int buflen, res_len, rv = -1;

  assert(fd >= 0 && res);

  buflen = CEREBRO_METRIC_SERVER_RESPONSE_HEADER_LEN + res->metric_value_len + 1;
  buf = Malloc(buflen);

  if ((res_len = _metric_server_response_marshall(res, buf, buflen)) < 0)
    goto cleanup;

  if (fd_write_n(fd, buf, res_len) < 0)
    {
      CEREBRO_DBG(("fd_write_n: %s", strerror(errno)));
      goto cleanup;
    }

  rv = 0;
 cleanup:
  Free(buf);
  return rv;
}

/*
 * _metric_server_err_response_send
 *
 * send an error response packet to the client
 *
 * Return 0 on success, -1 on error
 */
static int
_metric_server_err_response_send(int fd, 
                                 struct cerebro_metric_server_err_response *res)
{
  char buf[CEREBRO_MAX_PACKET_LEN];
  int res_len, buflen;

  assert(fd >= 0 && res);

  buflen = CEREBRO_MAX_PACKET_LEN;
  if ((res_len = _metric_server_err_response_marshall(res, buf, buflen)) < 0)
    return -1;
  
  if (fd_write_n(fd, buf, res_len) < 0)
    {
      CEREBRO_DBG(("fd_write_n: %s", strerror(errno)));
      return -1;
    }

  return 0;
}

/* 
 * _metric_server_respond_with_error
 *
 * respond to the metric_server_request with an error
 *
 * Return 0 on success, -1 on error
 */
static int
_metric_server_respond_with_error(int fd, int32_t version, u_int32_t err_code)
{
  struct cerebro_metric_server_err_response res;

  assert(fd >= 0
         && err_code >= CEREBRO_METRIC_SERVER_PROTOCOL_ERR_VERSION_INVALID
	 && err_code <= CEREBRO_METRIC_SERVER_PROTOCOL_ERR_INTERNAL_ERROR);
  
  memset(&res, '\0', CEREBRO_METRIC_SERVER_ERR_RESPONSE_LEN);
  res.version = version;
  res.err_code = err_code;

  if (_metric_server_err_response_send(fd, &res) < 0)
    return -1;

  return 0;
}

/* 
 * _metric_server_response_create
 *
 * Create a metric server response and add it to the list of responses
 * to reply with.
 *
 * Returns 0 on success, -1 on error
 */
static int
_metric_server_response_create(char *name,
                               unsigned int max_name_len,
                               u_int32_t metric_value_recevied_time,
                               u_int32_t metric_value_type,
                               u_int32_t metric_value_len,
                               void *metric_value,
                               List responses)
{
  struct cerebro_metric_server_response *res = NULL;

  assert(name && max_name_len && responses);
  
  if ((metric_value_type == CEREBRO_METRIC_VALUE_TYPE_NONE 
       && metric_value_len)
      || (metric_value_type != CEREBRO_METRIC_VALUE_TYPE_NONE 
          && !metric_value_len))
    {
      CEREBRO_DBG(("bogus metric: type=%d len=%d", 
                   metric_value_type, metric_value_len));
      return -1;
    }

  if ((metric_value_len && !metric_value) || (!metric_value_len && metric_value))
    {
      CEREBRO_DBG(("bogus metric: len=%d value=%p", 
                   metric_value_len, metric_value));
      return -1;
    }

  if (!(res = malloc(sizeof(struct cerebro_metric_server_response))))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      return -1;
    }
  memset(res, '\0', sizeof(struct cerebro_metric_server_response));

  res->version = CEREBRO_METRIC_SERVER_PROTOCOL_VERSION;
  res->err_code = CEREBRO_METRIC_SERVER_PROTOCOL_ERR_SUCCESS;
  res->end = CEREBRO_METRIC_SERVER_PROTOCOL_IS_NOT_LAST_RESPONSE;
#if CEREBRO_DEBUG
  if (sizeof(res->name) < max_name_len)
    {
      CEREBRO_DBG(("bad name size: %d", sizeof(res->name)));
      goto cleanup;
    }
#endif /* CEREBRO_DEBUG */
  /* strncpy, b/c terminating character not required */
  strncpy(res->name, name, max_name_len);
  res->metric_value_received_time = metric_value_recevied_time;
  res->metric_value_type = metric_value_type;
  res->metric_value_len = metric_value_len;
  
  if (metric_value_len)
    {
      if (!(res->metric_value = (void *)malloc(metric_value_len)))
        {
          CEREBRO_DBG(("malloc: %s", strerror(errno)));
          goto cleanup;
        }
      memcpy(res->metric_value, metric_value, metric_value_len);
    }
  
  if (!list_append(responses, res))
    {
      CEREBRO_DBG(("list_append: %s", strerror(errno)));
      goto cleanup;
    }

  return 0;

 cleanup:
  if (res)
    {
      if (res->metric_value)
        free(res->metric_value);
      free(res);
    }
  return -1;
}

/*  
 * _metric_names_callback
 *
 * Callback function to create metric name responses
 *
 * Return 0 on success, -1 on error
 */
static int
_metric_names_callback(void *data, const void *key, void *arg)
{
  struct cerebrod_metric_name_evaluation_data *ed;
  struct cerebrod_metric_name_data *mnd;
#if CEREBRO_DEBUG
  int rv;
#endif /* CEREBRO_DEBUG */

  assert(data && arg);

  mnd = (struct cerebrod_metric_name_data *)data;
  ed = (struct cerebrod_metric_name_evaluation_data *)arg;
  
#if CEREBRO_DEBUG
  /* Should be called with lock already set */
  rv = Pthread_mutex_trylock(&metric_names_lock);
  if (rv != EBUSY)
    CEREBRO_EXIT(("mutex not locked: rv=%d", rv));
#endif /* CEREBRO_DEBUG */

  if (_metric_server_response_create(mnd->metric_name, 
                                     CEREBRO_MAX_METRIC_NAME_LEN,
                                     0,
                                     CEREBRO_METRIC_VALUE_TYPE_NONE,
                                     0,
                                     NULL,
                                     ed->responses) < 0)
    return -1;

  return 0;
}

/*  
 * _metric_data_evaluate
 *
 * Callback function for list_for_each, to determine if a node data
 * should be sent.
 *
 * Return 0 on success, -1 on error
 */
static int
_metric_data_evaluate(void *x, const void *key, void *arg)
{
  struct cerebrod_node_data *nd;
  struct cerebrod_metric_data_evaluation_data *ed;
  struct cerebrod_metric_data *md;
#if CEREBRO_DEBUG
  int rv;
#endif /* CEREBRO_DEBUG */

  assert(x && arg);

  nd = (struct cerebrod_node_data *)x;
  ed = (struct cerebrod_metric_data_evaluation_data *)arg;
  
#if CEREBRO_DEBUG
  /* Should be called with lock already set */
  rv = Pthread_mutex_trylock(&listener_data_lock);
  if (rv != EBUSY)
    CEREBRO_EXIT(("mutex not locked: rv=%d", rv));
#endif /* CEREBRO_DEBUG */

  Pthread_mutex_lock(&(nd->node_data_lock));

#if CEREBRO_DEBUG
  /* With locking, it shouldn't be possible for local time to be
   * greater than the time stored in any last_received time.
   */
  if (ed->time_now < nd->last_received_time)
    CEREBRO_DBG(("last_received time later than time_now time"));
#endif /* CEREBRO_DEBUG */

  if (!strcmp(ed->metric_name, CEREBRO_METRIC_CLUSTER_NODES))
    {
      if (_metric_server_response_create(nd->nodename,
                                         CEREBRO_MAX_NODENAME_LEN,
                                         0,
                                         CEREBRO_METRIC_VALUE_TYPE_NONE,
                                         0,
                                         NULL,
                                         ed->responses) < 0)
        {
          Pthread_mutex_unlock(&(nd->node_data_lock));
          return -1;
        }
    }
  else if (!strcmp(ed->metric_name, CEREBRO_METRIC_UPDOWN_STATE))
    {
      u_int32_t updown_state;

      if ((ed->time_now - nd->last_received_time) < ed->req->timeout_len)
        updown_state = CEREBRO_METRIC_UPDOWN_STATE_NODE_UP;
      else
        updown_state = CEREBRO_METRIC_UPDOWN_STATE_NODE_DOWN;

      if (_metric_server_response_create(nd->nodename,
                                         CEREBRO_MAX_NODENAME_LEN,
                                         nd->last_received_time,
                                         CEREBRO_METRIC_VALUE_TYPE_U_INT32,
                                         sizeof(u_int32_t),
                                         &updown_state,
                                         ed->responses) < 0)
        {
          Pthread_mutex_unlock(&(nd->node_data_lock));
          return -1;
        }
    }
  else 
    {
      if (ed->req->flags & CEREBRO_METRIC_FLAGS_UP_ONLY
          && !((ed->time_now - nd->last_received_time) < ed->req->timeout_len))
        goto out;

      if (ed->req->flags & CEREBRO_METRIC_FLAGS_NONE_IF_DOWN
          && !((ed->time_now - nd->last_received_time) < ed->req->timeout_len))
        {
          if (_metric_server_response_create(nd->nodename,
                                             CEREBRO_MAX_NODENAME_LEN,
                                             0,
                                             CEREBRO_METRIC_VALUE_TYPE_NONE,
                                             0,
                                             NULL,
                                             ed->responses) < 0)
            {
              Pthread_mutex_unlock(&(nd->node_data_lock));
              return -1;
            }
          goto out;
        }
      
      if ((md = Hash_find(nd->metric_data, ed->metric_name)))
        {
          if (_metric_server_response_create(nd->nodename,
                                             CEREBRO_MAX_NODENAME_LEN,
                                             md->metric_value_received_time,
                                             md->metric_value_type,
                                             md->metric_value_len,
                                             md->metric_value,
                                             ed->responses) < 0)
            {
              Pthread_mutex_unlock(&(nd->node_data_lock));
              return -1;
            }
        }
      else if (ed->req->flags & CEREBRO_METRIC_FLAGS_NONE_IF_NOT_MONITORED)
        {
          if (_metric_server_response_create(nd->nodename,
                                             CEREBRO_MAX_NODENAME_LEN,
                                             0,
                                             CEREBRO_METRIC_VALUE_TYPE_NONE,
                                             0,
                                             NULL,
                                             ed->responses) < 0)
            {
              Pthread_mutex_unlock(&(nd->node_data_lock));
              return -1;
            }
        }

    out:
      ;
    }

  Pthread_mutex_unlock(&(nd->node_data_lock));
  return 0;
}

/* 
 * _send_end_response
 * 
 * Send end metric server response
 *
 * Returns 0 on success, -1 on error
 */
static int
_send_end_response(int fd)
{
  struct cerebro_metric_server_response end_res;

  /* Send end response */
  memset(&end_res, '\0', sizeof(struct cerebro_metric_server_response));
  end_res.version = CEREBRO_METRIC_SERVER_PROTOCOL_VERSION;
  end_res.err_code = CEREBRO_METRIC_SERVER_PROTOCOL_ERR_SUCCESS;
  end_res.end = CEREBRO_METRIC_SERVER_PROTOCOL_IS_LAST_RESPONSE;

  if (_metric_server_response_send(fd, &end_res) < 0)
    return -1;

  return 0;
}

/* 
 * _metric_server_response_send_callback
 *
 * Callback function to send all responses
 */
static int
_metric_server_response_send_callback(void *x, void *arg)
{
  struct cerebro_metric_server_response *res;
  int fd;

  assert(x && arg);

  res = (struct cerebro_metric_server_response *)x;
  fd = *((int *)arg);

  if (_metric_server_response_send(fd, res) < 0)
    return -1;

  return 0;
}

/* 
 * _responses_send_all
 *
 * Send responses
 *
 * Returns 0 on success, -1 on error
 */
static int
_responses_send_all(int fd, List responses)
{
  assert(responses);

  if (List_count(responses))
    {
      if (list_for_each(responses, 
                        _metric_server_response_send_callback, 
                        &fd) < 0)
        return -1;
    }

  return 0;
}

/*  
 * _respond_with_metric_names
 *
 * Return 0 on success, -1 on error
 */
static int
_respond_with_metric_names(int fd, struct cerebro_metric_server_request *req)
{
  struct cerebrod_metric_name_evaluation_data ed;
  List responses = NULL;
  int rv = -1;

  assert(fd >= 0 && req);

  memset(&ed, '\0', sizeof(struct cerebrod_metric_name_evaluation_data));

  Pthread_mutex_lock(&metric_names_lock);
  
  if (!Hash_count(metric_names))
    {
      Pthread_mutex_unlock(&metric_names_lock);
      goto end_response;
    }

  if (!(responses = list_create((ListDelF)free)))
    {
      CEREBRO_DBG(("list_create: %s", strerror(errno)));
      Pthread_mutex_unlock(&metric_names_lock);
      _metric_server_respond_with_error(fd, 
                                        req->version, 
                                        CEREBRO_METRIC_SERVER_PROTOCOL_ERR_INTERNAL_ERROR);
      goto cleanup;
    }

  ed.fd = fd;
  ed.responses = responses;

  if (Hash_for_each(metric_names, _metric_names_callback, &ed) < 0)
    {
      Pthread_mutex_unlock(&metric_names_lock);
      _metric_server_respond_with_error(fd,
                                        req->version,
                                        CEREBRO_METRIC_SERVER_PROTOCOL_ERR_INTERNAL_ERROR);
      goto cleanup;
    }

  /* Transmission of the results can be done without this lock. */
  Pthread_mutex_unlock(&metric_names_lock);

  if (_responses_send_all(fd, responses) < 0)
    {
      _metric_server_respond_with_error(fd,
                                        req->version,
                                        CEREBRO_METRIC_SERVER_PROTOCOL_ERR_INTERNAL_ERROR);
      goto cleanup;
    }

 end_response:
  if (_send_end_response(fd) < 0)
    goto cleanup;

  rv = 0;
 cleanup:
  if (responses)
    list_destroy(responses);
  return rv;
}

/* 
 * _metric_data_response_destroy
 *
 * destroy a metric server data response
 */
static void
_metric_data_response_destroy(void *x)
{
  struct cerebro_metric_server_response *res;

  assert(x);
 
  res = (struct cerebro_metric_server_response *)x;
  free(res->metric_value);
  free(res);
}

/*  
 * _respond_with_nodes
 *
 * Return 0 on success, -1 on error
 */
static int
_respond_with_nodes(int fd, 
                    struct cerebro_metric_server_request *req, 
                    char *metric_name)
{
  struct cerebrod_metric_data_evaluation_data ed;
  struct timeval tv;
  List responses = NULL;
  int rv = -1;

  assert(fd >= 0 && req && metric_name);

  memset(&ed, '\0', sizeof(struct cerebrod_metric_data_evaluation_data));

  Pthread_mutex_lock(&listener_data_lock);
  
  if (!Hash_count(listener_data))
    {
      Pthread_mutex_unlock(&listener_data_lock);
      goto end_response;
    }

  if (!(responses = list_create((ListDelF)_metric_data_response_destroy)))
    {
      CEREBRO_DBG(("list_create: %s", strerror(errno)));
      Pthread_mutex_unlock(&listener_data_lock);
      _metric_server_respond_with_error(fd,
                                        req->version,
                                        CEREBRO_METRIC_SERVER_PROTOCOL_ERR_INTERNAL_ERROR);
      goto cleanup;
    }

  Gettimeofday(&tv, NULL);
  ed.fd = fd;
  ed.req = req;
  ed.time_now = tv.tv_sec;
  ed.metric_name = metric_name;
  ed.responses = responses;

  if (hash_for_each(listener_data, _metric_data_evaluate, &ed) < 0)
    {
      Pthread_mutex_unlock(&listener_data_lock);
      _metric_server_respond_with_error(fd,
                                        req->version,
                                        CEREBRO_METRIC_SERVER_PROTOCOL_ERR_INTERNAL_ERROR);
      goto cleanup;
    }

  /* Transmission of the results can be done without this lock. */
  Pthread_mutex_unlock(&listener_data_lock);

  if (_responses_send_all(fd, responses) < 0)
    {
      _metric_server_respond_with_error(fd,
                                        req->version,
                                        CEREBRO_METRIC_SERVER_PROTOCOL_ERR_INTERNAL_ERROR);
      goto cleanup;
    }

 end_response:
  if (_send_end_response(fd) < 0)
    goto cleanup;

  rv = 0;
 cleanup:
  if (responses)
    list_destroy(responses);
  return rv;
}

/* 
 * _metric_server_service_connection
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
_metric_server_service_connection(void *arg)
{
  int fd, recv_len;
  struct cerebro_metric_server_request req;
  char buf[CEREBRO_MAX_PACKET_LEN];
  char metric_name_buf[CEREBRO_MAX_METRIC_NAME_LEN+1];
  int32_t version;

  fd = *((int *)arg);

  memset(&req, '\0', sizeof(struct cerebro_metric_server_request));

  if ((recv_len = receive_data(fd, 
                               CEREBRO_METRIC_SERVER_REQUEST_PACKET_LEN,
                               buf,
                               CEREBRO_MAX_PACKET_LEN,
                               CEREBRO_METRIC_SERVER_PROTOCOL_CLIENT_TIMEOUT_LEN,
                               NULL)) < 0)
    goto cleanup;

  if (recv_len < sizeof(version))
    goto cleanup;

  if (_metric_server_request_check_version(buf, recv_len, &version) < 0)
    {
      _metric_server_respond_with_error(fd, 
                                        version,
                                        CEREBRO_METRIC_SERVER_PROTOCOL_ERR_VERSION_INVALID);
      goto cleanup;
    }

  if (recv_len != CEREBRO_METRIC_SERVER_REQUEST_PACKET_LEN)
    {
      _metric_server_respond_with_error(fd, 
                                        version,
                                        CEREBRO_METRIC_SERVER_PROTOCOL_ERR_PACKET_INVALID);
      goto cleanup;
    }

  if (_metric_server_request_unmarshall(&req, buf, recv_len) < 0)
    {
      _metric_server_respond_with_error(fd, 
                                        version,
                                        CEREBRO_METRIC_SERVER_PROTOCOL_ERR_PACKET_INVALID);
      goto cleanup;
    } 

  _metric_server_request_dump(&req);

  /* Guarantee ending '\0' character */
  memset(metric_name_buf, '\0', CEREBRO_MAX_METRIC_NAME_LEN+1);
  memcpy(metric_name_buf, req.metric_name, CEREBRO_MAX_METRIC_NAME_LEN);

  if (!strlen(metric_name_buf))
    {
      _metric_server_respond_with_error(fd,
                                        req.version,
                                        CEREBRO_METRIC_SERVER_PROTOCOL_ERR_METRIC_INVALID);
      goto cleanup;
    }

  Pthread_mutex_lock(&metric_names_lock);
  if (!Hash_find(metric_names, metric_name_buf))
    {
      Pthread_mutex_unlock(&metric_names_lock);
      _metric_server_respond_with_error(fd,
                                        req.version,
                                        CEREBRO_METRIC_SERVER_PROTOCOL_ERR_METRIC_INVALID);
      goto cleanup;
    }
  Pthread_mutex_unlock(&metric_names_lock);
  
  if (!req.timeout_len)
    req.timeout_len = CEREBRO_METRIC_SERVER_TIMEOUT_LEN_DEFAULT;
  
  if (!strcmp(metric_name_buf, CEREBRO_METRIC_METRIC_NAMES))
    {
      if (_respond_with_metric_names(fd, &req) < 0)
        goto cleanup;
    }
  else
    {
      if (_respond_with_nodes(fd, &req, metric_name_buf) < 0)
        goto cleanup;
    }

 cleanup:
  Free(arg);
  Close(fd);
  return NULL;
}

/*
 * _metric_server_setup_socket
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
_metric_server_setup_socket(int num)
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
  addr.sin_port = htons(conf.metric_server_port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
    {
      CEREBRO_DBG(("bind: %s", strerror(errno)));
      goto cleanup;
    }

  if (listen(fd, CEREBROD_METRIC_SERVER_BACKLOG) < 0)
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
cerebrod_metric_server(void *arg)
{
  int server_fd;

  _metric_server_initialize();

  if ((server_fd = _metric_server_setup_socket(0)) < 0)
    CEREBRO_EXIT(("metric server fd setup failed"));

  for (;;)
    {
      pthread_t thread;
      pthread_attr_t attr;
      int fd, client_addr_len, *arg;
      struct sockaddr_in client_addr;
      
      client_addr_len = sizeof(struct sockaddr_in);
      if ((fd = accept(server_fd,
                       (struct sockaddr *)&client_addr, 
                       &client_addr_len)) < 0)
        server_fd = cerebrod_reinit_socket(server_fd, 
                                           0,
                                           _metric_server_setup_socket, 
                                           "metric_server: accept");
      
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
                     _metric_server_service_connection, 
                     (void *)arg);
      Pthread_attr_destroy(&attr);
    }

  return NULL;			/* NOT REACHED */
}

#endif /* !WITH_CEREBROD_SPEAKER_ONLY */
