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
#include "cerebro_api.h"
#include "cerebro_clusterlist_util.h"
#include "cerebro_config_util.h"
#include "cerebro_nodelist_util.h"
#include "cerebro_util.h"
#include "cerebro/cerebro_error.h"
#include "cerebro/cerebro_metric_protocol.h"

#include "cerebro_metric_util.h"

#include "fd.h"
#include "marshall.h"

/* 
 * _cerebro_node_metric_response_header_unmarshall
 *
 * Unmarshall contents of a metric server response header
 *
 * Returns 0 on success, -1 on error
 */
static int
_cerebro_node_metric_response_header_unmarshall(cerebro_t handle,
                                                struct cerebro_node_metric_response *res,
                                                const char *buf,
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
#endif /* CEREBRO_DEBUG */

  if ((len = unmarshall_int32(&(res->version),
                              buf + count,
                              buflen - count)) < 0)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (!len)
    return count;

  count += len;

  if ((len = unmarshall_u_int32(&(res->metric_err_code),
                                buf + count,
                                buflen - count)) < 0)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (!len)
    return count;

  count += len;

  if ((len = unmarshall_u_int8(&(res->end_of_responses),
                               buf + count,
                               buflen - count)) < 0)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (!len)
    return count;

  count += len;

  if ((len = unmarshall_buffer(res->nodename,
                               sizeof(res->nodename),
                               buf + count,
                               buflen - count)) < 0)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (!len)
    return count;

  count += len;

  if ((len = unmarshall_u_int32(&(res->metric_value_type),
                                buf + count,
                                buflen - count)) < 0)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (!len)
    return count;
  
  count += len;

  if ((len = unmarshall_u_int32(&(res->metric_value_len),
                                buf + count,
                                buflen - count)) < 0)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (!len)
    return count;
  
  count += len;

  return count;
}

/* 
 * _cerebro_node_metric_response_metric_value_unmarshall
 *
 * Unmarshall contents of a metric server response
 *
 * Returns 0 on success, -1 on error
 */
