/*****************************************************************************\
 *  $id: cerebro_metric.c,v 1.17 2005/06/07 16:18:58 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <errno.h>

#include "cerebro.h"
#include "cerebro/cerebro_metric_control_protocol.h"

#include "cerebro_api.h"
#include "cerebro_config_util.h"
#include "cerebro_util.h"

#include "debug.h"
#include "fd.h"
#include "marshall.h"
#include "network_util.h"

/*
 * _metric_control_protocol_err_code_conversion
 *
 * Convert metric control protocol err codes to API err codes
 *
 * Returns proper err code
 */
static int
_metric_control_protocol_err_code_conversion(u_int32_t err_code)
{
  switch(err_code)
    {
    case CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_SUCCESS:
      return CEREBRO_ERR_SUCCESS;
    case CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_VERSION_INVALID:
      return CEREBRO_ERR_VERSION_INCOMPATIBLE;
    case CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_COMMAND_INVALID:
      return CEREBRO_ERR_PROTOCOL;
    case CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_METRIC_INVALID:
      return CEREBRO_ERR_METRIC_UNKNOWN;
    case CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_PARAMETER_INVALID:
      return CEREBRO_ERR_PROTOCOL;
    case CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_PACKET_INVALID:
      return CEREBRO_ERR_PROTOCOL;
    case CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_INTERNAL_ERROR:
      CEREBRO_DBG(("control internal system error"));
      return CEREBRO_ERR_INTERNAL;
    default:
      CEREBRO_DBG(("invalid protocol error code: %d", err_code));
      return CEREBRO_ERR_INTERNAL;
    }
}

/*
 * _metric_control_request_marshall
 *
 * Marshall contents of a metric control request
 *
 * Returns length written to buffer on success, -1 on error
 */
