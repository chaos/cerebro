/*****************************************************************************\
 *  $Id: cerebro_nodelist_util.c,v 1.3 2005-06-23 22:54:05 achu Exp $
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
#include "cerebro_nodelist_util.h"
#include "cerebro/cerebro_error.h"

#include "list.h"

int
_cerebro_nodelist_check(cerebro_nodelist_t nodelist)
{
  if (!nodelist || nodelist->magic != CEREBRO_NODELIST_MAGIC_NUMBER)
    return -1;

  if (!nodelist->nodes)
    {
      cerebro_err_debug("%s(%s:%d): nodelist null",
			__FILE__, __FUNCTION__, __LINE__);
      nodelist->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (!nodelist->iterators)
    {
      cerebro_err_debug("%s(%s:%d): iterators null",
			__FILE__, __FUNCTION__, __LINE__);
      nodelist->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (!nodelist->handle)
    {
      cerebro_err_debug("%s(%s:%d): handle null",
			__FILE__, __FUNCTION__, __LINE__);
      nodelist->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (nodelist->handle->magic != CEREBRO_MAGIC_NUMBER)
    {
      cerebro_err_debug("%s(%s:%d): handle destroyed",
			__FILE__, __FUNCTION__, __LINE__);
      nodelist->errnum = CEREBRO_ERR_MAGIC_NUMBER;
      return -1;
    }

  return 0;
}

/* 
 * _cerebro_nodelist_data_destroy
 *
 * Free contents of a nodelist data item
 */
static void
_cerebro_nodelist_data_destroy(void *x)
{
  struct cerebro_nodelist_data *nd;
  
  nd = (struct cerebro_nodelist_data *)x;

  if (nd->metric_value)
    free(nd->metric_value);
  free(nd);
}

cerebro_nodelist_t 
_cerebro_nodelist_create(cerebro_t handle, const char *metric_name)
{
  cerebro_nodelist_t nodelist = NULL;

  if (!(nodelist = (cerebro_nodelist_t)malloc(sizeof(struct cerebro_nodelist))))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  memset(nodelist, '\0', sizeof(struct cerebro_nodelist));
  nodelist->magic = CEREBRO_NODELIST_MAGIC_NUMBER;
  strcpy(nodelist->metric_name, metric_name);

  if (!(nodelist->nodes = list_create((ListDelF)_cerebro_nodelist_data_destroy)))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  /* No delete function, list_destroy() on 'nodes' will take care of
   * deleting iterators.
   */
  if (!(nodelist->iterators = list_create(NULL)))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  if (!list_append(handle->nodelists, nodelist))
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }
  
  nodelist->handle = handle;

  return nodelist;
      
 cleanup:
  if (nodelist)
    {
      if (nodelist->nodes)
        list_destroy(nodelist->nodes);
      if (nodelist->iterators)
        list_destroy(nodelist->iterators);
      free(nodelist);
    }
  return NULL;
}

int 
_cerebro_nodelist_append(cerebro_nodelist_t nodelist,
			 const char *nodename,
                         u_int32_t metric_value_type,
                         u_int32_t metric_value_len,
                         void *metric_value)
{
  struct cerebro_nodelist_data *nd = NULL;

  if (_cerebro_nodelist_check(nodelist) < 0)
    return -1;

  if (!nodename || strlen(nodename) > CEREBRO_MAXNODENAMELEN)
    {
      nodelist->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  if (!(nd = (struct cerebro_nodelist_data *)malloc(sizeof(struct cerebro_nodelist_data))))
    {
      nodelist->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }
  memset(nd, '\0', sizeof(struct cerebro_nodelist_data));

  strncpy(nd->nodename, nodename, CEREBRO_MAXNODENAMELEN);
  nd->metric_value_type = metric_value_type;
  nd->metric_value_len = metric_value_len;
  nd->metric_value = metric_value;

  if (!list_append(nodelist->nodes, nd))
    {
      nodelist->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }
  
  return 0;

 cleanup:
  free(nd);
  return -1;
}

/*
 * _cerebro_nodelist_data_strcmp
 *
 * String comparison for nodelist data nodename
 *
 * Returns value identical to strcmp() for string comparisons
 */
int
_cerebro_nodelist_data_strcmp(void *x, void *y)
{
  struct cerebro_nodelist_data *datax, *datay;
  datax = (struct cerebro_nodelist_data *)x;
  datay = (struct cerebro_nodelist_data *)y;

  return strcmp(datax->nodename, datay->nodename);
}

int
_cerebro_nodelist_sort(cerebro_nodelist_t nodelist)
{
  if (_cerebro_nodelist_check(nodelist) < 0)
    return -1;

  list_sort(nodelist->nodes, _cerebro_nodelist_data_strcmp);
  return 0;
}
