/*****************************************************************************\
 *  $Id: cerebro_updown.c,v 1.6 2005-04-27 00:04:23 achu Exp $
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

#include "cerebro.h"
#include "cerebro_api.h"
#include "cerebro_util.h"
#include "cerebro_updown.h"
#include "cerebro_updown_protocol.h"
#include "cerebro_marshalling.h"
#include "hostlist.h"
#include "fd.h"

#define CEREBRO_UPDOWN_MAGIC_NUMBER             0xF00F5678
#define CEREBRO_UPDOWN_UP_NODES_LOADED          0x00000001
#define CEREBRO_UPDOWN_DOWN_NODES_LOADED        0x00000002
#define CEREBRO_UPDOWN_UP_AND_DOWN_NODES_LOADED 0x00000003
/* 
 * cerebro_updown_data
 *
 * cerebro update state date stored in cerebro handle's updown_data
 * pointer.
 */
struct cerebro_updown_data {
  int32_t magic;
  int32_t loaded_nodes;
  hostlist_t up_nodes;
  hostlist_t down_nodes;
};

/* 
 * _cerebro_updown_data_check
 *
 * Check for a proper updown_data handle, setting the errnum
 * appropriately if an error is found.
 *
 * Returns 0 on success, -1 on error
 */
static int
_cerebro_updown_data_check(cerebro_t handle, 
                           struct cerebro_updown_data *updown_data)
{
#ifndef NDEBUG
  if (!updown_data)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
#endif /* NDEBUG */

  if (updown_data->magic != CEREBRO_UPDOWN_MAGIC_NUMBER)
    {
      handle->errnum = CEREBRO_ERR_MAGIC_NUMBER;
      return -1;
    }

#ifndef NDEBUG
  if (!updown_data->up_nodes || !updown_data->down_nodes)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
#endif /* NDEBUG */

  return 0;
}

/* 
 * _cerebro_updown_err_conversion
 *
 * Convert updown protocol err codes to API err codes
 *
 * Returns proper err code
 */
static int
_cerebro_updown_err_conversion(int protocol_error)
{
  switch(protocol_error)
    {
    case CEREBRO_UPDOWN_PROTOCOL_ERR_SUCCESS:
      return CEREBRO_ERR_SUCCESS;
    case CEREBRO_UPDOWN_PROTOCOL_ERR_VERSION_INVALID:
      return CEREBRO_ERR_VERSION_INCOMPATIBLE;
    case CEREBRO_UPDOWN_PROTOCOL_ERR_UPDOWN_REQUEST_INVALID:
    case CEREBRO_UPDOWN_PROTOCOL_ERR_TIMEOUT_INVALID:
    case CEREBRO_UPDOWN_PROTOCOL_ERR_INTERNAL_SYSTEM_ERROR:
    default:
      return CEREBRO_ERR_INTERNAL;
    }
}

/* 
 * _cerebro_updown_data_loaded_check
 *
 * Checks if the handle contains properly loaded updown_data data.
 * 
 * Returns 0 on success, -1 on error
 */
static int
_cerebro_updown_data_loaded_check(cerebro_t handle)
{
  struct cerebro_updown_data *updown_data;

  if (cerebro_handle_check(handle) < 0)
    return -1;

  if (!(handle->loaded_state & CEREBRO_UPDOWN_DATA_LOADED))
    {
      handle->errnum = CEREBRO_ERR_NOT_LOADED;
      return -1;
    }

  updown_data = (struct cerebro_updown_data *)handle->updown_data;
  if (_cerebro_updown_data_check(handle, updown_data) < 0)
    return -1;

  return 0;
}

/* 
 * _cerebro_updown_init_request
 *
 * Initialize an updown request
 *
 * Returns 0 on success, -1 on error
 */
static int
_cerebro_updown_init_request(cerebro_t handle,
                             struct cerebro_updown_request *req,
                             unsigned int timeout_len,
                             int flags)
{
  u_int32_t updown_request;

  if (flags == CEREBRO_UPDOWN_UP_NODES)
    updown_request = CEREBRO_UPDOWN_PROTOCOL_REQUEST_UP_NODES;
  else if (flags == CEREBRO_UPDOWN_DOWN_NODES)
    updown_request = CEREBRO_UPDOWN_PROTOCOL_REQUEST_DOWN_NODES;
  else 
    updown_request = CEREBRO_UPDOWN_PROTOCOL_REQUEST_UP_AND_DOWN_NODES;
    
  req->version = CEREBRO_UPDOWN_PROTOCOL_VERSION;
  req->updown_request = updown_request;
  req->timeout_len = timeout_len;

  return 0;
}

