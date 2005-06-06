/*****************************************************************************\
 *  $Id: cerebro_nodelist.c,v 1.17 2005-06-06 23:35:06 achu Exp $
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
#include "cerebro_module.h"
#include "cerebro_nodelist_util.h"
#include "cerebro_util.h"
#include "cerebro/cerebro_error.h"

#include "list.h"

char *
cerebro_nodelist_metric_name(cerebro_nodelist_t nodelist)
{
  if (_cerebro_nodelist_check(nodelist) < 0)
    return NULL;
  
  nodelist->errnum = CEREBRO_ERR_SUCCESS;
  return nodelist->metric_name;
}

int 
cerebro_nodelist_length(cerebro_nodelist_t nodelist)
{
  if (_cerebro_nodelist_check(nodelist) < 0)
    return -1;
  
  nodelist->errnum = CEREBRO_ERR_SUCCESS;
  return list_count(nodelist->nodes);
}

/* 
 * _cerebro_nodelist_find_func
 *
 * Callback function to find node in nodelist.
 *
 * Returns 1 on match, 0 on no match 
 */
int
_cerebro_nodelist_find_func(void *x, void *key)
{
  if (!x || !key)
    return 0;

  return (!strcmp((char *)x, (char *)key) ? 1 : 0);
}

int 
cerebro_nodelist_find(cerebro_nodelist_t nodelist, 
		      const char *node,
		      void **metric_value,
                      unsigned int *metric_type,
                      unsigned int *metric_value_size)
{
  struct cerebro_nodelist_data *data;
  char nodebuf[CEREBRO_MAXNODENAMELEN+1];
  char *nodename;

  if (_cerebro_nodelist_check(nodelist) < 0)
    return -1;

  if (!node)
    {
      nodelist->errnum = CEREBRO_ERR_PARAMETERS;
      return -1;
    }

  if (nodelist->handle->loaded_state & CEREBRO_CLUSTERLIST_MODULE_LOADED)
    {
      int flag;
      
      if ((flag = _cerebro_clusterlist_module_node_in_cluster(node)) < 0)
        {
          nodelist->errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }

      if (!flag)
        {
          nodelist->errnum = CEREBRO_ERR_NODE_NOTFOUND;
          return -1;
        }
      
      memset(nodebuf, '\0', CEREBRO_MAXNODENAMELEN+1);

      if (_cerebro_clusterlist_module_get_nodename(node,
                                                   nodebuf,
                                                   CEREBRO_MAXNODENAMELEN+1) < 0)
        {
          nodelist->errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }

      nodename = nodebuf;
    }
  else
    nodename = (char *)node;

  nodelist->errnum = CEREBRO_ERR_SUCCESS;
  data = list_find_first(nodelist->nodes,
			 _cerebro_nodelist_find_func,
			 (void *)nodename);

  if (data)
    {
      if (metric_value)
        *metric_value = data->metric_data;

      if (metric_type)
        *metric_type = data->metric_type;

      if (metric_value_size)
        *metric_value_size = data->metric_len;
    }

  return (data) ? 1 : 0;
}

