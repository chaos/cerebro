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
#include "cerebro_api.h"
#include "cerebro_metriclist_util.h"
#include "cerebro_util.h"
#include "cerebro/cerebro_metric_protocol.h"

#include "cerebro_metric_util.h"

#include "fd.h"
#include "marshall.h"
#include "debug.h"

/* 
 * _cerebro_metric_name_response_unmarshall
 *
 * Unmarshall contents of a metric server response header
 *
 * Returns 0 on success, -1 on error
 */
static int
_cerebro_metric_name_response_unmarshall(cerebro_t handle,
					 struct cerebro_metric_name_response *res,
					 const char *buf,
					 unsigned int buflen)
{
  int n, len = 0;

#if CEREBRO_DEBUG
  if (!res || !buf)
    {
      CEREBRO_DBG(("invalid pointers"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
#endif /* CEREBRO_DEBUG */

  if ((n = unmarshall_int32(&(res->version), buf + len, buflen - len)) < 0)
    {
      CEREBRO_DBG(("unmarshall_int32"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  
  if (!n)
    return len;
  len += n;

  if ((n = unmarshall_u_int32(&(res->err_code), buf + len, buflen - len)) < 0)
    {
      CEREBRO_DBG(("unmarshall_u_int32"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (!n)
    return len;
  len += n;

  if ((n = unmarshall_u_int8(&(res->end), buf + len, buflen - len)) < 0)
    {
      CEREBRO_DBG(("unmarshall_u_int8"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (!n)
    return len;
  len += n;

  if ((n = unmarshall_buffer(res->metric_name,
                             sizeof(res->metric_name),
                             buf + len,
                             buflen - len)) < 0)
    {
      CEREBRO_DBG(("unmarshall_buffer"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (!n)
    return len;
  len += n;

  return len;
}

/* 
 * _cerebro_metric_name_response_receive_one
 *
 * Receive a single response
 *
 * Returns response packet and length of packet unmarshalled on
 * success, -1 on error
 */
static int
_cerebro_metric_name_response_receive_one(cerebro_t handle,
                                          int fd,
                                          struct cerebro_metric_name_response *res)
{
  char buf[CEREBRO_MAX_PACKET_LEN];
  int bytes_read, count;
  
  if ((bytes_read = _cerebro_metric_receive_data(handle,
						 fd,
						 CEREBRO_METRIC_NAME_RESPONSE_LEN,
						 buf,
						 CEREBRO_MAX_PACKET_LEN)) < 0)
    goto cleanup;
  
  if (!bytes_read)
    return bytes_read;
  
  if ((count = _cerebro_metric_name_response_unmarshall(handle, 
							res, 
							buf, 
							bytes_read)) < 0)
    goto cleanup;

  return count;

 cleanup:
  return -1;
}

/* 
 * _cerebro_metric_name_response_receive_all
 *
 * Receive all of the metric server responses.
 * 
 * Returns 0 on success, -1 on error
 */
static int
_cerebro_metric_name_response_receive_all(cerebro_t handle,
                                          struct cerebro_metriclist *metriclist,
                                          int fd,
                                          int flags)
{
  struct cerebro_metric_name_response res;
  char metric_name_buf[CEREBRO_MAX_METRIC_NAME_LEN+1];
  int res_len;

  /* XXX the cleanup here is disgusting */
  while (1)
    {
      memset(&res, '\0', sizeof(struct cerebro_metric_name_response));
      if ((res_len = _cerebro_metric_name_response_receive_one(handle,
                                                               fd, 
                                                               &res)) < 0)
        goto cleanup;
      
      if (res_len < CEREBRO_METRIC_NAME_RESPONSE_LEN)
        {
          if (res_len == CEREBRO_METRIC_ERR_RESPONSE_LEN)
            {
              if (res.err_code != CEREBRO_METRIC_PROTOCOL_ERR_SUCCESS)
                {
                  handle->errnum = _cerebro_metric_protocol_err_conversion(res.err_code);
                  goto cleanup;
                }
            }

          handle->errnum = CEREBRO_ERR_PROTOCOL;
          goto cleanup;
        }

      /* XXX possible? */
      if (res.version != CEREBRO_METRIC_PROTOCOL_VERSION)
        {
          handle->errnum = CEREBRO_ERR_VERSION_INCOMPATIBLE;
          goto cleanup;
        }
      
      /* XXX possible? */
      if (res.err_code != CEREBRO_METRIC_PROTOCOL_ERR_SUCCESS)
        {
          handle->errnum = _cerebro_metric_protocol_err_conversion(res.err_code);
          goto cleanup;
        }

      if (res.end == CEREBRO_METRIC_PROTOCOL_IS_LAST_RESPONSE)
        break;

      /* Guarantee ending '\0' character */
      memset(metric_name_buf, '\0', CEREBRO_MAX_METRIC_NAME_LEN+1);
      memcpy(metric_name_buf, res.metric_name, CEREBRO_MAX_METRIC_NAME_LEN);

      if (_cerebro_metriclist_append(metriclist, res.metric_name) < 0)
	goto cleanup;
    }

  return 0;

 cleanup:
  return -1;
}

/*  
 * _cerebro_metric_get_metric_names
 *
 * Get metric names and store it appropriately into the metriclist
 *
 * Returns 0 on success, -1 on error
 */
static int
_cerebro_metric_get_metric_names(cerebro_t handle,
                                 cerebro_metriclist_t metriclist,
                                 const char *hostname,
                                 unsigned int port,
                                 unsigned int timeout_len,
                                 int flags)
{
  int fd = -1, rv = -1;

  if ((fd = _cerebro_low_timeout_connect(handle, 
					 hostname, 
					 port, 
					 CEREBRO_METRIC_PROTOCOL_CONNECT_TIMEOUT_LEN)) < 0)
    goto cleanup;

  if (_cerebro_metric_request_send(handle, 
				   fd, 
				   CEREBRO_METRIC_METRIC_NAMES,
				   timeout_len, 
				   flags) < 0)
    goto cleanup;

  if (_cerebro_metric_name_response_receive_all(handle, 
                                                metriclist, 
                                                fd, 
                                                flags) < 0)
    goto cleanup;

  rv = 0;
 cleanup:
  close(fd);
  return rv;
}

cerebro_metriclist_t 
cerebro_get_metric_names(cerebro_t handle)
{
  struct cerebro_metriclist *metriclist = NULL;
  unsigned int port;
  unsigned int timeout_len;
  unsigned int flags;

  if (_cerebro_handle_check(handle) < 0)
    goto cleanup;

  if (_cerebro_metric_config(handle, &port, &timeout_len, &flags) < 0)
    goto cleanup;
  
  if (!(metriclist = _cerebro_metriclist_create(handle)))
    goto cleanup;

  if (!strlen(handle->hostname))
    {
      if (handle->config_data.cerebro_hostnames_flag)
	{
	  int i, rv = -1;

	  for (i = 0; i < handle->config_data.cerebro_hostnames_len; i++)
	    {
	      if ((rv = _cerebro_metric_get_metric_names(handle,
                                                         metriclist,
                                                         handle->config_data.cerebro_hostnames[i],
                                                         port,
                                                         timeout_len,
                                                         flags)) < 0)
		continue;
	      break;
	    }
	  
          if (i >= handle->config_data.cerebro_hostnames_len)
            {
              handle->errnum = CEREBRO_ERR_CONNECT;
              goto cleanup;
            }

	  if (rv < 0)
            goto cleanup;
	}
      else
	{
	  if (_cerebro_metric_get_metric_names(handle,
                                               metriclist,
                                               "localhost",
                                               port,
                                               timeout_len,
                                               flags) < 0)
	    goto cleanup;
	}
    }
  else
    {
      if (_cerebro_metric_get_metric_names(handle,
                                           metriclist,
                                           handle->hostname,
                                           port,
                                           timeout_len,
                                           flags) < 0)
	goto cleanup;
    }
  
  handle->errnum = CEREBRO_ERR_SUCCESS;
  return metriclist;
  
 cleanup:
  if (metriclist)
    (void)cerebro_metriclist_destroy(metriclist);
  return NULL;
}
