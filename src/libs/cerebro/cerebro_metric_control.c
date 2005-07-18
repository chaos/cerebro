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
 * _setup_metric_control_fd
 *
 * Setup the metric control file descriptor
 *
 * Returns fd on success, -1 on error
 */
static int
_setup_metric_control_fd(cerebro_t handle)
{
  struct sockaddr_un addr;
  int fd = -1;

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
      handle->errnum = CEREBRO_ERR_CONNECT;
      goto cleanup;
    }

  return fd;

 cleanup:
  close(fd);
  return -1;
}

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
      return CEREBRO_ERR_METRIC_INVALID;
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

  mlen = req->metric_value_len;
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
 * _metric_control_request_send
 *
 * Send the metric control request
 *
 * Returns 0 on success, -1 on error
 */
static int
_metric_control_request_send(cerebro_t handle,
                             int fd,
                             int command, 
                             const char *metric_name,
                             unsigned int metric_value_type,
                             unsigned int metric_value_len,
                             void *metric_value)
{
  struct cerebro_metric_control_request req;
  char buf[CEREBRO_MAX_PACKET_LEN];
  int req_len;

  if (!(command >= CEREBRO_METRIC_CONTROL_PROTOCOL_CMD_REGISTER
        && command <= CEREBRO_METRIC_CONTROL_PROTOCOL_CMD_RESTART)
      || !metric_name
      || !(metric_value_type >= CEREBRO_METRIC_VALUE_TYPE_NONE
           && metric_value_type <= CEREBRO_METRIC_VALUE_TYPE_STRING))
    {
      CEREBRO_DBG(("invalid parameters"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  memset(&req, '\0', sizeof(struct cerebro_metric_control_request));
  req.version = CEREBRO_METRIC_CONTROL_PROTOCOL_VERSION;
  req.command = command;
  strncpy(req.metric_name, metric_name, CEREBRO_MAX_METRIC_NAME_LEN);
  req.metric_value_type = metric_value_type;
  req.metric_value_len = metric_value_len;
  req.metric_value = metric_value;

  if ((req_len = _metric_control_request_marshall(handle, 
                                                  &req, 
                                                  buf, 
                                                  CEREBRO_MAX_PACKET_LEN)) < 0)
    return -1;
  
  if (fd_write_n(fd, buf, req_len) < 0)
    {
      CEREBRO_DBG(("fd_write_n: %s", strerror(errno)));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  return 0;
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

/*
 * _metric_control_response_receive
 *
 * Response a metric control response
 *
 * Returns 0 on success, -1 on error
 */
static int
_metric_control_response_receive(cerebro_t handle, int fd)
{
  char buf[CEREBRO_MAX_PACKET_LEN];
  unsigned int errnum;
  int bytes_read;

  if ((bytes_read = receive_data(fd, 
                                 CEREBRO_METRIC_CONTROL_RESPONSE_LEN,
                                 buf,
                                 CEREBRO_MAX_PACKET_LEN,
                                 CEREBRO_METRIC_CONTROL_PROTOCOL_CLIENT_TIMEOUT_LEN,
                                 &errnum)) < 0)
    {
      handle->errnum = errnum;
      return -1;
    }
               
  if (bytes_read != CEREBRO_METRIC_CONTROL_RESPONSE_LEN)
    {
      handle->errnum = CEREBRO_ERR_PROTOCOL;
      return -1;
    }

  if (_metric_control_response_check(handle, buf, bytes_read) < 0)
    return -1;

  return 0;
}

/* 
 * _cerebro_metric_control
 *
 * Common code for cerebro metric control API
 *
 * Returns 0 on success, -1 on error
 */
static int
_cerebro_metric_control(cerebro_t handle, 
                        unsigned int command,
                        const char *metric_name, 
                        unsigned int metric_value_type,
                        unsigned int metric_value_len,
                        void *metric_value)
{
  int fd = -1, rv = -1;

  if (_cerebro_handle_check(handle) < 0)
    return -1;

  if (!metric_name || strlen(metric_name) > CEREBRO_MAX_METRIC_NAME_LEN)
    {
      handle->errnum = CEREBRO_ERR_PARAMETERS;
      return -1;
    }

  if (command == CEREBRO_METRIC_CONTROL_PROTOCOL_CMD_UPDATE
      && (!(metric_value_type >= CEREBRO_METRIC_VALUE_TYPE_NONE
            && metric_value_type <= CEREBRO_METRIC_VALUE_TYPE_STRING)
          || (metric_value_type == CEREBRO_METRIC_VALUE_TYPE_NONE
              && metric_value_len)
          || (metric_value_type == CEREBRO_METRIC_VALUE_TYPE_NONE
              && metric_value)
          || (metric_value_type != CEREBRO_METRIC_VALUE_TYPE_NONE
              && !metric_value)
          || (metric_value_type == CEREBRO_METRIC_VALUE_TYPE_INT32
              && metric_value_len != sizeof(int32_t))
          || (metric_value_type == CEREBRO_METRIC_VALUE_TYPE_U_INT32
              && metric_value_len != sizeof(u_int32_t))
          || (metric_value_type == CEREBRO_METRIC_VALUE_TYPE_FLOAT
              && metric_value_len != sizeof(float))
          || (metric_value_type == CEREBRO_METRIC_VALUE_TYPE_DOUBLE
              && metric_value_len != sizeof(double))))
    {
      handle->errnum = CEREBRO_ERR_PARAMETERS;
      return -1;
    }

  if (command == CEREBRO_METRIC_CONTROL_PROTOCOL_CMD_UPDATE
      && metric_value_type == CEREBRO_METRIC_VALUE_TYPE_STRING
      && !metric_value_len)
    metric_value_type = CEREBRO_METRIC_VALUE_TYPE_NONE;

  if (_cerebro_load_config(handle) < 0)
    goto cleanup;

  if ((fd = _setup_metric_control_fd(handle)) < 0)
    goto cleanup;

  if (_metric_control_request_send(handle,
                                   fd,
                                   command,
                                   metric_name,
                                   metric_value_type,
                                   metric_value_len,
                                   metric_value) < 0)
    goto cleanup;

  if (_metric_control_response_receive(handle, fd) < 0)
    goto cleanup;

  handle->errnum = CEREBRO_ERR_SUCCESS;
  rv = 0;
 cleanup:
  close(fd);
  return rv;
}

int 
cerebro_register_metric(cerebro_t handle, const char *metric_name)
{
  return _cerebro_metric_control(handle, 
                                 CEREBRO_METRIC_CONTROL_PROTOCOL_CMD_REGISTER,
                                 metric_name, 
                                 CEREBRO_METRIC_VALUE_TYPE_NONE,
                                 0,
                                 NULL);
}

int 
cerebro_unregister_metric(cerebro_t handle, const char *metric_name)
{
  return _cerebro_metric_control(handle, 
                                 CEREBRO_METRIC_CONTROL_PROTOCOL_CMD_UNREGISTER,
                                 metric_name, 
                                 CEREBRO_METRIC_VALUE_TYPE_NONE,
                                 0,
                                 NULL);
}

int
cerebro_update_metric_value(cerebro_t handle,
                            const char *metric_name,
                            unsigned int metric_value_type,
                            unsigned int metric_value_len,
                            void *metric_value)
{
  return _cerebro_metric_control(handle, 
                                 CEREBRO_METRIC_CONTROL_PROTOCOL_CMD_UPDATE,
                                 metric_name, 
                                 metric_value_type,
                                 metric_value_len,
                                 metric_value);
}

int 
cerebro_restart_metric(cerebro_t handle, const char *metric_name)
{
  return _cerebro_metric_control(handle, 
                                 CEREBRO_METRIC_CONTROL_PROTOCOL_CMD_RESTART,
                                 metric_name, 
                                 CEREBRO_METRIC_VALUE_TYPE_NONE,
                                 0,
                                 NULL);
}

