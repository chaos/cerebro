/*****************************************************************************\
 *  $Id: cerebrod_metric_server.c,v 1.3 2005-06-30 22:43:59 achu Exp $
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

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_metric_protocol.h"

#include "cerebrod_config.h"
#include "cerebrod_listener_data.h"
#include "cerebrod_metric_server.h"
#include "cerebrod_util.h"

#include "debug.h"
#include "fd.h"
#include "list.h"
#include "network_util.h"
#include "wrappers.h"

extern struct cerebrod_config conf;
#if CEREBRO_DEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* CEREBRO_DEBUG */

extern List listener_data_list;
extern hash_t metric_name_index;
extern pthread_mutex_t listener_data_lock;
extern pthread_mutex_t metric_name_lock;

#define CEREBROD_METRIC_BACKLOG           10

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
 * _cerebrod_metric_server_initialize
 *
 * perform metric server initialization
 */
static void
_cerebrod_metric_server_initialize(void)
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
 * _metric_name_response_marshall
 *
 * marshall contents of a metric response packet buffer
 *
 * Returns length written to buffer on success, -1 on error
 */
static int
_metric_name_response_marshall(struct cerebro_metric_name_response *res,
                               char *buf, 
                               unsigned int buflen)
{
  int len = 0;

  assert(res && buf && buflen >= CEREBRO_METRIC_NAME_RESPONSE_LEN);

  memset(buf, '\0', buflen);
  len += Marshall_int32(res->version, buf + len, buflen - len);
  len += Marshall_u_int32(res->err_code, buf + len, buflen - len);
  len += Marshall_u_int8(res->end, buf + len, buflen - len);
  len += Marshall_buffer(res->metric_name,
                         sizeof(res->metric_name),
                         buf + len,
                         buflen - len);
  return len;
}

/*
 * _node_metric_response_marshall
 *
 * marshall contents of a metric response packet buffer
 *
 * Returns length written to buffer on success, -1 on error
 */
static int
_node_metric_response_marshall(struct cerebro_node_metric_response *res,
                               char *buf, 
                               unsigned int buflen)
{
  int c = 0;

  assert(res && buf && buflen >= CEREBRO_NODE_METRIC_RESPONSE_HEADER_LEN);

  memset(buf, '\0', buflen);

  c += Marshall_int32(res->version, buf + c, buflen - c);
  c += Marshall_u_int32(res->err_code, buf + c, buflen - c);
  c += Marshall_u_int8(res->end, buf + c, buflen - c);
  c += Marshall_buffer(res->nodename,
                       sizeof(res->nodename),
                       buf + c,
                       buflen - c);
  c += Marshall_u_int32(res->metric_value_type, buf + c, buflen - c);
  c += Marshall_u_int32(res->metric_value_len, buf + c, buflen - c);
  
  if (!res->metric_value_len)
    return c;

  switch(res->metric_value_type)
    {
    case CEREBRO_METRIC_VALUE_TYPE_NONE:
      CEREBRO_DBG(("metric value len > 0 for type NONE"));
      break;
    case CEREBRO_METRIC_VALUE_TYPE_INT32:
      c += Marshall_int32(*((int32_t *)res->metric_value),
                          buf + c,
                          buflen - c);
      break;
    case CEREBRO_METRIC_VALUE_TYPE_U_INT32:
      c += Marshall_u_int32(*((u_int32_t *)res->metric_value),
                            buf + c,
                            buflen - c);
      break;
    case CEREBRO_METRIC_VALUE_TYPE_FLOAT:
      c += Marshall_float(*((float *)res->metric_value), 
                          buf + c,
                          buflen - c);
      break;
    case CEREBRO_METRIC_VALUE_TYPE_DOUBLE:
      c += Marshall_double(*((double *)res->metric_value),
                           buf + c,
                           buflen - c);
      break;
    case CEREBRO_METRIC_VALUE_TYPE_STRING:
      c += Marshall_buffer(res->metric_value,
                           res->metric_value_len,
                           buf + c,
                           buflen - c);
      break;
    default:
      CEREBRO_DBG(("invalid type %d", res->metric_value_type));
      return -1;
    }

  return c;
}


