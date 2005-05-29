/*****************************************************************************\
 *  $Id: cerebro_nodelist_util.c,v 1.5 2005-05-29 14:45:52 achu Exp $
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
#include "hostlist.h"

int
_cerebro_nodelist_check(cerebro_nodelist_t nodelist)
{
  if (!nodelist || nodelist->magic != CEREBRO_NODELIST_MAGIC_NUMBER)
    return -1;

  if (!nodelist->nodes)
    {
      cerebro_err_debug_lib("%s(%s:%d): nodelist null",
                            __FILE__, __FUNCTION__, __LINE__);
      nodelist->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (!nodelist->iterators)
    {
      cerebro_err_debug_lib("%s(%s:%d): iterators null",
                            __FILE__, __FUNCTION__, __LINE__);
      nodelist->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  return 0;
}

cerebro_nodelist_t 
_cerebro_nodelist_create(cerebro_t handle)
{
  cerebro_nodelist_t nodelist = NULL;

  if (!(nodelist = (cerebro_nodelist_t)malloc(sizeof(struct cerebro_nodelist))))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  memset(nodelist, '\0', sizeof(struct cerebro_nodelist));
  nodelist->magic = CEREBRO_NODELIST_MAGIC_NUMBER;

  if (!(nodelist->nodes = list_create(free)))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  if (!(nodelist->iterators = list_create(NULL)))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

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
			 cerebro_metric_value_t *metric_value)
{
  struct cerebro_nodelist_data *data;

  if (_cerebro_nodelist_check(nodelist) < 0)
    return -1;

  if (!nodename || strlen(nodename) > CEREBRO_MAXNODENAMELEN)
    {
      nodelist->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  if (!(data = (struct cerebro_nodelist_data *)malloc(sizeof(struct cerebro_nodelist_data))))
    {
      nodelist->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }
  memset(data, '\0', sizeof(struct cerebro_nodelist_data));

  strcpy(data->nodename, nodename);
  memcpy(&(data->metric_value), metric_value, sizeof(cerebro_metric_value_t));

  if (!list_append(nodelist->nodes, data))
    {
      nodelist->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }
  
 cleanup:
  free(data);
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
