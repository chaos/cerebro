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
#include "cerebro_api.h"
#include "cerebro_config_util.h"
#include "cerebro_util.h"

#include "cerebro/cerebro_event_protocol.h"

#include "debug.h"
#include "fd.h"
#include "marshall.h"
#include "network_util.h"

/*
 * _event_server_request_marshall
 *
 * Marshall contents of a event server request
 *
 * Returns length written to buffer on success, -1 on error
 */
static int
_event_server_request_marshall(cerebro_t handle,
                               struct cerebro_event_server_request *req,
                               char *buf,
                               unsigned int buflen)
{
  int n, bufPtrlen, c = 0;
  char *bufPtr;
  
  if (!buf || buflen < CEREBRO_EVENT_SERVER_REQUEST_PACKET_LEN)
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
  
  bufPtr = req->event_name;
  bufPtrlen = sizeof(req->event_name);
  if ((n = marshall_buffer(bufPtr, bufPtrlen, buf + c, buflen - c)) <= 0)
    {
      CEREBRO_DBG(("marshall_buffer"));
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
 * _event_server_request_send
 *
 * Send the event server request
 *
 * Returns 0 on success, -1 on error
 */
static int
_event_server_request_send(cerebro_t handle,
                           int fd,
                           const char *event_name,
                           unsigned int flags)
{
  struct cerebro_event_server_request req;
  char buf[CEREBRO_MAX_PACKET_LEN];
  int req_len;

  if (!event_name)
    {
      CEREBRO_DBG(("invalid parameters"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  req.version = CEREBRO_EVENT_SERVER_PROTOCOL_VERSION;
  strncpy(req.event_name, event_name, CEREBRO_MAX_EVENT_NAME_LEN);
  req.flags = flags;

  if ((req_len = _event_server_request_marshall(handle,
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
 * _event_server_protocol_err_code_conversion
 *
 * Convert event server protocol err codes to API err codes
 *
 * Returns proper err code
 */
static int
_event_server_protocol_err_code_conversion(u_int32_t err_code)
{
  switch(err_code)
    {
    case CEREBRO_EVENT_SERVER_PROTOCOL_ERR_SUCCESS:
      return CEREBRO_ERR_SUCCESS;
    case CEREBRO_EVENT_SERVER_PROTOCOL_ERR_VERSION_INVALID:
      return CEREBRO_ERR_VERSION_INCOMPATIBLE;
    case CEREBRO_EVENT_SERVER_PROTOCOL_ERR_PACKET_INVALID:
      return CEREBRO_ERR_PROTOCOL;
    case CEREBRO_EVENT_SERVER_PROTOCOL_ERR_EVENT_INVALID:
      return CEREBRO_ERR_EVENT_INVALID;
    case CEREBRO_EVENT_SERVER_PROTOCOL_ERR_PARAMETER_INVALID:
      return CEREBRO_ERR_PROTOCOL;
    case CEREBRO_EVENT_SERVER_PROTOCOL_ERR_INTERNAL_ERROR:
      CEREBRO_DBG(("server internal system error"));
      return CEREBRO_ERR_INTERNAL;
    default:
      CEREBRO_DBG(("invalid protocol error code: %d", err_code));
      return CEREBRO_ERR_INTERNAL;
    }
}

/*
 * _event_server_response_check
 *
 * Check that the version and error code are good prior to
 * unmarshalling
 *
 * Returns 0 on success, -1 on error
 */
static int
_event_server_response_check(cerebro_t handle,
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

  if (version != CEREBRO_EVENT_SERVER_PROTOCOL_VERSION)
    {
      handle->errnum = CEREBRO_ERR_VERSION_INCOMPATIBLE;
      return -1;
    }
  
  if (err_code != CEREBRO_EVENT_SERVER_PROTOCOL_ERR_SUCCESS)
    {
      handle->errnum = _event_server_protocol_err_code_conversion(err_code);
      return -1;
    }

  return 0;
}

/* 
 * _event_connection
 *
 * Make a connection given the parameters
 *
 * Returns fd on success, -1 on error
 */
static int
_event_connection(cerebro_t handle,
                  const char *event_name,
                  const char *hostname,
                  unsigned int port,
                  int flags)
{
  char buf[CEREBRO_MAX_PACKET_LEN];
  int bytes_read;
  unsigned int errnum;
  int fd = -1;

  if (!event_name
      || !hostname)
    {
      CEREBRO_DBG(("invalid parameters"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if ((fd = low_timeout_connect(hostname,
                                port,
                                CEREBRO_EVENT_SERVER_PROTOCOL_CONNECT_TIMEOUT_LEN,
                                &errnum)) < 0)
    {
      handle->errnum = errnum;
      goto cleanup;
    }

  if (_event_server_request_send(handle,
                                 fd,
                                 event_name,
                                 flags) < 0)
    goto cleanup;

  if ((bytes_read = receive_data(fd,
                                 CEREBRO_EVENT_SERVER_RESPONSE_LEN,
                                 buf,
                                 CEREBRO_MAX_PACKET_LEN,
                                 CEREBRO_EVENT_SERVER_PROTOCOL_CLIENT_TIMEOUT_LEN,
                                 &errnum)) < 0)
    {
      handle->errnum = errnum;
      goto cleanup;
    }

  if (bytes_read < CEREBRO_EVENT_SERVER_RESPONSE_LEN)
    {
      handle->errnum = CEREBRO_ERR_PROTOCOL;
      goto cleanup;
    }
  
  if (_event_server_response_check(handle, buf, bytes_read) < 0)
    goto cleanup;

  return fd;

 cleanup:
  close(fd);
  return -1;
}

/* 
 * _setup_event_connection
 *
 * Setup an event connection
 *
 * Returns fd on success, -1 on error
 */
static int
_setup_event_connection(cerebro_t handle, const char *event_name)
{
  int fd = -1;
  unsigned int port;
  unsigned int flags;

  if (!event_name)
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
              goto cleanup;
            }
          port = handle->config_data.cerebro_port;
        }
      else
        port = CEREBRO_EVENT_SERVER_PORT;
    }
  else
    port = handle->port;

  /* No flags available for events right now, so nothing to handle */
  flags = 0;

  if (!strlen(handle->hostname))
    {
      if (handle->config_data.cerebro_hostnames_flag)
        {
          int i;
          
          for (i = 0; i < handle->config_data.cerebro_hostnames_len; i++)
            {
              if ((fd = _event_connection(handle,
                                          event_name,
                                          handle->config_data.cerebro_hostnames[i],
                                          port,
                                          flags)) < 0)
                continue;
              break;
            }
          
          if (i >= handle->config_data.cerebro_hostnames_len)
            {
              handle->errnum = CEREBRO_ERR_CONNECT;
              goto cleanup;
            }
          
          if (fd < 0)
            goto cleanup;
        }
      else
        {
          if ((fd = _event_connection(handle,
                                      event_name,
                                      "localhost",
                                      port,
                                      flags)) < 0)
            goto cleanup;
        }
    }
  else
    {
      if ((fd = _event_connection(handle,
                                  event_name,
                                  handle->hostname,
                                  port,
                                  flags)) < 0)
        goto cleanup;
    }

  return fd;

 cleanup:
  if (fd >= 0)
    close(fd);
  return -1;
}

int
cerebro_event_register(cerebro_t handle, const char *event_name)
{
  int fd = -1, *fdPtr = NULL;

  if (_cerebro_handle_check(handle) < 0)
    goto cleanup;

  if (!event_name || strlen(event_name) > CEREBRO_MAX_EVENT_NAME_LEN)
    {
      handle->errnum = CEREBRO_ERR_PARAMETERS;
      goto cleanup;
    }
  
  if ((fd = _setup_event_connection(handle, event_name)) < 0)
    goto cleanup;

  if (!(fdPtr = (int *)malloc(sizeof(int))))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }
  *fdPtr = fd;

  if (!list_append(handle->event_fds, fdPtr))
    {
      CEREBRO_DBG(("list_append: %s", strerror(errno)));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  handle->errnum = CEREBRO_ERR_SUCCESS;
  return fd;
  
 cleanup:
  if (fd >= 0)
    close(fd);
  if (fdPtr)
    free(fdPtr);
  return -1;
}

/* 
 * _event_fd_find
 *
 * Callback for finding an fd in a list
 *
 * Returns 1 on match, 0 on no-match
 */
static int
_event_fd_find(void *x, void *key)
{
  int *fd1, *fd2;

  fd1 = (int *)x;
  fd2 = (int *)key;

  if (*fd1 == *fd2)
    return 1;
  return 0;
}

int 
cerebro_event_unregister(cerebro_t handle, int fd)
{
  if (_cerebro_handle_check(handle) < 0)
    return -1;

  if (fd < 0)
    {
      handle->errnum = CEREBRO_ERR_PARAMETERS;
      return -1;
    }

  if (!list_find_first(handle->event_fds, _event_fd_find, &fd))
    {
      handle->errnum = CEREBRO_ERR_PARAMETERS;
      return -1;
    }

  if (!list_delete_all(handle->event_fds, _event_fd_find, &fd))
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  close(fd);

  handle->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}

int
cerebro_event_parse(cerebro_t handle, 
                    int fd,
                    unsigned int *event_value_type,
                    unsigned int *event_value_len,
                    void **event_value)
{
  if (_cerebro_handle_check(handle) < 0)
    goto cleanup;

  if (fd < 0)
    {
      handle->errnum = CEREBRO_ERR_PARAMETERS;
      goto cleanup;
    }
  
  if (!list_find_first(handle->event_fds,
                       _event_fd_find,
                       &fd))
    {
      handle->errnum = CEREBRO_ERR_PARAMETERS;
      goto cleanup;
    }

  /* XXX - read and parse event */

  if (event_value_type)
    ;
  
  if (event_value_len)
    ;

  if (event_value)
    ;

#if 0
  if (_cerebro_event_get_data(handle,
                               nodelist,
                               event_name,
                               _receive_event_data_response) < 0)
    goto cleanup;
#endif

  handle->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
  
 cleanup:
  return -1;
}
