/*****************************************************************************\
 *  $Id: cerebro_nodes_iterator_util.h,v 1.2 2005-05-11 22:31:21 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_NODES_ITERATOR_UTIL_H
#define _CEREBRO_NODES_ITERATOR_UTIL_H

#include "cerebro.h"

#include "hostlist.h"

/* 
 * _cerebro_nodes_iterator_create
 *
 * Create a nodes iterator based using the nodes from the hostlist.
 *
 * Returns iterator on success, NULL on error
 */
cerebro_nodes_iterator_t _cerebro_nodes_iterator_create(cerebro_t handle, 
                                                        hostlist_t nodes);

#endif /* _CEREBRO_NODES_ITERATOR_UTIL_H */
