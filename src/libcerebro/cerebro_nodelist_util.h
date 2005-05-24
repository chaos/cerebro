/*****************************************************************************\
 *  $Id: cerebro_nodelist_util.h,v 1.2 2005-05-24 00:07:45 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_NODELIST_UTIL_H
#define _CEREBRO_NODELIST_UTIL_H

#include "cerebro.h"

#include "list.h"
#include "hostlist.h"

/* 
 * _cerebro_nodelist_by_list_create
 *
 * Create a nodelist based using the nodes in the list.
 *
 * Returns nodelist on success, NULL on error
 */
cerebro_nodelist_t _cerebro_nodelist_by_list_create(cerebro_t handle, 
                                                    List nodes);

/* 
 * _cerebro_nodelist_by_hostlist_create
 *
 * Create a nodelist based using the nodes in the hostlist.
 *
 * Returns nodelist on success, NULL on error
 */
cerebro_nodelist_t _cerebro_nodelist_by_hostlist_create(cerebro_t handle, 
                                                        hostlist_t nodes);

#endif /* _CEREBRO_NODELIST_UTIL_H */
