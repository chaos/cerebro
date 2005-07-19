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
#include "cerebro_clusterlist_util.h"
#include "cerebro_nodelist_util.h"
#include "cerebro_util.h"
#include "cerebro/cerebro_metric_server_protocol.h"

#include "cerebro_metric_util.h"

#include "debug.h"
#include "fd.h"
#include "marshall.h"
#include "metric_util.h"
#include "network_util.h"

/* 
 * _metric_value_unmarshall
 *
 * Unmarshall contents of a metric server response
 *
 * Returns 0 on success, -1 on error
 */
static int
_metric_value_unmarshall(cerebro_t handle,
                         struct cerebro_metric_server_response *res,
                         void **metric_value,
                         const char *buf,
                         unsigned int buflen)
{
  int n, malloc_len = 0;
  void *mvalue = NULL;
  u_int32_t mtype, mlen;

#if CEREBRO_DEBUG
  if (!res 
      || res->metric_value_type == CEREBRO_METRIC_VALUE_TYPE_NONE 
      || !metric_value 
      || !buf)
    {
      CEREBRO_DBG(("invalid parameters"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
#endif /* CEREBRO_DEBUG */

  mtype = res->metric_value_type;
  mlen = res->metric_value_len;

  /* Special case for ending null character */
  if (mtype == CEREBRO_METRIC_VALUE_TYPE_STRING)
    malloc_len = buflen + 1;
  else
    malloc_len = buflen;

  if (!(mvalue = malloc(buflen)))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      return -1;
    }
  memset(mvalue, '\0', malloc_len);

  if (mtype == CEREBRO_METRIC_VALUE_TYPE_INT32)
    {
      if ((n = unmarshall_int32((int32_t *)mvalue, buf, buflen)) < 0)
        {
          CEREBRO_DBG(("unmarshall_int32"));
          goto cleanup;
        }
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_U_INT32)
    {
      if ((n = unmarshall_u_int32((u_int32_t *)mvalue, buf, buflen)) < 0)
        {
          CEREBRO_DBG(("unmarshall_u_int32"));
          goto cleanup;
        }
    }  
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_FLOAT)
    {
      if ((n = unmarshall_float((float *)mvalue, buf, buflen)) < 0)
        {
          CEREBRO_DBG(("unmarshall_float"));
          goto cleanup;
        }
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_DOUBLE)
    {
      if ((n = unmarshall_double((double *)mvalue, buf, buflen)) < 0)
        {
          CEREBRO_DBG(("unmarshall_double"));
          goto cleanup;
        }
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_STRING)
    {
      if ((n = unmarshall_buffer((char *)mvalue, mlen, buf, buflen)) < 0)
        {
          CEREBRO_DBG(("unmarshall_buffer"));
          goto cleanup;
        }
    }
  else
    {
      /* If an invalid param, should have been caught before here */
      CEREBRO_DBG(("invalid type %d", mtype));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  if (n != buflen)
    {
      CEREBRO_DBG(("received invalid metric value buflen"));
      handle->errnum = CEREBRO_ERR_PROTOCOL;
      goto cleanup;
    }

  *metric_value = mvalue;
  return buflen;

 cleanup:
  free(mvalue);
  return -1;
}

/* 
 * _receive_metric_value
 *
 * Receive the metric value
 *
 * Returns 0 on success, -1 on error
 */
static int
_receive_metric_value(cerebro_t handle, 
                      struct cerebro_metric_server_response *res,
                      int fd)
{
  char *vbuf = NULL;
  int vbytes_read, rv = -1;
  void *metric_value = NULL;
  unsigned int errnum;

  if (!res->metric_value_len)
    {
      CEREBRO_DBG(("invalid parameters"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  if (!(vbuf = malloc(res->metric_value_len)))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }
      
  if ((vbytes_read = receive_data(fd,
                                  res->metric_value_len,
                                  vbuf,
                                  res->metric_value_len,
                                  CEREBRO_METRIC_SERVER_PROTOCOL_CLIENT_TIMEOUT_LEN,
                                  &errnum)) < 0)
    {
      handle->errnum = errnum;
      goto cleanup;
    }
  
  if (vbytes_read != res->metric_value_len)
    {
      handle->errnum = CEREBRO_ERR_PROTOCOL;
      goto cleanup;
    }
  
  if (_metric_value_unmarshall(handle, 
                               res, 
                               &metric_value, 
                               vbuf, 
                               vbytes_read) < 0)
      goto cleanup;

  res->metric_value = metric_value;
  
  rv = 0;
 cleanup:
  free(vbuf);
  return rv;
}

/* 
 * _receive_metric_data_response
 *
 * Receive a metric server data response.
 * 
 * Returns 0 on success, -1 on error
 */
static int
_receive_metric_data_response(cerebro_t handle, 
                              void *list, 
                              struct cerebro_metric_server_response *res,
                              unsigned int bytes_read,
                              int fd)
{
  struct cerebro_nodelist *nodelist;
  char nodename_buf[CEREBRO_MAX_NODENAME_LEN+1];
  u_int32_t mtype, mlen;
  int rv = -1;

  res->metric_value = NULL;

  if (_cerebro_handle_check(handle) < 0)
    {
      CEREBRO_DBG(("handle invalid"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  if (!list || !res || fd <= 0)
    {
      CEREBRO_DBG(("invalid parameters"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  nodelist = (struct cerebro_nodelist *)list;
  if (nodelist->magic != CEREBRO_NODELIST_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("invalid parameters"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  mtype = res->metric_value_type;
  mlen = res->metric_value_len;
  if (check_metric_type_len(mtype, mlen) < 0)
    {
      handle->errnum = CEREBRO_ERR_PROTOCOL;
      goto cleanup;
    }

  if (mlen)
    {
      if (_receive_metric_value(handle, res, fd) < 0)
        goto cleanup;
    }

  /* Guarantee ending '\0' character */
  memset(nodename_buf, '\0', CEREBRO_MAX_NODENAME_LEN+1);
  memcpy(nodename_buf, res->name, CEREBRO_MAX_NODENAME_LEN);

  if (_cerebro_nodelist_append(nodelist, 
                               nodename_buf,
                               res->metric_value_type,
                               res->metric_value_len,
                               res->metric_value) < 0)
    goto cleanup_metric_value;
  
  rv = 0;
 cleanup_metric_value:
  free(res->metric_value);
 cleanup:
  return rv;
}

cerebro_nodelist_t 
cerebro_get_metric_data(cerebro_t handle, const char *metric_name)
{
  struct cerebro_nodelist *nodelist = NULL;

  if (_cerebro_handle_check(handle) < 0)
    goto cleanup;

  if (!metric_name || strlen(metric_name) > CEREBRO_MAX_METRIC_NAME_LEN)
    {
      handle->errnum = CEREBRO_ERR_PARAMETERS;
      goto cleanup;
    }
  
  if (_cerebro_load_clusterlist_module(handle) < 0)
    goto cleanup;
  
  if (!(nodelist = _cerebro_nodelist_create(handle, metric_name)))
    goto cleanup;
  
  if (_cerebro_metric_get_data(handle,
                               nodelist,
                               metric_name,
                               _receive_metric_data_response) < 0)
    goto cleanup;
  
  if (_cerebro_nodelist_sort(nodelist) < 0)
    goto cleanup;

  handle->errnum = CEREBRO_ERR_SUCCESS;
  return nodelist;
  
 cleanup:
  if (nodelist)
    (void)cerebro_nodelist_destroy(nodelist);
  return NULL;
}