static int
_metric_control_request_marshall(cerebro_t handle,
                                struct cerebro_metric_control_request *req,
                                char *buf,
                                unsigned int buflen)
{
  int n, bufPtrlen, c = 0;
  char *bufPtr;
  u_int32_t mtype, mlen;
  void *mvalue;

  if (!buf || buflen < CEREBRO_METRIC_CONTROL_REQUEST_HEADER_LEN)
    {
      CEREBRO_DBG(("invalid parameters"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  memset(buf, '\0', buflen);

  if ((n = marshall_int32(req->version, buf + c, buflen - c)) <= 0)
    {
      CEREBRO_DBG(("marshall_int32"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += n;

  if ((n = marshall_int32(req->command, buf + c, buflen - c)) <= 0)
    {
      CEREBRO_DBG(("marshall_int32"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += n;

  bufPtr = req->metric_name;
  bufPtrlen = sizeof(req->metric_name);
  if ((n = marshall_buffer(bufPtr, bufPtrlen, buf + c, buflen - c)) <= 0)
    {
      CEREBRO_DBG(("marshall_buffer"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += n;

  mtype = req->metric_value_type;
  if ((n = marshall_u_int32(mtype, buf + c, buflen - c)) <= 0)
    {
      CEREBRO_DBG(("marshall_u_int32"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += n;

  mlen = req->metric_value_type;
  if ((n = marshall_u_int32(mlen, buf + c, buflen - c)) <= 0)
    {
      CEREBRO_DBG(("marshall_u_int32"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += n;

  if (!mlen)
    return c;

  mvalue = req->metric_value;

  if (!mvalue)
    {
      CEREBRO_DBG(("metric value invalid"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  
  if (mtype == CEREBRO_METRIC_VALUE_TYPE_NONE)
    {
      CEREBRO_DBG(("metric value len > 0 for type NONE"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_INT32)
    {
      if ((n = marshall_int32(*((int32_t *)mvalue), buf + c, buflen - c)) <= 0)
        {
          CEREBRO_DBG(("marshall_int32"));
          handle->errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_U_INT32)
    {
      if ((n = marshall_u_int32(*((u_int32_t *)mvalue), buf + c, buflen - c)) <= 0)
        {
          CEREBRO_DBG(("marshall_u_int32"));
          handle->errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_FLOAT)
    {
      if ((n = marshall_float(*((float *)mvalue), buf + c, buflen - c)) <= 0)
        {
          CEREBRO_DBG(("marshall_float"));
          handle->errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_DOUBLE)
    {
      if ((n = marshall_double(*((double *)mvalue), buf + c, buflen - c)) <= 0)
        {
          CEREBRO_DBG(("marshall_double"));
          handle->errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_STRING)
    {
      if ((n = marshall_buffer(mvalue, mlen, buf + c, buflen - c)) <= 0)
        {
          CEREBRO_DBG(("marshall_buffer"));
          handle->errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }
  else
    {
      CEREBRO_DBG(("invalid type %d", mtype));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  c += n;

  return c;
}

/*
 * _metric_control_response_check
 *
 * Check the version and error code
 *
 * Returns 0 on success, -1 on error
 */
static int
_metric_control_response_check(cerebro_t handle, 
                               const char *buf, 
                               unsigned int buflen)
{
  int n, len = 0;
  int32_t version;
  u_int32_t err_code;

  if ((n = unmarshall_int32(&version, buf + len, buflen - len)) < 0)
    {
      CEREBRO_DBG(("unmarshall_int32"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (!n)
    {
      handle->errnum = CEREBRO_ERR_PROTOCOL;
      return -1;
    }
  len += n;

  if ((n = unmarshall_u_int32(&err_code, buf + len, buflen - len)) < 0)
    {
      CEREBRO_DBG(("unmarshall_u_int32"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (!n)
    {
      handle->errnum = CEREBRO_ERR_PROTOCOL;
      return -1;
    }
  len += n;
  
  if (version != CEREBRO_METRIC_CONTROL_PROTOCOL_VERSION)
    {
      handle->errnum = CEREBRO_ERR_VERSION_INCOMPATIBLE;
      return -1;
    }

  if (err_code != CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_SUCCESS)
    {
      handle->errnum = _metric_control_protocol_err_code_conversion(err_code);
      return -1;
    }
  
  return 0;
}

#if 0

/*
 * _metric_server_response_header_unmarshall
 *
 * Unmarshall contents of a metric server response header
 *
 * Returns 0 on success, -1 on error
 */
static int
_metric_server_response_header_unmarshall(cerebro_t handle,
                                          struct cerebro_metric_server_response *res,
                                          const char *buf,
                                          unsigned int buflen)
{
  u_int32_t *mtypePtr, *mlenPtr;
  int n, bufPtrlen, c = 0;
  char *bufPtr;

  if (!res || !buf || buflen < CEREBRO_METRIC_SERVER_RESPONSE_HEADER_LEN)
    {
      CEREBRO_DBG(("invalid pointers"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if ((n = unmarshall_u_int32(&(res->err_code), buf + c, buflen - c)) < 0)
    {
      CEREBRO_DBG(("unmarshall_u_int32"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += n;

  if ((n = unmarshall_u_int8(&(res->end), buf + c, buflen - c)) < 0)
    {
      CEREBRO_DBG(("unmarshall_u_int8"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += n;

  bufPtr = res->name;
  bufPtrlen = sizeof(res->name);
  if ((n = unmarshall_buffer(bufPtr, bufPtrlen, buf + c, buflen - c)) < 0)
    {
      CEREBRO_DBG(("unmarshall_buffer"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += n;

  mtypePtr = &(res->metric_value_type);
  if ((n = unmarshall_u_int32(mtypePtr, buf + c,  buflen - c)) < 0)
    {
      CEREBRO_DBG(("unmarshall_u_int32"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += n;
  
  mlenPtr = &(res->metric_value_len);
  if ((n = unmarshall_u_int32(mlenPtr, buf + c, buflen - c)) < 0)
    {
      CEREBRO_DBG(("unmarshall_u_int32"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += n;

  if (c != CEREBRO_METRIC_SERVER_RESPONSE_HEADER_LEN)
    {
      handle->errnum = CEREBRO_ERR_PROTOCOL;
      return -1;
    }

  return 0;
}

/*
 * _get_metric_data 
 *
 * Get metric data and store it appropriately
 *
 * Returns 0 on success, -1 on error
 */
static int
_get_metric_data(cerebro_t handle,
                 void *list,
                 const char *metric_name,
                 const char *hostname,
                 unsigned int port,
                 unsigned int timeout_len,
                 int flags,
                 Cerebro_metric_receive_response receive_response)
{
  int fd = -1, rv = -1;
                                                                  
  if (!metric_name || !hostname || !receive_response)
    {
      CEREBRO_DBG(("invalid parameters"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  
  if ((fd = _cerebro_low_timeout_connect(handle,
                                         hostname,
                                         port,
                                         CEREBRO_METRIC_SERVER_PROTOCOL_CONNECT_TIMEOUT_LEN)) < 0)
    goto cleanup;
  
  if (_metric_server_request_send(handle,
                                  fd,
                                  metric_name,
                                  timeout_len,
                                  flags) < 0)
    goto cleanup;

  while (1)
    {
      struct cerebro_metric_server_response res;
      char buf[CEREBRO_MAX_PACKET_LEN];
      char nodename_buf[CEREBRO_MAX_NODENAME_LEN+1];
      int bytes_read;
      unsigned int errnum;

      if ((bytes_read = receive_data(fd,
                                     CEREBRO_METRIC_SERVER_RESPONSE_HEADER_LEN,
                                     buf,
                                     CEREBRO_MAX_PACKET_LEN,
                                     CEREBRO_METRIC_SERVER_PROTOCOL_CLIENT_TIMEOUT_LEN,
                                     &errnum)) < 0)
        {
          handle->errnum = errnum;
          goto cleanup;
        }

      if (bytes_read < CEREBRO_METRIC_SERVER_ERR_RESPONSE_LEN)
        {
          handle->errnum = CEREBRO_ERR_PROTOCOL;
          goto cleanup;
        }

      if (_metric_server_response_check(handle, buf, bytes_read) < 0)
        goto cleanup;


      if (bytes_read < CEREBRO_METRIC_SERVER_RESPONSE_HEADER_LEN)
        {
          handle->errnum = CEREBRO_ERR_PROTOCOL;
          goto cleanup;
        }

      if (_metric_server_response_header_unmarshall(handle, 
                                                    &res, 
                                                    buf, 
                                                    bytes_read) < 0)
        goto cleanup;
     
      if (res.end == CEREBRO_METRIC_SERVER_PROTOCOL_IS_LAST_RESPONSE)
        break;

      if (receive_response(handle, list, &res, bytes_read, fd) < 0)
        goto cleanup;
    }
 
  rv = 0;
 cleanup:
  close(fd);
  return rv;
}

#endif /* 0 */

int 
cerebro_register_metric(cerebro_t handle, const char *metric_name)
{
  struct cerebro_metric_control_request req;
  char reqbuf[CEREBRO_MAX_PACKET_LEN];
  char resbuf[CEREBRO_MAX_PACKET_LEN];
  struct sockaddr_un addr;
  unsigned int errnum;
  int fd = -1;
  int rv = -1;
  int req_len, bytes_read;

  if (_cerebro_handle_check(handle) < 0)
    return -1;

  if (!metric_name || strlen(metric_name) > CEREBRO_MAX_METRIC_NAME_LEN)
    {
      handle->errnum = CEREBRO_ERR_PARAMETERS;
      return -1;
    }

  if (_cerebro_load_config(handle) < 0)
    goto cleanup;

  if ((fd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
    {
      CEREBRO_DBG(("socket: %s", strerror(errno)));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  memset(&addr, '\0', sizeof(struct sockaddr_un));
  addr.sun_family = AF_LOCAL;
  strncpy(addr.sun_path, CEREBRO_METRIC_CONTROL_PATH, sizeof(addr.sun_path));

  if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) < 0)
    {
      CEREBRO_DBG(("connect: %s", strerror(errno)));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  memset(&req, '\0', sizeof(struct cerebro_metric_control_request));
  req.version = CEREBRO_METRIC_CONTROL_PROTOCOL_VERSION;
  req.command = CEREBRO_METRIC_CONTROL_PROTOCOL_CMD_REGISTER;
  strncpy(req.metric_name, metric_name, CEREBRO_MAX_METRIC_NAME_LEN);
  req.metric_value_type = 0;
  req.metric_value_len = 0;

  if ((req_len = _metric_control_request_marshall(handle, 
                                                  &req, 
                                                  reqbuf, 
                                                  CEREBRO_MAX_PACKET_LEN)) < 0)
    goto cleanup;

  if (fd_write_n(fd, reqbuf, req_len) < 0)
    {
      CEREBRO_DBG(("fd_write_n: %s", strerror(errno)));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  if ((bytes_read = receive_data(fd, 
                                 CEREBRO_METRIC_CONTROL_RESPONSE_LEN,
                                 resbuf,
                                 CEREBRO_MAX_PACKET_LEN,
                                 CEREBRO_METRIC_CONTROL_PROTOCOL_CLIENT_TIMEOUT_LEN,
                                 &errnum)) < 0)
    {
      handle->errnum = errnum;
      goto cleanup;
    }
               
  if (bytes_read != CEREBRO_METRIC_CONTROL_RESPONSE_LEN)
    {
      handle->errnum = CEREBRO_ERR_PROTOCOL;
      goto cleanup;
    }

  if (_metric_control_response_check(handle, resbuf, bytes_read) < 0)
    goto cleanup;

  handle->errnum = CEREBRO_ERR_SUCCESS;
  rv = 0;
 cleanup:
  close(fd);
  return rv;
}