/*
 * _metric_err_response_marshall
 *
 * marshall contents of a metric err response packet buffer
 *
 * Returns length written to buffer on success, -1 on error
 */
static int
_metric_err_response_marshall(struct cerebro_metric_err_response *err_res,
                              char *buf, 
                              unsigned int buflen)
{
  int len = 0;
 
  assert(err_res && buf && buflen >= CEREBRO_METRIC_ERR_RESPONSE_LEN);

  memset(buf, '\0', buflen);
  len += Marshall_int32(err_res->version, buf + len, buflen - len);
  len += Marshall_u_int32(err_res->err_code, buf + len, buflen - len);
  return len;
}

/* 
 * _metric_request_check_version
 *
 * Check that the version is correct prior to unmarshalling
 *
 * Returns 0 if version is correct, -1 if not
 */
static int
_metric_request_check_version(const char *buf, 
                              unsigned int buflen,
                              int32_t *version)
{
  assert(version);
                                       
  if (!Unmarshall_int32(version, buf, buflen))
    return -1;
                                                                                     
  if (*version != CEREBRO_METRIC_PROTOCOL_VERSION)
    return -1;
                                                                                     
  return 0;
}


/*
 * _metric_request_unmarshall
 *
 * unmarshall contents of a metric request packet buffer
 *
 * Returns length of data unmarshalled on success, -1 on error
 */
static int
_metric_request_unmarshall(struct cerebro_metric_request *req,
                           const char *buf, 
                           unsigned int buflen)
{
  int n, c = 0;

  assert(req && buf);
 
  if (!(n = Unmarshall_int32(&(req->version), buf + c, buflen - c)))
    return c;
  c += n;
  
  if (!(n = Unmarshall_buffer(req->metric_name,
                              sizeof(req->metric_name),
                              buf + c,
                              buflen - c)))
    return c;
  c += n;
  
  if (!(n = Unmarshall_u_int32(&(req->timeout_len), buf + c, buflen - c)))
    return c;
  c += n;
  
  if (!(n = Unmarshall_u_int32(&(req->flags), buf + c, buflen - c)))
    return c;
  c += n;
  
  return c;
}
     
/*  
 * _metric_request_dump
 *
 * dump contents of an metric request
 */
static void
_metric_request_dump(struct cerebro_metric_request *req)
{
#if CEREBRO_DEBUG
  assert(req);

  if (conf.debug && conf.metric_server_debug)
    {
      Pthread_mutex_lock(&debug_output_mutex);
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Metric Request Received:\n");
      fprintf(stderr, "* ------------------------\n");
      fprintf(stderr, "* Version: %d\n", req->version);
      fprintf(stderr, "* Metric_Name: %s\n", req->metric_name);
      fprintf(stderr, "* Flags: %x\n", req->flags);
      fprintf(stderr, "* Timeout_len: %d\n", req->timeout_len);
      fprintf(stderr, "**************************************\n");
      Pthread_mutex_unlock(&debug_output_mutex);
    }
#endif /* CEREBRO_DEBUG */
}

/*
 * _metric_err_response_send
 *
 * send an error response packet to the client
 *
 * Return 0 on success, -1 on error
 */
static int
_metric_err_response_send(int client_fd, 
                          struct cerebro_metric_err_response *err_res)
{
  char buf[CEREBRO_MAX_PACKET_LEN];
  int err_res_len;

  assert(client_fd >= 0 && err_res);

  if ((err_res_len = _metric_err_response_marshall(err_res, 
                                                   buf, 
                                                   CEREBRO_MAX_PACKET_LEN)) < 0)
    return -1;
  
  if (fd_write_n(client_fd, buf, err_res_len) < 0)
    {
      CEREBRO_DBG(("fd_write_n: %s", strerror(errno)));
      return -1;
    }

  return 0;
}


/*
 * _metric_name_response_send_one
 *
 * send a response packet to the client
 *
 * Return 0 on success, -1 on error
 */
