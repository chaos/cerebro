/*****************************************************************************\
 *  $Id: cerebro_clusterlist_util.h,v 1.1 2005-04-20 23:36:26 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_CLUSTERLIST_UTIL_H
#define _CEREBRO_CLUSTERLIST_UTIL_H

int cerebro_clusterlist_parse_filename(char **options, char **filename, char *clusterlist_module_name);

int cerebro_clusterlist_copy_nodename(char *node, char *buf, unsigned int buflen, char *clusterlist_module_name);

#endif /* _CEREBRO_CLUSTERLIST_UTIL_H */
