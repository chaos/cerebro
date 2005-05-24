/*****************************************************************************\
 *  $Id: cerebro_nodelist_util.c,v 1.3 2005-05-24 21:35:34 achu Exp $
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

/* 
 * _cerebro_nodelist_cleanup
 *
 * Common cleanup routine for _cerebro_nodelist_create(),
 * _cerebro_nodelist_by_list_create(), and
 * _cerebro_nodelist_by_hostlist_create().
 *
 */
void _cerebro_nodelist_cleanup(cerebro_nodelist_t nodelist)
{
  if (nodelist)
    {
      if (nodelist->nodes)
        list_destroy(nodelist->nodes);
      if (nodelist->iterators)
        list_destroy(nodelist->iterators);
      free(nodelist);
    }
}

/* 
 * _cerebro_nodelist_create
 *
 * Common creation function for _cerebro_nodelist_by_list_create()
 * and _cerebro_nodelist_by_hostlist_create().
 *
 * Returns nodelist on success, NULL on error
 */
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
  _cerebro_nodelist_cleanup(nodelist);
  return NULL;
}

cerebro_nodelist_t 
_cerebro_nodelist_by_list_create(cerebro_t handle, List nodes)
{
  cerebro_nodelist_t nodelist = NULL;
  ListIterator itr = NULL;
  char *nodename;

  if (!nodes)
    {
      cerebro_err_debug_lib("%s(%s:%d): nodes null",
			    __FILE__, __FUNCTION__, __LINE__);
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return NULL;
    }

  if (!(nodelist = _cerebro_nodelist_create(handle)))
    goto cleanup;

  if (!(itr = list_iterator_create(nodes)))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }
  
  while ((nodename = list_next(itr)))
    {
      char *ptr;

      if (!(ptr = strdup(nodename)))
        {
          handle->errnum = CEREBRO_ERR_OUTMEM;
          goto cleanup;
        }

      if (!list_append(nodelist->nodes, ptr))
        {
          handle->errnum = CEREBRO_ERR_INTERNAL;
          goto cleanup;
        }
    }
  
  list_iterator_destroy(itr);
  return nodelist;
      
 cleanup:
  _cerebro_nodelist_cleanup(nodelist);
  if (itr)
    list_iterator_destroy(itr);
  return NULL;
}

cerebro_nodelist_t 
_cerebro_nodelist_by_hostlist_create(cerebro_t handle, hostlist_t nodes)
{
  cerebro_nodelist_t nodelist = NULL;
  hostlist_iterator_t itr = NULL;
  char *nodename = NULL;

  if (!nodes)
    {
      cerebro_err_debug_lib("%s(%s:%d): nodes null",
			    __FILE__, __FUNCTION__, __LINE__);
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return NULL;
    }

  if (!(nodelist = _cerebro_nodelist_create(handle)))
    goto cleanup;

  if (!(itr = hostlist_iterator_create(nodes)))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  while ((nodename = hostlist_next(itr)))
    {
      if (!list_append(nodelist->nodes, nodename))
        {
          handle->errnum = CEREBRO_ERR_INTERNAL;
          goto cleanup;
        }
    }
  nodename = NULL;

  hostlist_iterator_destroy(itr);
  return nodelist;

 cleanup:
  _cerebro_nodelist_cleanup(nodelist);
  if (itr)
    hostlist_iterator_destroy(itr);
  if (nodename)
    free(nodename);
  return NULL;
}