static int
_metric_name_response_send_one(int client_fd, 
                               struct cerebro_metric_name_response *res)
{
  char buf[CEREBRO_MAX_PACKET_LEN];
  int res_len, rv = -1;

  assert(client_fd >= 0 && res);

  if ((res_len = _metric_name_response_marshall(res, 
                                                buf, 
                                                CEREBRO_MAX_PACKET_LEN)) < 0)
    goto cleanup;

  if (fd_write_n(client_fd, buf, res_len) < 0)
    {
      CEREBRO_DBG(("fd_write_n: %s", strerror(errno)));
      goto cleanup;
    }
  
  rv = 0;
 cleanup:
  return rv;
}

/*
 * _node_metric_response_send_one
 *
 * send a response packet to the client
 *
 * Return 0 on success, -1 on error
 */
static int
_node_metric_response_send_one(int client_fd, 
                               struct cerebro_node_metric_response *res)
{
  char *buf = NULL;
  int buflen, res_len, rv = -1;

  assert(client_fd >= 0 && res);

  buflen = CEREBRO_NODE_METRIC_RESPONSE_HEADER_LEN + res->metric_value_len + 1;
  buf = Malloc(buflen);

  if ((res_len = _node_metric_response_marshall(res, buf, buflen)) < 0)
    goto cleanup;

  if (fd_write_n(client_fd, buf, res_len) < 0)
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
 * _metric_respond_with_error
 *
 * respond to the metric_request with an error
 *
 * Return 0 on success, -1 on error
 */
static int
_metric_respond_with_error(int client_fd, 
                           int32_t version,
                           u_int32_t err_code)
{
  struct cerebro_metric_err_response err_res;

  assert(client_fd >= 0
         && err_code >= CEREBRO_METRIC_PROTOCOL_ERR_VERSION_INVALID
	 && err_code <= CEREBRO_METRIC_PROTOCOL_ERR_INTERNAL_ERROR);
  
  memset(&err_res, '\0', CEREBRO_METRIC_ERR_RESPONSE_LEN);
  
  err_res.version = version;
  err_res.err_code = err_code;

  if (_metric_err_response_send(client_fd, &err_res) < 0)
    return -1;

  return 0;
}

/*
 * _metric_name_responses_send_all
 *
 * respond to the metric_request with nodes
 *
 * Return 0 on success, -1 on error
 */
static int
_metric_name_responses_send_all(void *x, void *arg)
{
  struct cerebro_metric_name_response *res;
  int client_fd;

  assert(x && arg);

  res = (struct cerebro_metric_name_response *)x;
  client_fd = *((int *)arg);

  if (_metric_name_response_send_one(client_fd, res) < 0)
    return -1;

  return 0;
}

/*
 * _metric_name_response_create
 *
 * Create a metric name response and add it to the list of responses
 * to reply with.
 *
 * Returns 0 on success, -1 on error
 */
static int
_metric_name_response_create(char *metric_name, List metric_name_responses)
{
  struct cerebro_metric_name_response *res = NULL;

  assert(metric_name && metric_name_responses);
  
  if (!(res = malloc(sizeof(struct cerebro_metric_name_response))))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      return -1;
    }
  memset(res, '\0', sizeof(struct cerebro_metric_name_response));
  
  res->version = CEREBRO_METRIC_PROTOCOL_VERSION;
  res->err_code = CEREBRO_METRIC_PROTOCOL_ERR_SUCCESS;
  res->end = CEREBRO_METRIC_PROTOCOL_IS_NOT_LAST_RESPONSE;
#if CEREBRO_DEBUG
  if (metric_name && strlen(metric_name) > CEREBRO_MAX_METRIC_NAME_LEN)
    CEREBRO_DBG(("invalid node name length: %s", metric_name));
#endif /* CEREBRO_DEBUG */
      
  /* strncpy, b/c terminating character not required */
  strncpy(res->metric_name, metric_name, CEREBRO_MAX_METRIC_NAME_LEN);
 
  if (!list_append(metric_name_responses, res))
    {
      CEREBRO_DBG(("list_append: %s", strerror(errno)));
      goto cleanup;
    }

