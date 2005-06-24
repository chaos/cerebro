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
#include <sys/select.h>
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else  /* !TIME_WITH_SYS_TIME */
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else /* !HAVE_SYS_TIME_H */
#  include <time.h>
# endif /* !HAVE_SYS_TIME_H */
#endif /* !TIME_WITH_SYS_TIME */
#include <sys/types.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <errno.h>

#include "cerebro.h"
#include "cerebro/cerebro_error.h"
#include "cerebro/cerebro_metric_protocol.h"

#include "cerebro_api.h"
#include "cerebro_config_util.h"
#include "cerebro_util.h"

#include "fd.h"
#include "marshall.h"

int
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
  char buf[CEREBRO_MAX_PACKET_LEN];
  int req_len;

  req.version = CEREBRO_METRIC_PROTOCOL_VERSION;
  strncpy(req.metric_name, metric_name, CEREBRO_MAX_METRIC_NAME_LEN);
  req.flags = flags;
  req.timeout_len = timeout_len;

  if ((req_len = _cerebro_metric_request_marshall(handle,
                                                  &req,
                                                  buf,
                                                  CEREBRO_MAX_PACKET_LEN)) < 0)
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

int
_cerebro_metric_receive_data(cerebro_t handle,
                             int fd,
                             unsigned int bytes_to_read,
                             char *buf,
                             unsigned int buflen)
{
  int bytes_read = 0;

  memset(buf, '\0', buflen);

  if (!bytes_to_read)
    {
      cerebro_err_debug("%s(%s:%d): invalid bytes_to_read",
                        __FILE__, __FUNCTION__, __LINE__);
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  if (buflen < bytes_to_read)
    {
      cerebro_err_debug("%s(%s:%d): invalid buflen: "
                        "bytes_to_read = %d buflen = %d",
                        __FILE__, __FUNCTION__, __LINE__,
                        bytes_to_read, buflen);
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  while (bytes_read < bytes_to_read)
    {
      fd_set rfds;
      struct timeval tv;
      int num;

      tv.tv_sec = CEREBRO_METRIC_PROTOCOL_CLIENT_TIMEOUT_LEN;
      tv.tv_usec = 0;

      FD_ZERO(&rfds);
      FD_SET(fd, &rfds);

      if ((num = select(fd + 1, &rfds, NULL, NULL, &tv)) < 0)
        {
          cerebro_err_debug("%s(%s:%d): select: %s",
                            __FILE__, __FUNCTION__, __LINE__,
                            strerror(errno));
          handle->errnum = CEREBRO_ERR_INTERNAL;
          goto cleanup;
        }

      if (!num)
        {
          /* Timed out.  If atleast some bytes were read, return data
           * back to the caller to let them possibly unmarshall the
           * data.  Its possible we are expecting more bytes than the
           * client is sending, perhaps because we are using a
           * different protocol version.  This will allow the server
           * to return a invalid version number back to the user.
           */
          if (!bytes_read)
            {
              handle->errnum = CEREBRO_ERR_PROTOCOL_TIMEOUT;
              goto cleanup;
            }
          else
            goto out;
        }

      if (FD_ISSET(fd, &rfds))
        {
          int n;

          /* Don't use fd_read_n b/c it loops until exactly
           * CEREBRO_METRIC_NAME_RESPONSE_PACKET_LEN is read.  Due to version
           * incompatability or error packets, we may want to read a
           * smaller packet.
           */
          if ((n = read(fd,
                        buf + bytes_read,
                        bytes_to_read - bytes_read)) < 0)
            {
              cerebro_err_debug("%s(%s:%d): read: %s",
                                __FILE__, __FUNCTION__, __LINE__,
                                strerror(errno));
              handle->errnum = CEREBRO_ERR_INTERNAL;
              goto cleanup;
            }

          if (!n)
            {
              /* Pipe closed */
              handle->errnum = CEREBRO_ERR_PROTOCOL;
              goto cleanup;
            }

          bytes_read += n;
        }
      else
        {
          cerebro_err_debug("%s(%s:%d): select returned bad data",
                            __FILE__, __FUNCTION__, __LINE__);
          handle->errnum = CEREBRO_ERR_INTERNAL;
          goto cleanup;
        }
    }

 out:
  return bytes_read;

 cleanup:
  return -1;
}
