/*****************************************************************************\
 *  $Id: cerebro_clusterlist_util.h,v 1.5 2005-05-04 20:58:09 achu Exp $
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
                                      unsigned int buflen);

#endif /* _CEREBRO_CLUSTERLIST_UTIL_H */