  return 0;

 cleanup:
  if (res)
    free(res);
  return -1;
}

/*  
 * _metric_name_index_callback
 *
 * Callback function for list_for_each, to determine if a node data
 * should be sent.
 *
 * Return 0 on success, -1 on error
 */
static int
_metric_name_index_callback(void *data, const void *key, void *arg)
{
  struct cerebrod_metric_name_evaluation_data *ed;
  char *metric_name;
#if CEREBRO_DEBUG
  int rv;
#endif /* CEREBRO_DEBUG */

  assert(data && arg);

  metric_name = (char *)data;
  ed = (struct cerebrod_metric_name_evaluation_data *)arg;
  
#if CEREBRO_DEBUG
  /* Should be called with lock already set */
  rv = Pthread_mutex_trylock(&metric_name_lock);
  if (rv != EBUSY)
    CEREBRO_EXIT(("mutex not locked: rv=%d", rv));
#endif /* CEREBRO_DEBUG */

  if (_metric_name_response_create(metric_name, ed->metric_name_responses) < 0)
    return -1;

  return 0;
}

/*  
 * _respond_with_metric_names
 *
 * Return 0 on success, -1 on error
 */
static int
_respond_with_metric_names(int client_fd, struct cerebro_metric_request *req)
{
  struct cerebrod_metric_name_evaluation_data ed;
  struct cerebro_metric_name_response end_res;
  List metric_name_responses = NULL;

  assert(client_fd >= 0 && req);

  memset(&ed, '\0', sizeof(struct cerebrod_metric_name_evaluation_data));
  Pthread_mutex_lock(&metric_name_lock);
  
  if (!Hash_count(metric_name_index))
    {
      Pthread_mutex_unlock(&metric_name_lock);
      goto end_response;
    }

  if (!(metric_name_responses = list_create((ListDelF)free)))
    {
      CEREBRO_DBG(("list_create: %s", strerror(errno)));
      Pthread_mutex_unlock(&metric_name_lock);
      _metric_respond_with_error(client_fd,
                                 req->version,
                                 CEREBRO_METRIC_PROTOCOL_ERR_INTERNAL_ERROR);
      goto cleanup;
    }

  ed.client_fd = client_fd;
  ed.metric_name_responses = metric_name_responses;

  if (Hash_for_each(metric_name_index,
		    _metric_name_index_callback, 
		    &ed) < 0)
    {
      Pthread_mutex_unlock(&metric_name_lock);
      _metric_respond_with_error(client_fd,
                                 req->version,
                                 CEREBRO_METRIC_PROTOCOL_ERR_INTERNAL_ERROR);
      goto cleanup;
    }

  /* Transmission of the results can be done without this lock. */
  Pthread_mutex_unlock(&metric_name_lock);

  if (List_count(metric_name_responses))
    {
      if (list_for_each(metric_name_responses,
                        _metric_name_responses_send_all, 
			&client_fd) < 0)
        {
          _metric_respond_with_error(client_fd,
                                     req->version,
                                     CEREBRO_METRIC_PROTOCOL_ERR_INTERNAL_ERROR);
          goto cleanup;
        }
    }

 end_response:

  /* Send end response */
  memset(&end_res, '\0', sizeof(struct cerebro_metric_name_response));
  end_res.version = CEREBRO_METRIC_PROTOCOL_VERSION;
  end_res.err_code = CEREBRO_METRIC_PROTOCOL_ERR_SUCCESS;
  end_res.end = CEREBRO_METRIC_PROTOCOL_IS_LAST_RESPONSE;

  if (_metric_name_response_send_one(client_fd, &end_res) < 0)
    return -1;

  list_destroy(metric_name_responses);
  return 0;

 cleanup:
  if (metric_name_responses)
    list_destroy(metric_name_responses);
  return -1;
}

/*
 * _node_metric_responses_send_all
 *
 * respond to the metric_request with nodes
 *
 * Return 0 on success, -1 on error
 */
