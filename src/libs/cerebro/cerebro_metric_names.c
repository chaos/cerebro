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
#include "debug.h"
#include "marshall.h"

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

  if (!res || !buf)
    {
      CEREBRO_DBG(("invalid pointers"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

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
 * _metric_response_check
 *
 * Check that the version and error code are good prior to unmarshalling
 *
 * Returns 0 on success, -1 on error
 */
static int
_metric_response_check(cerebro_t handle,
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
  
  if (version != CEREBRO_METRIC_PROTOCOL_VERSION)
    {
      handle->errnum = CEREBRO_ERR_VERSION_INCOMPATIBLE;
      return -1;
    }

  if (err_code != CEREBRO_METRIC_PROTOCOL_ERR_SUCCESS)
    {
      handle->errnum = _cerebro_metric_protocol_err_conversion(err_code);
      return -1;
    }
  
  return 0;
}

/* 
 * _receive_metric_name_responses
 *
 * Receive all of the metric server responses.
 * 
 * Returns 0 on success, -1 on error
 */
static int
_receive_metric_name_responses(cerebro_t handle, void *list, int fd)
{
  struct cerebro_metriclist *metriclist;

  if (_cerebro_handle_check(handle) < 0)
    {
      CEREBRO_DBG(("handle invalid"));
      goto cleanup;
    }

  if (!list || fd <= 0)
    {
      CEREBRO_DBG(("invalid parameters"));
      goto cleanup;
    }

  metriclist = (struct cerebro_metriclist *)list;
  if (metriclist->magic != CEREBRO_METRICLIST_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("invalid parameters"));
      goto cleanup;
    }

  while (1)
    {
      struct cerebro_metric_name_response res;
      char metric_name_buf[CEREBRO_MAX_METRIC_NAME_LEN+1]; 
      char buf[CEREBRO_MAX_PACKET_LEN];
      int bytes_read, res_len;
  
      if ((bytes_read = _cerebro_metric_receive_data(handle,
                                                     fd,
                                                     CEREBRO_METRIC_NAME_RESPONSE_LEN,
                                                     buf,
                                                     CEREBRO_MAX_PACKET_LEN)) < 0)
        goto cleanup;
  
      if (bytes_read < CEREBRO_METRIC_ERR_RESPONSE_LEN)
        {
          handle->errnum = CEREBRO_ERR_PROTOCOL;
          goto cleanup;
        }
  
      if (_metric_response_check(handle, buf, bytes_read) < 0)
        goto cleanup;
      
      memset(&res, '\0', sizeof(struct cerebro_metric_name_response));
      if ((res_len = _cerebro_metric_name_response_unmarshall(handle, 
                                                              &res, 
                                                              buf, 
                                                              bytes_read)) < 0)
        goto cleanup;
      
      if (res_len < CEREBRO_METRIC_NAME_RESPONSE_LEN)
        {
          handle->errnum = CEREBRO_ERR_PROTOCOL;
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

cerebro_metriclist_t 
cerebro_get_metric_names(cerebro_t handle)
{
  struct cerebro_metriclist *metriclist = NULL;

  if (_cerebro_handle_check(handle) < 0)
    goto cleanup;

  if (!(metriclist = _cerebro_metriclist_create(handle)))
    goto cleanup;

  if (_cerebro_metric_connect_and_receive(handle,
                                          metriclist,
                                          CEREBRO_METRIC_METRIC_NAMES,
                                          _receive_metric_name_responses) < 0)
    goto cleanup;
  
                                            
  
  handle->errnum = CEREBRO_ERR_SUCCESS;
  return metriclist;
  
 cleanup:
  if (metriclist)
    (void)cerebro_metriclist_destroy(metriclist);
  return NULL;
}