static int
_cerebro_node_metric_response_metric_value_unmarshall(cerebro_t handle,
                                                      struct cerebro_node_metric_response *res,
                                                      const char *buf,
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
#endif /* CEREBRO_DEBUG */

  switch(res->metric_value_type)
    {
    case CEREBRO_METRIC_VALUE_TYPE_NONE:
      cerebro_err_debug("%s(%s:%d): type none despite data returned",
			__FILE__, __FUNCTION__, __LINE__);
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
      break;
    case CEREBRO_METRIC_VALUE_TYPE_INT32:
      if (buflen != sizeof(int32_t))
        {
          handle->errnum = CEREBRO_ERR_PROTOCOL;
          return -1;
        }

      if (!(res->metric_value = malloc(sizeof(int32_t))))
        {
          handle->errnum = CEREBRO_ERR_OUTMEM;
          return -1;
        }

      if ((len = unmarshall_int32((int32_t *)res->metric_value,
                                  buf,
                                  buflen)) < 0)
        {
          handle->errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
      break;
    case CEREBRO_METRIC_VALUE_TYPE_U_INT32:
      if (buflen != sizeof(u_int32_t))
        {
          handle->errnum = CEREBRO_ERR_PROTOCOL;
          return -1;
        }

      if (!(res->metric_value = malloc(sizeof(u_int32_t))))
        {
          handle->errnum = CEREBRO_ERR_OUTMEM;
          return -1;
        }

      if ((len = unmarshall_u_int32((u_int32_t *)res->metric_value,
                                    buf,
                                    buflen)) < 0)
        {
          handle->errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
      break;
    case CEREBRO_METRIC_VALUE_TYPE_FLOAT:
      if (buflen != sizeof(float))
        {
          handle->errnum = CEREBRO_ERR_PROTOCOL;
          return -1;
        }

      if (!(res->metric_value = malloc(sizeof(float))))
        {
          handle->errnum = CEREBRO_ERR_OUTMEM;
          return -1;
        }

      if ((len = unmarshall_float((float *)res->metric_value,
                                  buf,
                                  buflen)) < 0)
        {
          handle->errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
      break;
    case CEREBRO_METRIC_VALUE_TYPE_DOUBLE:
      if (buflen != sizeof(double))
        {
          handle->errnum = CEREBRO_ERR_PROTOCOL;
          return -1;
        }

      if (!(res->metric_value = malloc(sizeof(double))))
        {
          handle->errnum = CEREBRO_ERR_OUTMEM;
          return -1;
        }

      if ((len = unmarshall_double((double *)res->metric_value,
                                   buf,
                                   buflen)) < 0)
        {
          handle->errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
      break;
    case CEREBRO_METRIC_VALUE_TYPE_STRING:

      /* Watch for NUL termination */

      if (!(res->metric_value = malloc(res->metric_value_len + 1)));
        {
          handle->errnum = CEREBRO_ERR_OUTMEM;
          return -1;
        }
      memset(res->metric_value, '\0', res->metric_value_len + 1);

      if ((len = unmarshall_buffer((char *)res->metric_value,
                                   res->metric_value_len,
                                   buf,
                                   buflen)) < 0)
        {
          handle->errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
      break;

    default:
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    };

  if (!len)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  count += len;

  return count;
}

/* 
 * _cerebro_node_metric_response_receive_one
 *
 * Receive a single response
 *
 * Returns response packet and length of packet unmarshalled on
 * success, -1 on error
 */
static int
_cerebro_node_metric_response_receive_one(cerebro_t handle,
                                          int fd,
                                          struct cerebro_node_metric_response *res)
{
  int header_count = 0, metric_value_count = 0, bytes_read;
  char header_buf[CEREBRO_PACKET_BUFLEN];
  char *buf = NULL;
  
  if ((bytes_read = _cerebro_metric_receive_data(handle,
						 fd,
						 CEREBRO_NODE_METRIC_RESPONSE_HEADER_LEN,
						 header_buf,
						 CEREBRO_PACKET_BUFLEN)) < 0)
    goto cleanup;
  
  if (!bytes_read)
    return bytes_read;
  
  if ((header_count = _cerebro_node_metric_response_header_unmarshall(handle, 
                                                                      res, 
                                                                      header_buf, 
                                                                      bytes_read)) < 0)
    goto cleanup;

  if (!header_count)
    return header_count;

  if (res->metric_value_len)
    {
      int buflen = res->metric_value_len + 1;

      if (!(buf = malloc(buflen)))
        {
          handle->errnum = CEREBRO_ERR_OUTMEM;
          goto cleanup;
        }
      
      if (res->metric_value_type == CEREBRO_METRIC_VALUE_TYPE_NONE)
        {
          handle->errnum = CEREBRO_ERR_PROTOCOL;
          goto cleanup;
        }

      if ((bytes_read = _cerebro_metric_receive_data(handle,
						     fd,
						     res->metric_value_len,
						     buf,
						     buflen)) < 0)
        goto cleanup;
      
      if ((metric_value_count = _cerebro_node_metric_response_metric_value_unmarshall(handle,
                                                                                      res,
                                                                                      buf,
                                                                                      bytes_read)) < 0)
        goto cleanup;
    }
  else
    res->metric_value = NULL;

  free(buf);
  return header_count + metric_value_count;

 cleanup:
  free(buf);
  return -1;
}

/* 
 * _cerebro_node_metric_response_receive_all
 *
 * Receive all of the metric server responses.
 * 
 * Returns 0 on success, -1 on error
 */
static int
_cerebro_node_metric_response_receive_all(cerebro_t handle,
				     struct cerebro_nodelist *nodelist,
				     int fd,
				     int flags)
{
  struct cerebro_node_metric_response res;
  int res_len;

  /* XXX the cleanup here is disgusting */
  while (1)
    {
      memset(&res, '\0', sizeof(struct cerebro_node_metric_response));
      if ((res_len = _cerebro_node_metric_response_receive_one(handle,
                                                               fd, 
                                                               &res)) < 0)
        goto cleanup;
      
      if (res_len < CEREBRO_NODE_METRIC_RESPONSE_HEADER_LEN)
        {
          if (res_len == CEREBRO_METRIC_ERR_RESPONSE_LEN)
            {
              if (res.metric_err_code != CEREBRO_METRIC_PROTOCOL_ERR_SUCCESS)
                {
                  handle->errnum = _cerebro_metric_protocol_err_conversion(res.metric_err_code);
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
      if (res.metric_err_code != CEREBRO_METRIC_PROTOCOL_ERR_SUCCESS)
        {
          handle->errnum = _cerebro_metric_protocol_err_conversion(res.metric_err_code);
          goto cleanup;
        }

      if (res.end_of_responses == CEREBRO_METRIC_PROTOCOL_IS_LAST_RESPONSE)
        break;

      if (_cerebro_nodelist_append(nodelist, 
				   res.nodename,
                                   res.metric_value_type,
                                   res.metric_value_len,
                                   res.metric_value) < 0)
	goto cleanup;
    }
  
  if (_cerebro_nodelist_sort(nodelist) < 0)
    goto cleanup;

  return 0;

 cleanup:
  if (res.metric_value)
    free(res.metric_value);
  return -1;
}

/*  
 * _cerebro_metric_get_metric_value
 *
 * Get metric data and store it appropriately into the nodelist
 *
 * Returns 0 on success, -1 on error
 */
static int
_cerebro_metric_get_metric_data(cerebro_t handle,
                                cerebro_nodelist_t nodelist,
                                const char *metric_name,
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
				   metric_name,
				   timeout_len, 
				   flags) < 0)
    goto cleanup;

  if (_cerebro_node_metric_response_receive_all(handle, 
                                                nodelist, 
                                                fd, 
                                                flags) < 0)
    goto cleanup;

  rv = 0;
 cleanup:
  close(fd);
  return rv;
}

cerebro_nodelist_t 
cerebro_get_metric_data(cerebro_t handle, 
                        const char *metric_name)
{
  struct cerebro_nodelist *nodelist = NULL;
  unsigned int port;
  unsigned int timeout_len;
  unsigned int flags;

  if (_cerebro_handle_check(handle) < 0)
    goto cleanup;

  if (!metric_name || strlen(metric_name) > CEREBRO_METRIC_NAME_MAXLEN)
    {
      handle->errnum = CEREBRO_ERR_PARAMETERS;
      goto cleanup;
    }

  if (_cerebro_load_clusterlist_module(handle) < 0)
    goto cleanup;
  
  if (_cerebro_metric_config(handle, &port, &timeout_len, &flags) < 0)
    goto cleanup;

  if (!(nodelist = _cerebro_nodelist_create(handle, metric_name)))
    goto cleanup;

  if (!strlen(handle->hostname))
    {
      if (handle->config_data.cerebro_hostnames_flag)
	{
	  int i, rv = -1;

	  for (i = 0; i < handle->config_data.cerebro_hostnames_len; i++)
	    {
	      if ((rv = _cerebro_metric_get_metric_data(handle,
                                                        nodelist,
                                                        metric_name,
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
	  if (_cerebro_metric_get_metric_data(handle,
                                              nodelist,
                                              metric_name,
                                              "localhost",
                                              port,
                                              timeout_len,
                                              flags) < 0)
	    goto cleanup;
	}
    }
  else
    {
      if (_cerebro_metric_get_metric_data(handle,
                                          nodelist,
                                          metric_name,
                                          handle->hostname,
                                          port,
                                          timeout_len,
                                          flags) < 0)
	goto cleanup;
    }
  
  handle->errnum = CEREBRO_ERR_SUCCESS;
  return nodelist;
  
 cleanup:
  if (nodelist)
    (void)cerebro_nodelist_destroy(nodelist);
  return NULL;
}
