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
#include "cerebro/cerebro_metric_protocol.h"

#include "cerebro_api.h"
#include "cerebro_config_util.h"
#include "cerebro_metric_util.h"
#include "cerebro_util.h"

#include "debug.h"
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
      return CEREBRO_ERR_PROTOCOL;
    case CEREBRO_METRIC_PROTOCOL_ERR_INTERNAL_SYSTEM_ERROR:
      CEREBRO_DBG(("server internal system error"));
      return CEREBRO_ERR_INTERNAL;
    default:
      CEREBRO_DBG(("invalid protocol error code: %d", protocol_error));
      return CEREBRO_ERR_INTERNAL;
    }
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
  int n, len = 0;

  if (!buf || buflen < CEREBRO_METRIC_REQUEST_PACKET_LEN)
    {
      CEREBRO_DBG(("invalid parameters"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  memset(buf, '\0', buflen);

  if ((n = marshall_int32(req->version, buf + len, buflen - len)) <= 0)
    {
      CEREBRO_DBG(("marshall_int32"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  len += n;

  if ((n = marshall_buffer(req->metric_name,
                           sizeof(req->metric_name),
                           buf + len,
                           buflen - len)) <= 0)
    {
      CEREBRO_DBG(("marshall_buffer"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  len += n;

  if ((n = marshall_u_int32(req->timeout_len, buf + len, buflen - len)) <= 0)
    {
      CEREBRO_DBG(("marshall_u_int32"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  len += n;

  if ((n = marshall_u_int32(req->flags, buf + len, buflen - len)) <= 0)
    {
      CEREBRO_DBG(("marshall_u_int32"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  len += n;

  return len;
}

/*
 * _cerebro_metric_request_send
 *
 * Send the metric request
 *
 * Returns 0 on success, -1 on error
 */
static int
_cerebro_metric_request_send(cerebro_t handle,
                             int fd,
                             const char *metric_name,
                             unsigned int timeout_len,
                             int flags)
{
  struct cerebro_metric_request req;
  char buf[CEREBRO_MAX_PACKET_LEN];
  int req_len;

  if (!metric_name)
    {
      CEREBRO_DBG(("invalid parameters"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

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
      CEREBRO_DBG(("fd_write_n: %s", strerror(errno)));
      handle->errnum = CEREBRO_ERR_INTERNAL;
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
                 Cerebro_metric_response_receive response_receive)
{
  int fd = -1, rv = -1;
                                                                  
  if (!metric_name || !hostname || !response_receive)
    {
      CEREBRO_DBG(("invalid parameters"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  
  if ((fd = _cerebro_low_timeout_connect(handle,
                                         hostname,
                                         port,
                                         CEREBRO_METRIC_PROTOCOL_CONNECT_TIMEOUT_LEN)) < 0)
    goto cleanup;
  
  if (_cerebro_metric_request_send(handle,
                                   fd,
                                   metric_name,
                                   timeout_len,
                                   flags) < 0)
    goto cleanup;

  if (response_receive(handle, list, fd) < 0)
    goto cleanup;
  
  rv = 0;
 cleanup:
  close(fd);
  return rv;
}

int 
_cerebro_metric_connect_and_receive(cerebro_t handle,
                                    void *list,
                                    const char *metric_name,
                                    Cerebro_metric_response_receive response_receive)
{
  unsigned int port;
  unsigned int timeout_len;
  unsigned int flags;

  if (_cerebro_handle_check(handle) < 0)
    return -1;

  if (!list || !metric_name || !response_receive)
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
        timeout_len = CEREBRO_METRIC_TIMEOUT_LEN_DEFAULT;
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
      flags = CEREBRO_METRIC_FLAGS_DEFAULT;
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
                                         response_receive)) < 0)
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
                               response_receive) < 0)
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
                           response_receive) < 0)
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

  if (!buf || !buflen || !bytes_to_read || buflen < bytes_to_read)
    {
      CEREBRO_DBG(("invalid parameters"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  memset(buf, '\0', buflen);

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
          CEREBRO_DBG(("select: %s", strerror(errno)));
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
           * bytes_to_read is read.  Due to version incompatability or
           * error packets, we may want to read a smaller packet.
           */
          if ((n = read(fd, buf + bytes_read, bytes_to_read - bytes_read)) < 0)
            {
              CEREBRO_DBG(("read: %s", strerror(errno)));
              handle->errnum = CEREBRO_ERR_INTERNAL;
              goto cleanup;
            }

          /* Pipe closed */
          if (!n)
            {
              handle->errnum = CEREBRO_ERR_PROTOCOL;
              goto cleanup;
            }

          bytes_read += n;
        }
      else
        {
          CEREBRO_DBG(("num != 0 but fd not set"));
          handle->errnum = CEREBRO_ERR_INTERNAL;
          goto cleanup;
        }
    }

 out:
  return bytes_read;

 cleanup:
  return -1;
}
