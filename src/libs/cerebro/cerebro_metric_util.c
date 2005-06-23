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

#include "cerebro.h"
#include "cerebro/cerebro_error.h"
#include "cerebro/cerebro_metric_protocol.h"

#include "cerebro_util.h"

#include "fd.h"
#include "marshall.h"

static int
_cerebro_metric_protocol_err_conversion(u_int32_t protocol_error)
{
  switch(protocol_error)
    {
    case CEREBRO_METRIC_PROTOCOL_ERR_SUCCESS:
      return CEREBRO_ERR_SUCCESS;
    case CEREBRO_METRIC_PROTOCOL_ERR_VERSION_INVALID:
      return CEREBRO_ERR_VERSION_INCOMPATIBLE;
    case CEREBRO_METRIC_PROTOCOL_ERR_PACKET_INVALID:
      return CEREBRO_ERR_PROTOCOL;
    case CEREBRO_METRIC_PROTOCOL_ERR_METRIC_UNKNOWN:
      return CEREBRO_ERR_METRIC_UNKNOWN;
    case CEREBRO_METRIC_PROTOCOL_ERR_PARAMETER_INVALID:
    case CEREBRO_METRIC_PROTOCOL_ERR_INTERNAL_SYSTEM_ERROR:
      return CEREBRO_ERR_INTERNAL;
    default:
      cerebro_err_debug("%s(%s:%d): invalid protocol error code: %d",
                        __FILE__, __FUNCTION__, __LINE__, protocol_error);
      return CEREBRO_ERR_INTERNAL;
    }
}

int
_cerebro_metric_config(cerebro_t handle,
                       unsigned int *port,
                       unsigned int *timeout_len,
                       unsigned int *flags)
{
  if (_cerebro_handle_check(handle) < 0)
    return -1;

  if (!port || !timeout_len || !flags)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (_cerebro_load_config(handle) < 0)
    return -1;

  if (!handle->port)
    {
      if (handle->config_data.cerebro_port_flag)
        {
          if (!handle->config_data.cerebro_port)
            {
              handle->errnum = CEREBRO_ERR_CONFIG_INPUT;
              return -1;
            }
          *port = handle->config_data.cerebro_port;
        }
      else
        *port = CEREBRO_METRIC_SERVER_PORT;
    }
  else
    *port = handle->port;

  if (!handle->timeout_len)
    {
      if (handle->config_data.cerebro_timeout_len_flag)
        {
          if (!handle->config_data.cerebro_timeout_len)
            {
              handle->errnum = CEREBRO_ERR_CONFIG_INPUT;
              return -1;
            }
          *timeout_len = handle->config_data.cerebro_timeout_len;
        }
      else
        *timeout_len = CEREBRO_METRIC_TIMEOUT_LEN_DEFAULT;
    }
  else
    *timeout_len = handle->timeout_len;

  if (!handle->flags)
    {
      if (handle->config_data.cerebro_flags_flag)
        {
          if (handle->config_data.cerebro_flags & ~CEREBRO_METRIC_FLAGS_MASK)
            {
              handle->errnum = CEREBRO_ERR_CONFIG_INPUT;
              return -1;
            }
          *flags = handle->config_data.cerebro_flags;
        }
      *flags = CEREBRO_METRIC_FLAGS_DEFAULT;
    }
  else
    *flags = handle->flags;

  return 0;
}

/*
 * _cerebro_metric_request_marshall
 *
 * Marshall contents of a metric server request
 *
 * Returns length written to buffer on success, -1 on error
 */
static int
_cerebro_metric_request_marshall(cerebro_t handle,
                                 struct cerebro_metric_request *req,
                                 char *buf,
                                 unsigned int buflen)
{
  int len, count = 0;

#if CEREBRO_DEBUG
  if (!buf)
    {
      cerebro_err_debug("%s(%s:%d): buf null",
                        __FILE__, __FUNCTION__, __LINE__);
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (buflen < CEREBRO_METRIC_REQUEST_PACKET_LEN)
    {
      cerebro_err_debug("%s(%s:%d): buflen invalid",
                        __FILE__, __FUNCTION__, __LINE__);
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
#endif /* CEREBRO_DEBUG */

  memset(buf, '\0', buflen);

  if ((len = marshall_int32(req->version,
                            buf + count,
                            buflen - count)) <= 0)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  count += len;

  if ((len = marshall_buffer(req->metric_name,
                             sizeof(req->metric_name),
                             buf + count,
                             buflen - count)) <= 0)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  count += len;

  if ((len = marshall_u_int32(req->timeout_len,
                              buf + count,
                              buflen - count)) <= 0)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  count += len;

  if ((len = marshall_u_int32(req->flags,
                              buf + count,
                              buflen - count)) <= 0)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  count += len;

  return count;
}

int
_cerebro_metric_request_send(cerebro_t handle,
                             int fd,
                             const char *metric_name,
                             unsigned int timeout_len,
                             int flags)
{
  struct cerebro_metric_request req;
  char buf[CEREBRO_PACKET_BUFLEN];
  int req_len;

  req.version = CEREBRO_METRIC_PROTOCOL_VERSION;
  strncpy(req.metric_name, metric_name, CEREBRO_METRIC_NAME_MAXLEN);
  req.flags = flags;
  req.timeout_len = timeout_len;

  if ((req_len = _cerebro_metric_request_marshall(handle,
                                                  &req,
                                                  buf,
                                                  CEREBRO_PACKET_BUFLEN)) < 0)
    return -1;

  if (fd_write_n(fd, buf, req_len) < 0)
    {
      cerebro_err_debug("%s(%s:%d): fd_write_n: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  return 0;
}