/* 
 * _cerebro_updown_request_marshall
 *
 * Marshall contents of a updown server request
 *
 * Returns length written to buffer on success, -1 on error
 */
static int
_cerebro_updown_request_marshall(cerebro_t handle,
                                 struct cerebro_updown_request *req,
                                 char *buffer,
                                 int bufferlen)
{
  int ret, c = 0;

#ifndef NDEBUG
  if (!buffer || bufferlen < CEREBRO_UPDOWN_REQUEST_LEN)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
#endif /* NDEBUG */

  memset(buffer, '\0', bufferlen);

  if ((ret = cerebro_marshall_int32(req->version,
                                    buffer + c, 
                                    bufferlen - c)) < 0)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += ret;

  if ((ret = cerebro_marshall_uint32(req->updown_request,
                                     buffer + c, 
                                     bufferlen - c)) < 0)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += ret;

  if ((ret = cerebro_marshall_uint32(req->timeout_len,
                                     buffer + c, 
                                     bufferlen - c)) < 0)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += ret;

  return c;
}

/* 
 * _cerebro_updown_response_unmarshall
 *
 * Marshall contents of a updown server request
 *
 * Returns 0 on success, -1 on error
 */
static int
_cerebro_updown_response_unmarshall(cerebro_t handle,
                                    struct cerebro_updown_response *res,
                                    char *buffer,
                                    int bufferlen)
{
  int ret, c = 0;
  int invalid_size = 0;

#ifndef NDEBUG
  if (!buffer)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
#endif /* NDEBUG */

  if (bufferlen != CEREBRO_UPDOWN_RESPONSE_LEN)
    invalid_size++;

  if (invalid_size && bufferlen < sizeof(res->version))
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if ((ret = cerebro_unmarshall_int32(&(res->version),
                                      buffer + c,
                                      bufferlen - c)) < 0)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += ret;

  if (invalid_size)
    {
      /* Invalid version to be handled by later code */
      if (res->version != CEREBRO_UPDOWN_PROTOCOL_VERSION)
        return 0;
    }

  if (invalid_size && ((bufferlen - c) < sizeof(res->updown_err_code)))
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if ((ret = cerebro_unmarshall_uint32(&(res->updown_err_code),
                                       buffer + c,
                                       bufferlen - c)) < 0)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += ret;

  if (invalid_size)
    {
      /* Error code to be handled by later code */
      if (res->updown_err_code != CEREBRO_UPDOWN_PROTOCOL_ERR_SUCCESS)
        return 0;

      /* Ok, we really have to give up now, the other end didn't give
       * us anything to work with 
       */
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if ((ret = cerebro_unmarshall_uint8(&(res->end_of_responses),
                                      buffer + c,
                                      bufferlen - c)) < 0)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += ret;

  if ((ret = cerebro_unmarshall_buffer(res->nodename,
                                       sizeof(res->nodename),
                                       buffer + c,
                                       bufferlen - c)) < 0)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += ret;

  if ((ret = cerebro_unmarshall_uint8(&(res->updown_state),
                                      buffer + c,
                                      bufferlen - c)) < 0)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += ret;

  return 0;
}

/* 
 * _cerebro_updown_send_request
 *
 * Send the updown request
 *
 * Returns 0 on success, -1 on error
 */
static int
_cerebro_updown_send_request(cerebro_t handle,
                             int fd,
                             unsigned int timeout_len,
                             int flags)
{
  struct cerebro_updown_request req;
  char buffer[CEREBRO_PACKET_BUFLEN];
  int req_len;

  if (_cerebro_updown_init_request(handle, &req, timeout_len, flags) < 0)
    return -1;
  
  if ((req_len = _cerebro_updown_request_marshall(handle,
                                                  &req,
                                                  buffer,
                                                  CEREBRO_PACKET_BUFLEN)) < 0)
    return -1;

  if (fd_write_n(fd, buffer, req_len) < 0)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  return 0;
}

/* 
 * _cerebro_updown_receive_a_response
 *
 * Receive a single response
 *
 * Returns 0 on success, -1 on error
 */
static int
_cerebro_updown_receive_a_response(cerebro_t handle,
                                   int fd,
                                   struct cerebro_updown_response *res)
{
  int rv, bytes_read = 0;
  char buffer[CEREBRO_PACKET_BUFLEN];

  memset(buffer, '\0', CEREBRO_PACKET_BUFLEN);
  while (bytes_read < CEREBRO_UPDOWN_RESPONSE_LEN)
    {
      fd_set rfds;
      struct timeval tv;
      tv.tv_sec = CEREBRO_UPDOWN_PROTOCOL_CONNECT_TIMEOUT_LEN;
      tv.tv_usec = 0;

      FD_ZERO(&rfds);
      FD_SET(fd, &rfds);

      if ((rv = select(fd + 1, &rfds, NULL, NULL, &tv)) < 0)
        {
          handle->errnum = CEREBRO_ERR_INTERNAL;
          goto cleanup;
        }

      if (!rv)
        {
          /* Timed out.  If atleast some bytes were read, unmarshall
           * the received bytes.  Its possible we are expecting more
           * bytes than the client is sending, perhaps because we are
           * using a different protocol version.  This will allow the
           * server to return a invalid version number back to the
           * user.
           */
          if (!bytes_read) 
            {
              handle->errnum = CEREBRO_ERR_CONNECT_TIMEOUT;
              goto cleanup;
            }
          else
            goto unmarshall_received;
        }

      if (FD_ISSET(fd, &rfds))
        {
          int n;

          if ((n = fd_read_n(fd,
                             buffer + bytes_read,
                             CEREBRO_UPDOWN_REQUEST_LEN - bytes_read)) < 0)
            {
              handle->errnum = CEREBRO_ERR_INTERNAL;
              goto cleanup;
            }

          if (!n)
            {
              /* Pipe closed */
              handle->errnum = CEREBRO_ERR_INTERNAL;
              goto cleanup;
            }

          bytes_read += n;
        }
      else
        {
          handle->errnum = CEREBRO_ERR_INTERNAL;
          goto cleanup;
        }
    }

 unmarshall_received:
  if (_cerebro_updown_response_unmarshall(handle, res, buffer, bytes_read) < 0)
    goto cleanup;

  return 0;

 cleanup:
  return -1;
}

/* 
 * _cerebro_updown_receive_responses
 *
 * Receive all of the updown server responses.
 * 
 * Returns 0 on success, -1 on error
 */
static int
_cerebro_updown_receive_responses(cerebro_t handle,
                                  int fd,
                                  struct cerebro_updown_data *updown_data)
{
  if (_cerebro_updown_receive_a_response(handle,
                                         fd, 
                                         &res) < 0)
    goto cleanup;


  return 0;
 cleanup:
  return -1;
}

/*  
 * _cerebro_updown_get_updown_data
 *
 * Get updown data and store it appropriately into the updown_data
 * structure.
 *
 * Returns 0 on success, -1 on error
 */
static int
_cerebro_updown_get_updown_data(cerebro_t handle,
                                struct cerebro_updown_data *updown_data,
                                const char *hostname,
                                unsigned int port,
                                unsigned int timeout_len,
                                int flags)
{
  int fd = -1, ret = -1;

  if ((fd = cerebro_low_timeout_connect(handle, 
                                        hostname, 
                                        port, 
                                        CEREBRO_UPDOWN_PROTOCOL_CONNECT_TIMEOUT_LEN)) < 0)
    goto cleanup;

  if (_cerebro_updown_send_request(handle, fd, timeout_len, flags) < 0)
    goto cleanup;

  if (_cerebro_updown_receive_responses(handle, fd, updown_data) < 0)
    goto cleanup;
  
  ret = 0;

 cleanup:
  close(fd);
  return ret;
}

int 
cerebro_updown_load_data(cerebro_t handle, 
                         const char *hostname, 
                         unsigned int port, 
                         unsigned int timeout_len,
                         int flags)
{
  struct cerebro_updown_data *updown_data = NULL;

  if (cerebro_handle_check(handle) < 0)
    goto cleanup;

  if (flags 
      && flags != CEREBRO_UPDOWN_UP_NODES
      && flags != CEREBRO_UPDOWN_DOWN_NODES
      && flags != CEREBRO_UPDOWN_UP_AND_DOWN_NODES)
    {
      handle->errnum = CEREBRO_ERR_PARAMETERS;
      goto cleanup;
    }

  if (!hostname)
    hostname = "localhost";

  if (!port)
    port = CEREBRO_UPDOWN_SERVER_PORT;

  if (!timeout_len)
    timeout_len = CEREBRO_UPDOWN_TIMEOUT_LEN_DEFAULT;

  if (!flags)
    flags = CEREBRO_UPDOWN_UP_AND_DOWN_NODES;
    
  if (!(updown_data = (struct cerebro_updown_data *)malloc(sizeof(struct cerebro_updown_data))))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }
  memset(updown_data, '\0', sizeof(struct cerebro_updown_data));
  updown_data->magic = CEREBRO_UPDOWN_MAGIC_NUMBER;
  updown_data->loaded_nodes = 0;

  if (!(updown_data->up_nodes = hostlist_create(NULL)))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  if (!(updown_data->down_nodes = hostlist_create(NULL)))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  if (_cerebro_updown_get_updown_data(handle,
                                      updown_data,
                                      hostname,
                                      port,
                                      timeout_len,
                                      flags) < 0)
    goto cleanup;

  if (handle->loaded_state & CEREBRO_UPDOWN_DATA_LOADED)
    {
      struct cerebro_updown_data *updown_data_temp;

      if (_cerebro_updown_data_check(handle, updown_data_temp) < 0)
        goto cleanup;

      updown_data_temp = handle->updown_data;

      if (flags == CEREBRO_UPDOWN_UP_AND_DOWN_NODES)
        {
          hostlist_destroy(updown_data_temp->up_nodes);
          hostlist_destroy(updown_data_temp->down_nodes);
        }
      else if (flags == CEREBRO_UPDOWN_UP_NODES)
        {
          if (updown_data_temp->loaded_nodes & CEREBRO_UPDOWN_DOWN_NODES_LOADED)
            {
              hostlist_destroy(updown_data->down_nodes);
              updown_data->down_nodes = updown_data_temp->down_nodes;
              updown_data->loaded_nodes |= CEREBRO_UPDOWN_DOWN_NODES_LOADED;
            }
          hostlist_destroy(updown_data_temp->up_nodes);
        }
      else
        {
          if (updown_data_temp->loaded_nodes & CEREBRO_UPDOWN_UP_NODES_LOADED)
            {
              hostlist_destroy(updown_data->up_nodes);
              updown_data->up_nodes = updown_data_temp->up_nodes;
              updown_data->loaded_nodes |= CEREBRO_UPDOWN_UP_NODES_LOADED;
            }

          hostlist_destroy(updown_data_temp->down_nodes);
        }
      free(updown_data_temp);
      handle->loaded_state &= ~CEREBRO_UPDOWN_DATA_LOADED;
      handle->updown_data = NULL;
    }

  handle->loaded_state |= CEREBRO_UPDOWN_DATA_LOADED;
  handle->updown_data = updown_data;
  handle->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
  
 cleanup:
  if (updown_data)
    {
      if (updown_data->up_nodes)
        {
          hostlist_destroy(updown_data->up_nodes);
          updown_data->up_nodes = NULL;
        }
      if (updown_data->down_nodes)
        {
          hostlist_destroy(updown_data->down_nodes);
          updown_data->down_nodes = NULL;
        }
      free(updown_data);
    }
  return -1;
}

