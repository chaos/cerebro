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
#include "cerebro/cerebro_metric_server_protocol.h"

#include "cerebro_metric_util.h"

#include "fd.h"
#include "debug.h"
#include "marshall.h"

/* 
 * _receive_metric_name_response
 *
 * Receive a metric server name response
 * 
 * Returns 0 on success, -1 on error
 */
static int
_receive_metric_name_response(cerebro_t handle,
                              void *list,
                              struct cerebro_metric_server_response *res,
                              unsigned int bytes_read,
                              int fd)
{
  char metric_name_buf[CEREBRO_MAX_METRIC_NAME_LEN+1];
  struct cerebro_metriclist *metriclist;

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

  metriclist = (struct cerebro_metriclist *)list;
  if (metriclist->magic != CEREBRO_METRICLIST_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("invalid parameters"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  if (bytes_read != CEREBRO_METRIC_SERVER_RESPONSE_HEADER_LEN)
    {
      handle->errnum = CEREBRO_ERR_PROTOCOL;
      goto cleanup;
    }
      
  /* Guarantee ending '\0' character */
  memset(metric_name_buf, '\0', CEREBRO_MAX_METRIC_NAME_LEN+1);
  memcpy(metric_name_buf, res->name, CEREBRO_MAX_METRIC_NAME_LEN);
  
  if (_cerebro_metriclist_append(metriclist, metric_name_buf) < 0)
    goto cleanup;

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

  if (_cerebro_metric_get_data(handle,
                               metriclist,
                               CEREBRO_METRIC_METRIC_NAMES,
                               _receive_metric_name_response) < 0)
    goto cleanup;
  
                                            
  
  handle->errnum = CEREBRO_ERR_SUCCESS;
  return metriclist;
  
 cleanup:
  if (metriclist)
    (void)cerebro_metriclist_destroy(metriclist);
  return NULL;
}
