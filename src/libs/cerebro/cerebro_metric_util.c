/*****************************************************************************\
 *  $id: cerebro_metric.c,v 1.17 2005/06/07 16:18:58 achu Exp $
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

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <errno.h>

#include "cerebro.h"

#include "protocol/cerebro_metric_server_protocol.h"

#include "cerebro_api.h"
#include "cerebro_config_util.h"
#include "cerebro_metric_util.h"
#include "cerebro_util.h"

#include "debug.h"
#include "fd.h"
#include "marshall.h"
#include "metric_util.h"
#include "network_util.h"

/*
 * _metric_server_protocol_err_code_conversion
 *
 * Convert metric server protocol err codes to API err codes
 *
 * Returns proper err code
 */
static int
_metric_server_protocol_err_code_conversion(u_int32_t err_code)
{
  switch(err_code)
    {
    case CEREBRO_METRIC_SERVER_PROTOCOL_ERR_SUCCESS:
      return CEREBRO_ERR_SUCCESS;
    case CEREBRO_METRIC_SERVER_PROTOCOL_ERR_VERSION_INVALID:
      return CEREBRO_ERR_VERSION_INCOMPATIBLE;
    case CEREBRO_METRIC_SERVER_PROTOCOL_ERR_PACKET_INVALID:
      return CEREBRO_ERR_PROTOCOL;
    case CEREBRO_METRIC_SERVER_PROTOCOL_ERR_METRIC_INVALID:
      return CEREBRO_ERR_METRIC_INVALID;
    case CEREBRO_METRIC_SERVER_PROTOCOL_ERR_PARAMETER_INVALID:
      return CEREBRO_ERR_PROTOCOL;
    case CEREBRO_METRIC_SERVER_PROTOCOL_ERR_INTERNAL_ERROR:
      CEREBRO_DBG(("server internal system error"));
      return CEREBRO_ERR_INTERNAL;
    default:
      CEREBRO_DBG(("invalid protocol error code: %d", err_code));
      return CEREBRO_ERR_INTERNAL;
    }
}

/*
 * _metric_server_request_marshall
 *
 * Marshall contents of a metric server request
 *
 * Returns length written to buffer on success, -1 on error
 */