int
cerebro_updown_unload_data(cerebro_t handle)
{
  if (cerebro_handle_check(handle) < 0)
    return -1;

  if (handle->loaded_state & CEREBRO_UPDOWN_DATA_LOADED)
    {
      struct cerebro_updown_data *updown_data;

      updown_data = (struct cerebro_updown_data *)handle->updown_data;

      if (_cerebro_updown_data_check(handle, updown_data) < 0)
        return -1;

      hostlist_destroy(updown_data->up_nodes);
      updown_data->up_nodes = NULL;
      hostlist_destroy(updown_data->down_nodes);
      updown_data->down_nodes = NULL;
      updown_data->magic = ~CEREBRO_UPDOWN_MAGIC_NUMBER;
      free(updown_data);
    }

  handle->loaded_state &= ~CEREBRO_UPDOWN_DATA_LOADED;
  handle->updown_data = NULL;

  handle->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}

int 
cerebro_updown_get_up_nodes(cerebro_t handle, char *buf, unsigned int buflen)
{
  if (_cerebro_updown_data_loaded_check(handle) < 0)
    return -1;

  handle->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}
 
int 
cerebro_updown_get_down_nodes(cerebro_t handle, char *buf, unsigned int buflen)
{
  if (_cerebro_updown_data_loaded_check(handle) < 0)
    return -1;

  handle->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}
 
int 
cerebro_updown_is_node_up(cerebro_t handle, const char *node)
{
  if (_cerebro_updown_data_loaded_check(handle) < 0)
    return -1;

  handle->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}
 
int 
cerebro_updown_is_node_down(cerebro_t handle, const char *node)
{
  if (_cerebro_updown_data_loaded_check(handle) < 0)
    return -1;

  handle->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}
 
int 
cerebro_updown_up_count(cerebro_t handle)
{
  if (_cerebro_updown_data_loaded_check(handle) < 0)
    return -1;

  handle->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}

int 
cerebro_updown_down_count(cerebro_t handle)
{
  if (_cerebro_updown_data_loaded_check(handle) < 0)
    return -1;

  handle->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}
