/*****************************************************************************\
 *  $Id: cerebrod_metric.c,v 1.8 2005-05-25 20:39:35 achu Exp $
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

#include "cerebro_marshalling.h"
#include "cerebro_module.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_error.h"
#include "cerebro/cerebro_metric_protocol.h"

#include "cerebrod.h"
#include "cerebrod_config.h"
#include "cerebrod_metric.h"
#include "cerebrod_node_data.h"
#include "cerebrod_util.h"
#include "fd.h"
#include "list.h"
#include "wrappers.h"

#define CEREBROD_METRIC_BACKLOG           10
#define CEREBROD_METRIC_REINITIALIZE_WAIT 2

extern struct cerebrod_config conf;
#if CEREBRO_DEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* CEREBRO_DEBUG */

/*
 * cerebrod_metric_initialization_complete
 * cerebrod_metric_initialization_complete_cond
 * cerebrod_metric_initialization_complete_lock
 *
 * variables for synchronizing initialization between different pthreads
 * and signaling when it is complete
 */
int cerebrod_metric_initialization_complete = 0;
pthread_cond_t cerebrod_metric_initialization_complete_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t cerebrod_metric_initialization_complete_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * _cerebrod_metric_initialize
 *
 * perform metric server initialization
 */
static void
_cerebrod_metric_initialize(void)
{
  Pthread_mutex_lock(&cerebrod_metric_initialization_complete_lock);
  if (cerebrod_metric_initialization_complete)
    goto out;

  cerebrod_node_data_initialize();

  cerebrod_metric_initialization_complete++;
  Pthread_cond_signal(&cerebrod_metric_initialization_complete_cond);
 out:
  Pthread_mutex_unlock(&cerebrod_metric_initialization_complete_lock);
}

/*
 * _cerebrod_metric_response_marshall
 *
 * marshall contents of a metric response packet buffer
 *
 * Returns length written to buffer on success, -1 on error
 */