static int
_node_metric_responses_send_all(void *x, void *arg)
{
  struct cerebro_node_metric_response *res;
  int client_fd;

  assert(x && arg);

  res = (struct cerebro_node_metric_response *)x;
  client_fd = *((int *)arg);

  if (_node_metric_response_send_one(client_fd, res) < 0)
    return -1;

  return 0;
}

/* 
 * _node_metric_response_create
 *
 * Create a node metric response and add it to the list of responses
 * to reply with.
 *
 * Returns 0 on success, -1 on error
 */
static int
_node_metric_response_create(char *nodename,
                             u_int32_t metric_value_type,
                             u_int32_t metric_value_len,
                             void *metric_value,
                             List node_responses)
{
  struct cerebro_node_metric_response *res = NULL;

  assert(nodename && node_responses);
  
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

  if (!(res = malloc(sizeof(struct cerebro_node_metric_response))))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      return -1;
    }
  memset(res, '\0', sizeof(struct cerebro_node_metric_response));

  res->version = CEREBRO_METRIC_PROTOCOL_VERSION;
  res->err_code = CEREBRO_METRIC_PROTOCOL_ERR_SUCCESS;
  res->end = CEREBRO_METRIC_PROTOCOL_IS_NOT_LAST_RESPONSE;
#if CEREBRO_DEBUG
  if (nodename && strlen(nodename) > CEREBRO_MAX_NODENAME_LEN)
    CEREBRO_DBG(("invalid node name length: %s", nodename));
#endif /* CEREBRO_DEBUG */
      
  /* strncpy, b/c terminating character not required */
  strncpy(res->nodename, nodename, CEREBRO_MAX_NODENAME_LEN);
      
  res->metric_value_type = metric_value_type;
  res->metric_value_len = metric_value_len;
  
  if (metric_value_len)
    {
      if (!(res->metric_value = (void *)malloc(metric_value_len)))
        {
          CEREBRO_DBG(("malloc: %s", strerror(errno)));
          return -1;
        }
      memcpy(res->metric_value, metric_value, metric_value_len);
    }
  
  if (!list_append(node_responses, res))
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
 * _node_metric_evaluate
 *
 * Callback function for list_for_each, to determine if a node data
 * should be sent.
 *
 * Return 0 on success, -1 on error
 */
