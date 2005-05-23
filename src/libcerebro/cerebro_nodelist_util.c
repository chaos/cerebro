/*****************************************************************************\
 *  $Id: cerebro_nodelist_util.c,v 1.1 2005-05-23 21:30:29 achu Exp $
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
#if 0
#include "cerebro_nodes_iterator_util.h"
#endif /* 0 */
#include "cerebro/cerebro_error.h"

#include "hostlist.h"

#if 0
cerebro_nodes_iterator_t 
_cerebro_nodes_iterator_create(cerebro_t handle,
                               hostlist_t nodes)
{
  cerebro_nodes_iterator_t itr = NULL;

  if (!nodes)
    {
      cerebro_err_debug_lib("%s(%s:%d): nodes null",
			    __FILE__, __FUNCTION__, __LINE__);
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return NULL;
    }

  if (!(itr = (cerebro_nodes_iterator_t)malloc(sizeof(struct cerebro_nodes_iterator))))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  memset(itr, '\0', sizeof(struct cerebro_nodes_iterator));

  if (!(itr->nodes = hostlist_copy(nodes)))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  if (!(itr->nodesitr = hostlist_iterator_create(itr->nodes)))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  itr->magic = CEREBRO_NODES_ITERATOR_MAGIC_NUMBER;
  return itr;

 cleanup:
  if (itr)
    {
      if (itr->nodesitr)
        hostlist_iterator_destroy(itr->nodesitr);
      if (itr->nodes)
        hostlist_destroy(itr->nodes);
    }
  return NULL;
}

#endif /* 0 */
