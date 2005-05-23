/*****************************************************************************\
 *  $Id: cerebro_nodelist_util.h,v 1.1 2005-05-23 21:30:29 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_NODELIST_UTIL_H
#define _CEREBRO_NODELIST_UTIL_H

#include "cerebro.h"

#include "hostlist.h"

/* 
 * _cerebro_nodelist_create
 *
 * Create a nodes iterator based using the nodes from the hostlist.
 *
 * Returns iterator on success, NULL on error
 */
cerebro_nodelist_t _cerebro_nodelist_create(cerebro_t handle, 
                                            hostlist_t nodes);

#endif /* _CEREBRO_NODELIST_UTIL_H */