static int
_node_metric_evaluate(void *x, void *arg)
{
  struct cerebrod_node_data *nd;
  struct cerebrod_node_metric_evaluation_data *ed;
  struct cerebrod_listener_metric_data *md;
#if CEREBRO_DEBUG
  int rv;
#endif /* CEREBRO_DEBUG */

  assert(x && arg);

  nd = (struct cerebrod_node_data *)x;
  ed = (struct cerebrod_node_metric_evaluation_data *)arg;
  
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
      if (_node_metric_response_create(nd->nodename,
                                       CEREBRO_METRIC_VALUE_TYPE_NONE,
                                       0,
                                       NULL,
                                       ed->node_responses) < 0)
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

      if (_node_metric_response_create(nd->nodename,
                                       CEREBRO_METRIC_VALUE_TYPE_U_INT32,
                                       sizeof(u_int32_t),
                                       &updown_state,
                                       ed->node_responses) < 0)
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
          if (_node_metric_response_create(nd->nodename,
                                           CEREBRO_METRIC_VALUE_TYPE_NONE,
                                           0,
                                           NULL,
                                           ed->node_responses) < 0)
            {
              Pthread_mutex_unlock(&(nd->node_data_lock));
              return -1;
            }
        }
      
      if ((md = Hash_find(nd->metric_data, ed->metric_name)))
        {
          if (_node_metric_response_create(nd->nodename,
                                           md->metric_value_type,
                                           md->metric_value_len,
                                           md->metric_value,
                                           ed->node_responses) < 0)
            {
              Pthread_mutex_unlock(&(nd->node_data_lock));
              return -1;
            }
        }
      else if (ed->req->flags & CEREBRO_METRIC_FLAGS_NONE_IF_NOEXIST)
        {
          if (_node_metric_response_create(nd->nodename,
                                           CEREBRO_METRIC_VALUE_TYPE_NONE,
                                           0,
                                           NULL,
                                           ed->node_responses) < 0)
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
 * _node_metric_response_destroy
 *
 * destroy a metric server data response
 */
static void
_node_metric_response_destroy(void *x)
{
  struct cerebro_node_metric_response *res;

  assert(x);
 
  res = (struct cerebro_node_metric_response *)x;
  free(res->metric_value);
  free(res);
}

/*  
 * _respond_with_nodes
 *
 * Return 0 on success, -1 on error
 */
static int
_respond_with_nodes(int client_fd, 
                    struct cerebro_metric_request *req,
                    char *metric_name)
{
  struct cerebrod_node_metric_evaluation_data ed;
  struct cerebro_node_metric_response end_res;
  struct timeval tv;
  List node_responses = NULL;

  assert(client_fd >= 0 && req && metric_name);

  memset(&ed, '\0', sizeof(struct cerebrod_node_metric_evaluation_data));

  Pthread_mutex_lock(&listener_data_lock);
  
  if (!List_count(listener_data_list))
    {
      Pthread_mutex_unlock(&listener_data_lock);
      goto end_response;
    }

  if (!(node_responses = list_create((ListDelF)_node_metric_response_destroy)))
    {
      CEREBRO_DBG(("list_create: %s", strerror(errno)));
      Pthread_mutex_unlock(&listener_data_lock);
      _metric_respond_with_error(client_fd,
                                 req->version,
                                 CEREBRO_METRIC_PROTOCOL_ERR_INTERNAL_ERROR);
      goto cleanup;
    }

  Gettimeofday(&tv, NULL);
  ed.client_fd = client_fd;
  ed.req = req;
  ed.time_now = tv.tv_sec;
  ed.metric_name = metric_name;
  ed.node_responses = node_responses;

  if (list_for_each(listener_data_list, _node_metric_evaluate, &ed) < 0)
    {
      Pthread_mutex_unlock(&listener_data_lock);
      _metric_respond_with_error(client_fd,
                                 req->version,
                                 CEREBRO_METRIC_PROTOCOL_ERR_INTERNAL_ERROR);
      goto cleanup;
    }

  /* Transmission of the results can be done without this lock. */
  Pthread_mutex_unlock(&listener_data_lock);

  if (List_count(node_responses))
    {
      if (list_for_each(node_responses,
			_node_metric_responses_send_all, 
			&client_fd) < 0)
        {
          _metric_respond_with_error(client_fd,
                                     req->version,
                                     CEREBRO_METRIC_PROTOCOL_ERR_INTERNAL_ERROR);
          goto cleanup;
        }
    }

 end_response:
  /* Send end response */
  memset(&end_res, '\0', sizeof(struct cerebro_node_metric_response));
  end_res.version = CEREBRO_METRIC_PROTOCOL_VERSION;
  end_res.err_code = CEREBRO_METRIC_PROTOCOL_ERR_SUCCESS;
  end_res.end = CEREBRO_METRIC_PROTOCOL_IS_LAST_RESPONSE;

  if (_node_metric_response_send_one(client_fd, &end_res) < 0)
    return -1;

  list_destroy(node_responses);
  return 0;

 cleanup:
  if (node_responses)
    list_destroy(node_responses);
  return -1;
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
  int client_fd, req_len, recv_len;
  struct cerebro_metric_request req;
  char buf[CEREBRO_MAX_PACKET_LEN];
  char metric_name_buf[CEREBRO_MAX_METRIC_NAME_LEN+1];
  client_fd = *((int *)arg);
  int32_t version;

  memset(&req, '\0', sizeof(struct cerebro_metric_request));

  if ((recv_len = receive_data(client_fd, 
                               CEREBRO_METRIC_REQUEST_PACKET_LEN,
                               buf,
                               CEREBRO_MAX_PACKET_LEN,
                               CEREBRO_METRIC_PROTOCOL_CLIENT_TIMEOUT_LEN,
                               NULL)) < 0)
    goto cleanup;

  if (_metric_request_check_version(buf, recv_len, &version) < 0)
    {
      _metric_respond_with_error(client_fd, 
                                 version,
                                 CEREBRO_METRIC_PROTOCOL_ERR_VERSION_INVALID);
      goto cleanup;
    }

  if ((req_len = _metric_request_unmarshall(&req, buf, recv_len)) < 0)
    {
      _metric_respond_with_error(client_fd, 
                                 req.version,
                                 CEREBRO_METRIC_PROTOCOL_ERR_PACKET_INVALID);
      goto cleanup;
    }
  
  if (req_len != CEREBRO_METRIC_REQUEST_PACKET_LEN)
    {
      _metric_respond_with_error(client_fd, 
                                 req.version,
                                 CEREBRO_METRIC_PROTOCOL_ERR_PACKET_INVALID);
      goto cleanup;
    }

  _metric_request_dump(&req);

  /* Guarantee ending '\0' character */
  memset(metric_name_buf, '\0', CEREBRO_MAX_METRIC_NAME_LEN+1);
  memcpy(metric_name_buf, req.metric_name, CEREBRO_MAX_METRIC_NAME_LEN);

  Pthread_mutex_lock(&metric_name_lock);
  if (!Hash_find(metric_name_index, metric_name_buf))
    {
      Pthread_mutex_unlock(&metric_name_lock);
      _metric_respond_with_error(client_fd,
                                 req.version,
                                 CEREBRO_METRIC_PROTOCOL_ERR_METRIC_UNKNOWN);
      goto cleanup;
    }
  Pthread_mutex_unlock(&metric_name_lock);
  
  if (!req.timeout_len)
    req.timeout_len = CEREBRO_METRIC_TIMEOUT_LEN_DEFAULT;
  
  if (!strcmp(metric_name_buf, CEREBRO_METRIC_METRIC_NAMES))
    {
      if (_respond_with_metric_names(client_fd, &req) < 0)
        goto cleanup;
    }
  else
    {
      if (_respond_with_nodes(client_fd, &req, metric_name_buf) < 0)
        goto cleanup;
    }

 cleanup:
  Free(arg);
  Close(client_fd);
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
 */
static int
_metric_server_setup_socket(void)
{
  struct sockaddr_in addr;
  int fd, optval = 1;

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      CEREBRO_DBG(("socket: %s", strerror(errno)));
      return -1;
    }

  /* Configuration checks ensure destination ip is on this machine if
   * it is a non-multicast address.
   */
  memset(&addr, '\0', sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(conf.metric_server_port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
    {
      CEREBRO_DBG(("bind: %s", strerror(errno)));
      return -1;
    }

  if (listen(fd, CEREBROD_METRIC_BACKLOG) < 0)
    {
      CEREBRO_DBG(("listen: %s", strerror(errno)));
      return -1;
    }

  /* For quick start/restart */
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) < 0)
    {
      CEREBRO_DBG(("setsockopt: %s", strerror(errno)));
      return -1;
    }

  return fd;
}

void *
cerebrod_metric_server(void *arg)
{
  int fd;

  _cerebrod_metric_server_initialize();

  if ((fd = _metric_server_setup_socket()) < 0)
    CEREBRO_EXIT(("fd setup failed"));

  for (;;)
    {
      pthread_t thread;
      pthread_attr_t attr;
      int client_fd, client_addr_len, *arg;
      struct sockaddr_in client_addr;
      
      client_addr_len = sizeof(struct sockaddr_in);
      if ((client_fd = accept(fd,
                              (struct sockaddr *)&client_addr,
                              &client_addr_len)) < 0)
        fd = cerebrod_reinitialize_socket(fd,
                                          _metric_server_setup_socket,
                                          "metric_server: accept");
      
      if (client_fd < 0)
        continue;
      
      /* Pass off connection to thread */
      Pthread_attr_init(&attr);
      Pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      arg = Malloc(sizeof(int));
      *arg = client_fd;
      Pthread_create(&thread, 
                     &attr, 
                     _metric_server_service_connection, 
                     (void *)arg);
      Pthread_attr_destroy(&attr);
    }

  return NULL;			/* NOT REACHED */
}
