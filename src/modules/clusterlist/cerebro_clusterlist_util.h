/*****************************************************************************\
 *  $Id: cerebro_clusterlist_util.h,v 1.2 2005-04-21 17:59:15 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_CLUSTERLIST_UTIL_H
#define _CEREBRO_CLUSTERLIST_UTIL_H

/* 
 * cerebro_clusterlist_parse_filename
 *
 * Parse the key=value pair filename=<filename> out of the NULL
 * terminated array of options pointer strings.  Malloc a copy of the
 * filename into 'filename' if one is found.
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_clusterlist_parse_filename(char **options, 
                                       char **filename, 
                                       char *clusterlist_module_name);

/* 
 * cerebro_clusterlist_copy_nodename
 *
 * Copy a node string into a buffer, checking for length appropriately
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_clusterlist_copy_nodename(char *node, 
                                      char *buf, 
                                      unsigned int buflen, 
                                      char *clusterlist_module_name);

#endif /* _CEREBRO_CLUSTERLIST_UTIL_H */
