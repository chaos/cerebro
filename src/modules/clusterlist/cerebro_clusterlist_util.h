/*****************************************************************************\
 *  $Id: cerebro_clusterlist_util.h,v 1.4 2005-04-29 23:39:44 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_CLUSTERLIST_UTIL_H
#define _CEREBRO_CLUSTERLIST_UTIL_H

/* 
 * cerebro_clusterlist_copy_nodename
 *
 * Copy a node string into a buffer, checking for length appropriately
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_clusterlist_copy_nodename(const char *node, 
                                      char *buf, 
                                      unsigned int buflen, 
                                      char *clusterlist_module_name);

#endif /* _CEREBRO_CLUSTERLIST_UTIL_H */
