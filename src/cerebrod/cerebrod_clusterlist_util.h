/*****************************************************************************\
 *  $Id: cerebrod_clusterlist_util.h,v 1.4 2005-04-20 19:43:22 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_CLUSTERLIST_UTIL_H
#define _CEREBROD_CLUSTERLIST_UTIL_H

int cerebrod_clusterlist_parse_filename(char **options, char **filename, char *clusterlist_module_name);

int cerebrod_clusterlist_copy_nodename(char *node, char *buf, unsigned int buflen, char *clusterlist_module_name);

#endif /* _CEREBROD_CLUSTERLIST_UTIL_H */