static int
_metric_server_request_marshall(cerebro_t handle,
                                struct cerebro_metric_server_request *req,
                                char *buf,
                                unsigned int buflen)
{
  int n, bufPtrlen, c = 0;
  char *bufPtr;

  if (!buf || buflen < CEREBRO_METRIC_SERVER_REQUEST_PACKET_LEN)
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

  bufPtr = req->metric_name;
  bufPtrlen = sizeof(req->metric_name);
  if ((n = marshall_buffer(bufPtr, bufPtrlen, buf + c, buflen - c)) <= 0)
    {
      CEREBRO_DBG(("marshall_buffer"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += n;

  if ((n = marshall_u_int32(req->timeout_len, buf + c, buflen - c)) <= 0)
    {
      CEREBRO_DBG(("marshall_u_int32"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += n;

  if ((n = marshall_u_int32(req->flags, buf + c, buflen - c)) <= 0)
    {
      CEREBRO_DBG(("marshall_u_int32"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += n;

  return c;
}

/*
 * _metric_server_request_send
 *
 * Send the metric server request
 *
 * Returns 0 on success, -1 on error
 */
static int
_metric_server_request_send(cerebro_t handle,
                            int fd,
                            const char *metric_name,
                            unsigned int timeout_len,
                            int flags)
{
  struct cerebro_metric_server_request req;
  char buf[CEREBRO_MAX_PACKET_LEN];
  int req_len;

  if (!metric_name)
    {
      CEREBRO_DBG(("invalid parameters"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  req.version = CEREBRO_METRIC_SERVER_PROTOCOL_VERSION;
  strncpy(req.metric_name, metric_name, CEREBRO_MAX_METRIC_NAME_LEN);
  req.flags = flags;
  req.timeout_len = timeout_len;

  if ((req_len = _metric_server_request_marshall(handle,
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
 * _metric_server_response_check
 *
 * Check that the version and error code are good prior to
 * unmarshalling
 *
 * Returns 0 on success, -1 on error
 */
static int
_metric_server_response_check(cerebro_t handle, 
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
  
  if (version != CEREBRO_METRIC_SERVER_PROTOCOL_VERSION)
    {
      handle->errnum = CEREBRO_ERR_VERSION_INCOMPATIBLE;
      return -1;
    }

  if (err_code != CEREBRO_METRIC_SERVER_PROTOCOL_ERR_SUCCESS)
    {
      handle->errnum = _metric_server_protocol_err_code_conversion(err_code);
      return -1;
    }
  
  return 0;
}

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
  u_int32_t *mtimePtr, *mtypePtr, *mlenPtr;
  int n, bufPtrlen, c = 0;
  char *bufPtr;

  if (!res || !buf || buflen < CEREBRO_METRIC_SERVER_RESPONSE_HEADER_LEN)
    {
      CEREBRO_DBG(("invalid pointers"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if ((n = unmarshall_int32(&(res->version), buf + c, buflen - c)) < 0)
    {
      CEREBRO_DBG(("unmarshall_int32"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += n;

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

  mtimePtr = &(res->metric_value_received_time);
  if ((n = unmarshall_u_int32(mtimePtr, buf + c,  buflen - c)) < 0)
    {
      CEREBRO_DBG(("unmarshall_u_int32"));
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
  unsigned int errnum;

  if (!metric_name || !hostname || !receive_response)
    {
      CEREBRO_DBG(("invalid parameters"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  
  if ((fd = low_timeout_connect(hostname,
                                port,
                                CEREBRO_METRIC_SERVER_PROTOCOL_CONNECT_TIMEOUT_LEN,
                                &errnum)) < 0)
    {
      handle->errnum = errnum;
      goto cleanup;
    }
  
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

int 
_cerebro_metric_get_data(cerebro_t handle,
                         void *list,
                         const char *metric_name,
                         Cerebro_metric_receive_response receive_response)
{
  unsigned int port;
  unsigned int timeout_len;
  unsigned int flags;

  if (_cerebro_handle_check(handle) < 0)
    return -1;

  if (!list || !metric_name || !receive_response)
    {
      CEREBRO_DBG(("invalid parameters"));
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
          port = handle->config_data.cerebro_port;
        }
      else
        port = CEREBRO_METRIC_SERVER_PORT;
    }
  else
    port = handle->port;

  if (!handle->timeout_len)
    {
      if (handle->config_data.cerebro_timeout_len_flag)
        {
          if (!handle->config_data.cerebro_timeout_len)
            {
              handle->errnum = CEREBRO_ERR_CONFIG_INPUT;
              return -1;
            }
          timeout_len = handle->config_data.cerebro_timeout_len;
        }
      else
        timeout_len = CEREBRO_METRIC_SERVER_TIMEOUT_LEN_DEFAULT;
    }
  else
    timeout_len = handle->timeout_len;

  if (!handle->flags)
    {
      if (handle->config_data.cerebro_flags_flag)
        {
          if (handle->config_data.cerebro_flags & ~CEREBRO_METRIC_FLAGS_MASK)
            {
              handle->errnum = CEREBRO_ERR_CONFIG_INPUT;
              return -1;
            }
          flags = handle->config_data.cerebro_flags;
        }
      flags = CEREBRO_METRIC_SERVER_FLAGS_DEFAULT;
    }
  else
    flags = handle->flags;

  if (!strlen(handle->hostname))
    {
      if (handle->config_data.cerebro_hostnames_flag)
        {
          int i, rv = -1;
          
          for (i = 0; i < handle->config_data.cerebro_hostnames_len; i++)
            {
              if ((rv = _get_metric_data(handle,
                                         list,
                                         metric_name,
                                         handle->config_data.cerebro_hostnames[i],
                                         port,
                                         timeout_len,
                                         flags,
                                         receive_response)) < 0)
                continue;
              break;
            }
          
          if (i >= handle->config_data.cerebro_hostnames_len)
            {
              handle->errnum = CEREBRO_ERR_CONNECT;
              return -1;
            }
          
          if (rv < 0)
            return -1;
        }
      else
        {
          if (_get_metric_data(handle,
                               list,
                               metric_name,
                               "localhost",
                               port,
                               timeout_len,
                               flags,
                               receive_response) < 0)
            return -1;
        }
    }
  else
    {
      if (_get_metric_data(handle,
                           list,
                           metric_name,
                           handle->hostname,
                           port,
                           timeout_len,
                           flags,
                           receive_response) < 0)
        return -1;
    }
  
  return 0;
}