int 
cerebro_nodelist_for_each(cerebro_nodelist_t nodelist,
                          Cerebro_for_each for_each,
                          void *arg)
{
  struct cerebro_nodelist_data *data;
  ListIterator itr = NULL;

  if (_cerebro_nodelist_check(nodelist) < 0)
    return -1;

  if (!for_each)
    {
      nodelist->errnum = CEREBRO_ERR_PARAMETERS;
      return -1;
    }
  
  if (!(itr = list_iterator_create(nodelist->nodes)))
    {
      nodelist->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  while ((data = list_next(itr)))
    {
      if (for_each(data->nodename, 
                   data->metric_data, 
                   data->metric_type,
                   data->metric_len,
                   arg) < 0)
	goto cleanup;
    }

  list_iterator_destroy(itr);
  return 0;
 cleanup:
  if (itr)
    list_iterator_destroy(itr);
  return -1;
}

int 
cerebro_nodelist_destroy(cerebro_nodelist_t nodelist)
{
  cerebro_nodelist_t tempnodelist;
  ListIterator itr = NULL;
  int found = 0, rv = -1;

  if (_cerebro_nodelist_check(nodelist) < 0)
    return -1;

  if (!(itr = list_iterator_create(nodelist->handle->nodelists)))
    {
      nodelist->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  while ((tempnodelist = list_next(itr)))
    {
      if (tempnodelist == nodelist)
        {
          list_remove(itr);
          
          nodelist->magic = ~CEREBRO_NODELIST_MAGIC_NUMBER;
          nodelist->errnum = CEREBRO_ERR_SUCCESS;
          /* 
           * destroy nodes first, since it will destroy iterators
           */
          list_destroy(nodelist->nodes);
          nodelist->nodes = NULL;
          list_destroy(nodelist->iterators);
          nodelist->iterators = NULL;
          nodelist->handle = NULL;
          free(nodelist);
          found++;
          break;
        }
    }

  if (!found)
    {
      nodelist->errnum = CEREBRO_ERR_PARAMETERS;
      goto cleanup;
    }

  rv = 0;
  list_iterator_destroy(itr);
  return 0;

 cleanup:
  if (itr)
    list_iterator_destroy(itr);
  return rv;
}
 
cerebro_nodelist_iterator_t 
cerebro_nodelist_iterator_create(cerebro_nodelist_t nodelist)
{
  cerebro_nodelist_iterator_t nodelistItr = NULL;

  if (_cerebro_nodelist_check(nodelist) < 0)
    return NULL;

  if (!(nodelistItr = malloc(sizeof(struct cerebro_nodelist_iterator))))
    {
      nodelist->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }
  memset(nodelistItr, '\0', sizeof(struct cerebro_nodelist_iterator));
  nodelistItr->magic = CEREBRO_NODELIST_ITERATOR_MAGIC_NUMBER;
  
  if (!(nodelistItr->itr = list_iterator_create(nodelist->nodes)))
    {
      nodelist->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }
  
  if (!list_append(nodelist->iterators, nodelistItr))
    {
      nodelist->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  nodelistItr->nodelist = nodelist;
  nodelistItr->errnum = CEREBRO_ERR_SUCCESS;
  nodelistItr->current = list_next(nodelistItr->itr);
  return nodelistItr;

 cleanup:
  if (nodelistItr)
    {
      if (nodelistItr->itr)
        list_iterator_destroy(nodelistItr->itr);
      free(nodelistItr);
    }
  return NULL;
}
 
/* 
 * _cerebro_nodelist_iterator_check
 *
 * Checks for a proper cerebro nodelist
 *
 * Returns 0 on success, -1 on error
 */
int
_cerebro_nodelist_iterator_check(cerebro_nodelist_iterator_t nodelistItr)
{
  if (!nodelistItr 
      || nodelistItr->magic != CEREBRO_NODELIST_ITERATOR_MAGIC_NUMBER)
    return -1;

  if (!nodelistItr->itr)
    {
      cerebro_err_debug_lib("%s(%s:%d): itr null",
                            __FILE__, __FUNCTION__, __LINE__);
      nodelistItr->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (!nodelistItr->nodelist)
    {
      cerebro_err_debug_lib("%s(%s:%d): nodelist null",
                            __FILE__, __FUNCTION__, __LINE__);
      nodelistItr->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  
  if (nodelistItr->nodelist->magic != CEREBRO_NODELIST_MAGIC_NUMBER)
    {
      cerebro_err_debug_lib("%s(%s:%d): nodelist destroyed",
                            __FILE__, __FUNCTION__, __LINE__);
      nodelistItr->errnum = CEREBRO_ERR_MAGIC_NUMBER;
      return -1;
    }

  return 0;
}


int 
cerebro_nodelist_iterator_nodename(cerebro_nodelist_iterator_t nodelistItr,
                                   char **nodename)
{
  if (_cerebro_nodelist_iterator_check(nodelistItr) < 0)
    return -1;
  
  if (!nodelistItr->current)
    {
      nodelistItr->errnum = CEREBRO_ERR_END_OF_LIST;
      return -1;
    }
  
  if (nodename)
    *nodename = nodelistItr->current->nodename;
  return 0;
}

int 
cerebro_nodelist_iterator_metric_value(cerebro_nodelist_iterator_t nodelistItr,
                                       void **metric_value,
                                       unsigned int *metric_type,
                                       unsigned int *metric_value_size)
{
  if (_cerebro_nodelist_iterator_check(nodelistItr) < 0)
    return -1;

  if (!nodelistItr->current)
    {
      nodelistItr->errnum = CEREBRO_ERR_END_OF_LIST;
      return -1;
    }

  if (metric_value)
    *metric_value = nodelistItr->current->metric_data;
  
  if (metric_type)
    *metric_type = nodelistItr->current->metric_type;
  
  if (metric_value_size)
    *metric_value_size = nodelistItr->current->metric_len;
  
  return 0;
}

int
cerebro_nodelist_iterator_next(cerebro_nodelist_iterator_t nodelistItr)
{
  if (_cerebro_nodelist_iterator_check(nodelistItr) < 0)
    return -1;

  if (nodelistItr->current)
    nodelistItr->current = (struct cerebro_nodelist_data *)list_next(nodelistItr->itr);
  nodelistItr->errnum = CEREBRO_ERR_SUCCESS;
  return (nodelistItr->current) ? 1 : 0;
}
 
int 
cerebro_nodelist_iterator_reset(cerebro_nodelist_iterator_t nodelistItr)
{
  if (_cerebro_nodelist_iterator_check(nodelistItr) < 0)
    return -1;

  list_iterator_reset(nodelistItr->itr);
  nodelistItr->current = (struct cerebro_nodelist_data *)list_next(nodelistItr->itr);
  nodelistItr->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}

int
cerebro_nodelist_iterator_at_end(cerebro_nodelist_iterator_t nodelistItr)
{
  if (_cerebro_nodelist_iterator_check(nodelistItr) < 0)
    return -1;

  return (nodelistItr->current) ? 0 : 1;
}

int 
cerebro_nodelist_iterator_destroy(cerebro_nodelist_iterator_t nodelistItr)
{
  cerebro_nodelist_t nodelist;
  cerebro_nodelist_iterator_t tempItr;
  ListIterator itr = NULL;
  int found = 0, rv = -1;

  if (_cerebro_nodelist_iterator_check(nodelistItr) < 0)
    return -1;
  
  nodelist = nodelistItr->nodelist;

  if (!(itr = list_iterator_create(nodelist->iterators)))
    {
      nodelistItr->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }
  
  while ((tempItr = list_next(itr)))
    {
      if (tempItr == nodelistItr)
        {
          list_remove(itr);
          list_iterator_destroy(nodelistItr->itr);
          nodelistItr->magic = ~CEREBRO_NODELIST_ITERATOR_MAGIC_NUMBER;
          nodelistItr->errnum = CEREBRO_ERR_SUCCESS;
          free(nodelistItr);
          found++;
          break;
        }
    }
  
  if (!found)
    {
      nodelistItr->errnum = CEREBRO_ERR_PARAMETERS;
      goto cleanup;
    }

  rv = 0;
 cleanup:
  if (itr)
    list_iterator_destroy(itr);
  return rv;
}
