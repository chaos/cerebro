/*****************************************************************************\
 *  $Id: cerebro_nodes_iterator.c,v 1.1 2005-05-11 21:56:57 achu Exp $
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
#include "cerebro_util.h"
#include "cerebro/cerebro_error.h"

#include "hostlist.h"

/* 
 * _cerebro_nodes_iterator_check
 *
 * Checks for a proper cerebro nodes interator, setting the errnum
 * appropriately if an error is found.
 *
 * Returns 0 on success, -1 on error
 */
int
_cerebro_handle_and_nodes_iterator_check(cerebro_t handle,
                                         cerebro_nodes_iterator_t itr)
{
  if (_cerebro_handle_check(handle) < 0)
    return -1;

  if (!itr)
    {
      handle->errnum = CEREBRO_ERR_PARAMETERS;
      return -1;
    }

  if (itr->magic != CEREBRO_NODES_ITERATOR_MAGIC_NUMBER)
    {
      handle->errnum = CEREBRO_ERR_MAGIC_NUMBER;
      return -1;
    }

  if (!itr->nodesitr)
    {
      cerebro_err_debug_lib("%s(%s:%d): nodesitr null",
                            __FILE__, __FUNCTION__, __LINE__);
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (!itr->nodes)
    {
      cerebro_err_debug_lib("%s(%s:%d): nodes null",
                            __FILE__, __FUNCTION__, __LINE__);
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  
  return 0;
}


int cerebro_nodes_iterator_next(cerebro_t handle, 
                                cerebro_nodes_iterator_t itr,
                                char *buf,
                                unsigned int buflen)
{
  char *node = NULL;
  int rv = 0;

  if (_cerebro_handle_and_nodes_iterator_check(handle, itr) < 0)
    return -1;

  if (!buf || !buflen)
    {
      handle->errnum = CEREBRO_ERR_PARAMETERS;
      return -1;
    }

  if ((node = hostlist_next(itr->nodesitr)))
    {
      if ((strlen(node) + 1) > buflen)
        {
          handle->errnum = CEREBRO_ERR_OVERFLOW;
          goto cleanup;
        }
      strcpy(buf, node);
      rv = 1;
    }
  else
    rv = 0;

  handle->errnum = CEREBRO_ERR_SUCCESS;
 cleanup:
  free(node);
  return rv;
}

int 
cerebro_nodes_iterator_reset(cerebro_t handle,
                             cerebro_nodes_iterator_t itr)
{
  if (_cerebro_handle_and_nodes_iterator_check(handle, itr) < 0)
    return -1;

  hostlist_iterator_reset(itr->nodesitr);
  
  handle->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}
                                                                                
int 
cerebro_nodes_iterator_destroy(cerebro_t handle,
                               cerebro_nodes_iterator_t itr)
{
  if (_cerebro_handle_and_nodes_iterator_check(handle, itr) < 0)
    return -1;

  hostlist_iterator_destroy(itr->nodesitr);
  hostlist_destroy(itr->nodes);
  itr->magic = ~CEREBRO_NODES_ITERATOR_MAGIC_NUMBER;
  itr->nodesitr = NULL;
  itr->nodes = NULL;
  free(itr);

  handle->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}