static int
_cerebrod_metric_response_marshall(struct cerebro_metric_response *res,
				   char *buf, 
                                   unsigned int buflen)
{
  int len, count = 0;
  char val_buf[CEREBRO_METRIC_VALUE_LEN];

  assert(res);
  assert(buf);
  assert(buflen >= CEREBRO_METRIC_RESPONSE_LEN);

  memset(buf, '\0', buflen);

  if ((len = _cerebro_marshall_int32(res->version, 
                                     buf + count, 
                                     buflen - count)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_marshall_int32",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  count += len;
 
  if ((len = _cerebro_marshall_unsigned_int32(res->metric_err_code, 
                                              buf + count, 
                                              buflen - count)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_marshall_unsigned_int32",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  count += len;

  if ((len = _cerebro_marshall_unsigned_int8(res->end_of_responses, 
                                             buf + count, 
                                             buflen - count)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_marshall_unsigned_int8",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  count += len;

  if ((len = _cerebro_marshall_buffer(res->nodename,
                                      sizeof(res->nodename),
                                      buf + count,
                                      buflen - count)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_marshall_buffer",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  count += len;

  if ((len = _cerebro_marshall_unsigned_int32(res->metric_type, 
                                              buf + count, 
                                              buflen - count)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_marshall_unsigned_int32",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  count += len;

  memset(val_buf, '\0', CEREBRO_METRIC_VALUE_LEN);
  switch(res->metric_type)
    {
    case CEREBRO_METRIC_TYPE_BOOL:
      if ((len = _cerebro_marshall_int8(res->metric_value.val_bool, 
                                        val_buf, 
                                        CEREBRO_METRIC_VALUE_LEN)) < 0)
        {
          cerebro_err_debug("%s(%s:%d): _cerebro_marshall_int8",
                            __FILE__, __FUNCTION__, __LINE__);
          return -1;
        }
      break;
    case CEREBRO_METRIC_TYPE_INT32:
      if ((len = _cerebro_marshall_int32(res->metric_value.val_int32, 
                                        val_buf, 
                                        CEREBRO_METRIC_VALUE_LEN)) < 0)
        {
          cerebro_err_debug("%s(%s:%d): _cerebro_marshall_int32",
                            __FILE__, __FUNCTION__, __LINE__);
          return -1;
        }
      break;
    case CEREBRO_METRIC_TYPE_UNSIGNED_INT32:
      if ((len = _cerebro_marshall_unsigned_int32(res->metric_value.val_unsigned_int32, 
                                                  val_buf, 
                                                  CEREBRO_METRIC_VALUE_LEN)) < 0)
        {
          cerebro_err_debug("%s(%s:%d): _cerebro_marshall_unsigned_int32",
                            __FILE__, __FUNCTION__, __LINE__);
          return -1;
        }
      break;
    case CEREBRO_METRIC_TYPE_FLOAT:
      if ((len = _cerebro_marshall_float(res->metric_value.val_float, 
                                        val_buf, 
                                        CEREBRO_METRIC_VALUE_LEN)) < 0)
        {
          cerebro_err_debug("%s(%s:%d): _cerebro_marshall_float",
                            __FILE__, __FUNCTION__, __LINE__);
          return -1;
        }
      break;
    case CEREBRO_METRIC_TYPE_DOUBLE:
      if ((len = _cerebro_marshall_double(res->metric_value.val_double, 
                                          val_buf, 
                                          CEREBRO_METRIC_VALUE_LEN)) < 0)
        {
          cerebro_err_debug("%s(%s:%d): _cerebro_marshall_double",
                            __FILE__, __FUNCTION__, __LINE__);
          return -1;
        }
      break;
    case CEREBRO_METRIC_TYPE_STRING:
      if ((len = _cerebro_marshall_buffer(res->metric_value.val_string,
                                          sizeof(res->metric_value.val_string),
                                          val_buf,
                                          CEREBRO_METRIC_VALUE_LEN)) < 0)
        {
          cerebro_err_debug("%s(%s:%d): _cerebro_marshall_buffer",
                            __FILE__, __FUNCTION__, __LINE__);
          return -1;
        }
      break;
    default:
      cerebro_err_debug("%s(%s:%d): invalid type: %d",
                        __FILE__, __FUNCTION__, __LINE__,
                        res->metric_type);
      return -1;
    }

  if ((len = _cerebro_marshall_buffer(val_buf,
                                      sizeof(val_buf),
                                      buf + count,
                                      buflen - count)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_marshall_buffer",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  count += len;

  return count;
}

/*
 * _cerebrod_metric_err_response_marshall
 *
 * marshall contents of a metric err response packet buffer
 *
 * Returns length written to buffer on success, -1 on error
 */
static int
_cerebrod_metric_err_response_marshall(struct cerebro_metric_err_response *err_res,
                                       char *buf, 
                                       unsigned int buflen)
{
  int len, count = 0;
 
  assert(err_res);
  assert(buf);
  assert(buflen >= CEREBRO_METRIC_ERR_RESPONSE_LEN);

  memset(buf, '\0', buflen);

  if ((len = _cerebro_marshall_int32(err_res->version, 
                                     buf + count, 
                                     buflen - count)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_marshall_int32",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  count += len;
 
  if ((len = _cerebro_marshall_unsigned_int32(err_res->metric_err_code, 
                                              buf + count, 
                                              buflen - count)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_marshall_unsigned_int32",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  count += len;

  return count;
}

/*
 * _cerebrod_metric_request_unmarshall
 *
 * unmarshall contents of a metric request packet buffer
 *
 * Returns length of data unmarshalled on success, -1 on error
 */
static int
_cerebrod_metric_request_unmarshall(struct cerebro_metric_request *req,
				    const char *buf, 
                                    unsigned int buflen)
{
  int len, count = 0;

  assert(req);
  assert(buf);
 
  if ((len = _cerebro_unmarshall_int32(&(req->version), 
                                       buf + count, 
                                       buflen - count)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_unmarshall_int32",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!len)
    return count;

  count += len;

  if ((len = _cerebro_unmarshall_buffer(req->metric_name,
                                        sizeof(req->metric_name),
                                        buf + count,
                                        buflen - count)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_unmarshall_buffer",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!len)
    return count;

  count += len;

  if ((len = _cerebro_unmarshall_unsigned_int32(&(req->int32_param1), 
                                                buf + count, 
                                                buflen - count)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_unmarshall_unsigned_int32",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!len)
    return count;

  count += len;
 
  if ((len = _cerebro_unmarshall_unsigned_int32(&(req->unsigned_int32_param1), 
                                                buf + count, 
                                                buflen - count)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_unmarshall_unsigned_int32",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!len)
    return count;

  count += len;

  if ((len = _cerebro_unmarshall_buffer(req->string_param1,
                                        sizeof(req->string_param1),
                                        buf + count,
                                        buflen - count)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_unmarshall_buffer",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!len)
    return count;

  count += len;
   
  return count;
}

/*
 * _cerebrod_metric_request_receive
 *
 * Receive metric server request
 * 
 * Return request packet and length of packet unmarshalled on success,
 * -1 on error
 */
static int
_cerebrod_metric_request_receive(int client_fd,	
				 struct cerebro_metric_request *req)
{
  int rv, bytes_read = 0;
  char buf[CEREBRO_PACKET_BUFLEN];
  
  assert(client_fd >= 0);
  assert(req);
  
  memset(buf, '\0', CEREBRO_PACKET_BUFLEN);
  
  /* Wait for request from client */
  while (bytes_read < CEREBRO_METRIC_REQUEST_LEN)
    {
      fd_set rfds;
      struct timeval tv;
      int num;

      tv.tv_sec = CEREBRO_METRIC_PROTOCOL_SERVER_TIMEOUT_LEN;
      tv.tv_usec = 0;
      
      FD_ZERO(&rfds);
      FD_SET(client_fd, &rfds);
  
      if ((num = select(client_fd + 1, &rfds, NULL, NULL, &tv)) < 0)
	{
	  cerebro_err_debug("%s(%s:%d): select: %s", 
                            __FILE__, __FUNCTION__, __LINE__,
                            strerror(errno));
	  goto cleanup;
	}

      if (!num)
	{
	  /* Timed out.  If atleast some bytes were read, unmarshall
	   * the received bytes.  Its possible we are expecting more
	   * bytes than the client is sending, perhaps because we are
	   * using a different protocol version.  This will allow the
	   * server to return a invalid version number error back to
	   * the user.
	   */
	  if (!bytes_read)
	    goto cleanup;
	  else
	    goto unmarshall_received;
	}
      
      if (FD_ISSET(client_fd, &rfds))
	{
	  int n;

          /* Don't use fd_read_n b/c it loops until exactly
           * CEREBRO_METRIC_REQUEST_LEN is read.  Due to version
           * incompatability, we may want to read a smaller packet.
           */
	  if ((n = read(client_fd, 
                        buf + bytes_read, 
                        CEREBRO_METRIC_REQUEST_LEN - bytes_read)) < 0)
	    {
	      cerebro_err_debug("%s(%s:%d): read: %s", 
                                __FILE__, __FUNCTION__, __LINE__,
                                strerror(errno));
	      goto cleanup;
	    }

	  if (!n)
            {
              /* Pipe closed */
              goto cleanup;
            }

	  bytes_read += n;
	}
      else
	{
	  cerebro_err_debug("%s(%s:%d): num != 0 but fd not set",
                            __FILE__, __FUNCTION__, __LINE__);
	  goto cleanup;
	}
    }

 unmarshall_received:
  if ((rv = _cerebrod_metric_request_unmarshall(req, buf, bytes_read)) < 0)
    goto cleanup;

  return rv;

 cleanup:
  return -1;
}

/*  
 * _cerebrod_metric_request_dump
 *
 * dump contents of an metric request
 */
static void
_cerebrod_metric_request_dump(struct cerebro_metric_request *req)
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
      fprintf(stderr, "* Int32_Param1: %d\n", req->int32_param1);
      fprintf(stderr, "* Unsigned_Int32_Param1: %d\n", req->unsigned_int32_param1);
      fprintf(stderr, "* String_Param1: %s\n", req->string_param1);
      fprintf(stderr, "**************************************\n");
      Pthread_mutex_unlock(&debug_output_mutex);
    }
#endif /* CEREBRO_DEBUG */
}

/*
 * _cerebrod_metric_response_send_one
 *
 * send a response packet to the client
 *
 * Return 0 on success, -1 on error
 */
static int
_cerebrod_metric_response_send_one(int client_fd, 
				   struct cerebro_metric_response *res)
{
  char buf[CEREBRO_PACKET_BUFLEN];
  int res_len;

  assert(client_fd >= 0);
  assert(res);

  if ((res_len = _cerebrod_metric_response_marshall(res, 
						    buf, 
						    CEREBRO_PACKET_BUFLEN)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebrod_metric_response_marshall",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  if (fd_write_n(client_fd, buf, res_len) < 0)
    {
      cerebro_err_debug("%s(%s:%d): fd_write_n: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      return -1;
    }

  return 0;
}

/*
 * _cerebrod_metric_err_response_send
 *
 * send an error response packet to the client
 *
 * Return 0 on success, -1 on error
 */
static int
_cerebrod_metric_err_response_send(int client_fd, 
                                   struct cerebro_metric_err_response *err_res)
{
  char buf[CEREBRO_PACKET_BUFLEN];
  int err_res_len;

  assert(client_fd >= 0);
  assert(err_res);

  if ((err_res_len = _cerebrod_metric_err_response_marshall(err_res, 
                                                            buf, 
                                                            CEREBRO_PACKET_BUFLEN)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebrod_metric_err_response_marshall",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  if (fd_write_n(client_fd, buf, err_res_len) < 0)
    {
      cerebro_err_debug("%s(%s:%d): fd_write_n: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      return -1;
    }

  return 0;
}


/* 
 * _cerebrod_metric_respond_with_error
 *
 * respond to the metric_request with an error
 *
 * Return 0 on success, -1 on error
 */
static int
_cerebrod_metric_respond_with_error(int client_fd, 
                                    int32_t version,
                                    u_int32_t metric_err_code)
{
  struct cerebro_metric_response res;

  assert(client_fd >= 0);
  assert(metric_err_code == CEREBRO_METRIC_PROTOCOL_ERR_VERSION_INVALID
	 || metric_err_code == CEREBRO_METRIC_PROTOCOL_ERR_METRIC_UNKNOWN
	 || metric_err_code == CEREBRO_METRIC_PROTOCOL_ERR_PARAMETER_INVALID
         || metric_err_code == CEREBRO_METRIC_PROTOCOL_ERR_PACKET_INVALID
	 || metric_err_code == CEREBRO_METRIC_PROTOCOL_ERR_INTERNAL_SYSTEM_ERROR);
  
  memset(&res, '\0', CEREBRO_METRIC_RESPONSE_LEN);

  /* 
   * If the version sent is an older version, send a packet in
   * that older verion's packet format.  If it is a version that
   * we don't know of, fall through to the err_response section.
   */

  if (metric_err_code == CEREBRO_METRIC_PROTOCOL_ERR_VERSION_INVALID)
    {
      if (version == 1)
        {
          res.version = version;
          res.metric_err_code = CEREBRO_METRIC_PROTOCOL_ERR_VERSION_INVALID;
          res.end_of_responses = CEREBRO_METRIC_PROTOCOL_IS_LAST_RESPONSE;

          if (_cerebrod_metric_response_send_one(client_fd, &res) < 0)
            return -1;

          return 0;
        }
    }

  if (metric_err_code == CEREBRO_METRIC_PROTOCOL_ERR_VERSION_INVALID
      || metric_err_code == CEREBRO_METRIC_PROTOCOL_ERR_PACKET_INVALID)
    {
      struct cerebro_metric_err_response err_res;
      memset(&err_res, '\0', CEREBRO_METRIC_ERR_RESPONSE_LEN);
      
      err_res.version = version;
      err_res.metric_err_code = metric_err_code;

      if (_cerebrod_metric_err_response_send(client_fd, &err_res) < 0)
        return -1;

      return 0;
    }

  res.version = version;
  res.metric_err_code = metric_err_code;
  res.end_of_responses = CEREBRO_METRIC_PROTOCOL_IS_LAST_RESPONSE;
  
  if (_cerebrod_metric_response_send_one(client_fd, &res) < 0)
    return -1;
      
  return 0;
}

#if 0

/*  
 * _cerebrod_updown_evaluate_updown_state
 *
 * Callback function for list_for_each, to determine if a node is
 * up or down and if state data should be sent.
 *
 * Return 0 on success, -1 on error
 */
static int
_cerebrod_updown_evaluate_updown_state(void *x, void *arg)
{
  struct cerebrod_updown_node_data *ud;
  struct cerebrod_updown_evaluation_data *ed;
  struct cerebro_updown_response *res = NULL;
  u_int8_t updown_state;
#if CEREBRO_DEBUG
  int rv;
#endif /* CEREBRO_DEBUG */

  assert(x);
  assert(arg);

  ud = (struct cerebrod_updown_node_data *)x;
  ed = (struct cerebrod_updown_evaluation_data *)arg;
  
#if CEREBRO_DEBUG
  /* Should be called with lock already set */
  rv = Pthread_mutex_trylock(&updown_node_data_lock);
  if (rv != EBUSY)
    cerebro_err_exit("%s(%s:%d): mutex not locked: rv=%d",	
                     __FILE__, __FUNCTION__, __LINE__, rv);

  /* With locking, it shouldn't be possible for local time to be
   * greater than the time stored in any last_received time.
   */
  if (ed->time_now < ud->last_received)
    cerebro_err_debug("%s(%s:%d): last_received time later than time_now time",
                      __FILE__, __FUNCTION__, __LINE__);
#endif /* CEREBRO_DEBUG */

  if ((ed->time_now - ud->last_received) < ed->timeout_len)
    updown_state = CEREBRO_UPDOWN_PROTOCOL_STATE_NODE_UP;
  else
    updown_state = CEREBRO_UPDOWN_PROTOCOL_STATE_NODE_DOWN;

  if ((ed->updown_request == CEREBRO_UPDOWN_PROTOCOL_REQUEST_UP_NODES
       && updown_state != CEREBRO_UPDOWN_PROTOCOL_STATE_NODE_UP)
      || (ed->updown_request == CEREBRO_UPDOWN_PROTOCOL_REQUEST_DOWN_NODES
          && updown_state != CEREBRO_UPDOWN_PROTOCOL_STATE_NODE_DOWN))
    return 0;

  res = Malloc(sizeof(struct cerebro_updown_response));
  res->version = CEREBRO_UPDOWN_PROTOCOL_VERSION;
  res->updown_err_code = CEREBRO_UPDOWN_PROTOCOL_ERR_SUCCESS;
  res->end_of_responses = CEREBRO_UPDOWN_PROTOCOL_IS_NOT_LAST_RESPONSE;
#if CEREBRO_DEBUG
  if (ud->nodename && strlen(ud->nodename) > CEREBRO_MAXNODENAMELEN)
    cerebro_err_debug("%s(%s:%d): invalid node name length: %s", 
                      __FILE__, __FUNCTION__, __LINE__,
                      ud->nodename);
#endif /* CEREBRO_DEBUG */
  /* strncpy, b/c terminating character not required */
  strncpy(res->nodename, ud->nodename, CEREBRO_MAXNODENAMELEN);
  res->updown_state = updown_state;

  if (!list_append(ed->node_responses, res))
    {
      cerebro_err_debug("%s(%s:%d): list_append: %s", 
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      Free(res);
      return -1;
    }

  return 0;
}

/*
 * _cerebrod_updown_response_send_all
 *
 * respond to the updown_request with a node
 *
 * Return 0 on success, -1 on error
 */
static int
_cerebrod_updown_response_send_all(void *x, void *arg)
{
  struct cerebro_updown_response *res;
  int client_fd;

  assert(x);
  assert(arg);

  res = (struct cerebro_updown_response *)x;
  client_fd = *((int *)arg);

  if (_cerebrod_updown_response_send_one(client_fd, res) < 0)
    return -1;

  return 0;
}

/*  
 * _cerebrod_updown_respond_with_updown_nodes
 *
 * Return 0 on success, -1 on error
 */
static int
_cerebrod_updown_respond_with_updown_nodes(int client_fd,
                                           int32_t version,
					   u_int32_t updown_request, 
					   u_int32_t timeout_len)
{
  struct cerebrod_updown_evaluation_data ed;
  struct cerebro_updown_response end_res;
  struct timeval tv;
  List node_responses = NULL;

  assert(client_fd >= 0);
  assert(version == CEREBRO_UPDOWN_PROTOCOL_VERSION);
  assert(updown_request == CEREBRO_UPDOWN_PROTOCOL_REQUEST_UP_NODES
	 || updown_request == CEREBRO_UPDOWN_PROTOCOL_REQUEST_DOWN_NODES
	 || updown_request == CEREBRO_UPDOWN_PROTOCOL_REQUEST_UP_AND_DOWN_NODES);
  assert(timeout_len);
  assert(updown_node_data);

  memset(&ed, '\0', sizeof(struct cerebrod_updown_evaluation_data));

  Pthread_mutex_lock(&updown_node_data_lock);
  
  if (!List_count(updown_node_data))
    {
      Pthread_mutex_unlock(&updown_node_data_lock);
      goto end_response;
    }

  if (!(node_responses = list_create((ListDelF)_Free)))
    {
      cerebro_err_debug("%s(%s:%d): list_create: %s", 
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      Pthread_mutex_unlock(&updown_node_data_lock);
      _cerebrod_updown_respond_with_error(client_fd,
                                          version,
					  CEREBRO_UPDOWN_PROTOCOL_ERR_INTERNAL_SYSTEM_ERROR);
      goto cleanup;
    }

  Gettimeofday(&tv, NULL);
  ed.client_fd = client_fd;
  ed.updown_request = updown_request;
  ed.timeout_len = timeout_len;
  ed.time_now = tv.tv_sec;
  ed.node_responses = node_responses;

  if (list_for_each(updown_node_data, 
		    _cerebrod_updown_evaluate_updown_state, 
		    &ed) < 0)
    {
      Pthread_mutex_unlock(&updown_node_data_lock);
      _cerebrod_updown_respond_with_error(client_fd,
                                          version,
					  CEREBRO_UPDOWN_PROTOCOL_ERR_INTERNAL_SYSTEM_ERROR);
      goto cleanup;
    }

  /* Evaluation is done.  Transmission of the results can be done
   * without this lock.
   */
  Pthread_mutex_unlock(&updown_node_data_lock);

  if (List_count(node_responses))
    {
      if (list_for_each(node_responses,
			_cerebrod_updown_response_send_all, 
			&client_fd) < 0)
        {
          Pthread_mutex_unlock(&updown_node_data_lock);
          _cerebrod_updown_respond_with_error(client_fd,
                                              version,
                                              CEREBRO_UPDOWN_PROTOCOL_ERR_INTERNAL_SYSTEM_ERROR);
          goto cleanup;
        }
    }

 end_response:
  /* Send end response */
  memset(&end_res, '\0', sizeof(struct cerebro_updown_response));
  end_res.version = CEREBRO_UPDOWN_PROTOCOL_VERSION;
  end_res.updown_err_code = CEREBRO_UPDOWN_PROTOCOL_ERR_SUCCESS;
  end_res.end_of_responses = CEREBRO_UPDOWN_PROTOCOL_IS_LAST_RESPONSE;

  if (_cerebrod_updown_response_send_one(client_fd, &end_res) < 0)
    return -1;

  list_destroy(node_responses);
  return 0;

 cleanup:
  if (node_responses)
    list_destroy(node_responses);
  return -1;
}

#endif /* 0 */

/* 
 * _cerebrod_metric_service_connection
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
_cerebrod_metric_service_connection(void *arg)
{
  int client_fd, req_len;
  struct cerebro_metric_request req;
  client_fd = *((int *)arg);

  memset(&req, '\0', sizeof(struct cerebro_metric_request));
  if ((req_len = _cerebrod_metric_request_receive(client_fd, &req)) < 0)
    /* At this point, there is no successfully read request, therefore
     * there is no reason to respond w/ a response and an error code.
     */
    goto out;
  
  _cerebrod_metric_request_dump(&req);

  if (req_len != CEREBRO_METRIC_REQUEST_LEN)
    {
      cerebro_err_debug("%s(%s:%d): received packet unexpected size: "
                        "expect %d, req_len %d", 
                        __FILE__, __FUNCTION__, __LINE__,
                        CEREBRO_METRIC_REQUEST_LEN, req_len);

      if (req_len >= sizeof(req.version)
          && req.version != CEREBRO_METRIC_PROTOCOL_VERSION)
        {
          _cerebrod_metric_respond_with_error(client_fd, 
                                              req.version,
                                              CEREBRO_METRIC_PROTOCOL_ERR_VERSION_INVALID);
          goto out;
        }

      _cerebrod_metric_respond_with_error(client_fd, 
                                          req.version,
                                          CEREBRO_METRIC_PROTOCOL_ERR_PACKET_INVALID);
      goto out;
    }

  if (req.version != CEREBRO_METRIC_PROTOCOL_VERSION)
    {
      _cerebrod_metric_respond_with_error(client_fd, 
                                          req.version,
                                          CEREBRO_METRIC_PROTOCOL_ERR_VERSION_INVALID);
      goto out;
    }

  if (!strcmp(req.metric_name, CEREBRO_METRIC_STARTTIME)
      || !strcmp(req.metric_name, CEREBRO_METRIC_BOOTTIME))
    {
      _cerebrod_metric_respond_with_error(client_fd,
                                          req.version,
					  CEREBRO_METRIC_PROTOCOL_ERR_METRIC_UNKNOWN);
      goto out;
    }

#if 0
  if (_cerebrod_metric_respond_with_nodes(client_fd, 
                                          req.version,
                                          req.metric_request, 
                                          req.timeout_len) < 0)
    goto out;

#endif /* 0 */
 out:
  Free(arg);
  Close(client_fd);
  return NULL;
}

void *
cerebrod_metric(void *arg)
{
  _cerebrod_metric_initialize();

  cerebrod_tcp_data_server(_cerebrod_metric_service_connection, 
                           conf.metric_server_port,
                           CEREBROD_METRIC_BACKLOG,
                           CEREBROD_METRIC_REINITIALIZE_WAIT);

  return NULL;			/* NOT REACHED */
}
