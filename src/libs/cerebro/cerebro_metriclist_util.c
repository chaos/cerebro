/*****************************************************************************\
 *  $Id: cerebro_metriclist_util.c,v 1.2 2005-06-23 23:00:48 achu Exp $
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
#include "cerebro/cerebro_error.h"

#include "list.h"

int
_cerebro_metriclist_check(cerebro_metriclist_t metriclist)
{
  if (!metriclist || metriclist->magic != CEREBRO_METRICLIST_MAGIC_NUMBER)
    return -1;

  if (!metriclist->metric_names)
    {
      cerebro_err_debug("%s(%s:%d): metriclist null",
			__FILE__, __FUNCTION__, __LINE__);
      metriclist->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (!metriclist->iterators)
    {
      cerebro_err_debug("%s(%s:%d): iterators null",
			__FILE__, __FUNCTION__, __LINE__);
      metriclist->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (!metriclist->handle)
    {
      cerebro_err_debug("%s(%s:%d): handle null",
			__FILE__, __FUNCTION__, __LINE__);
      metriclist->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (metriclist->handle->magic != CEREBRO_MAGIC_NUMBER)
    {
      cerebro_err_debug("%s(%s:%d): handle destroyed",
			__FILE__, __FUNCTION__, __LINE__);
      metriclist->errnum = CEREBRO_ERR_MAGIC_NUMBER;
      return -1;
    }

  return 0;
}

cerebro_metriclist_t 
_cerebro_metriclist_create(cerebro_t handle)
{
  cerebro_metriclist_t metriclist = NULL;

  if (!(metriclist = (cerebro_metriclist_t)malloc(sizeof(struct cerebro_metriclist))))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  memset(metriclist, '\0', sizeof(struct cerebro_metriclist));
  metriclist->magic = CEREBRO_METRICLIST_MAGIC_NUMBER;

  if (!(metriclist->metric_names = list_create((ListDelF)free)))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  /* No delete function, list_destroy() on 'metric_names' will take care of
   * deleting iterators.
   */
  if (!(metriclist->iterators = list_create(NULL)))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  if (!list_append(handle->metriclists, metriclist))
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }
  
  metriclist->handle = handle;

  return metriclist;
      
 cleanup:
  if (metriclist)
    {
      if (metriclist->metric_names)
        list_destroy(metriclist->metric_names);
      if (metriclist->iterators)
        list_destroy(metriclist->iterators);
      free(metriclist);
    }
  return NULL;
}

int 
_cerebro_metriclist_append(cerebro_metriclist_t metriclist,
                           const char *metric_name)
{
  char *str;

  if (_cerebro_metriclist_check(metriclist) < 0)
    return -1;

  if (!metric_name || strlen(metric_name) > CEREBRO_METRIC_NAME_MAXLEN)
    {
      metriclist->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  /* XXX not safe */
  if (!(str = strdup(metric_name)))
    {
      metriclist->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  if (!list_append(metriclist->metric_names, str))
    {
      metriclist->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }
  
  return 0;

 cleanup:
  free(str);
  return -1;
}
