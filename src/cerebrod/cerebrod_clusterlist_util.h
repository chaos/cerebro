/*****************************************************************************\
 *  $Id: cerebrod_clusterlist_util.h,v 1.2 2005-03-17 22:32:03 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_CLUSTERLIST_UTIL_H
#define _CEREBROD_CLUSTERLIST_UTIL_H

int cerebrod_clusterlist_parse_filename(char **options, char **filename);

int cerebrod_clusterlist_copy_nodename(char *node, char *buf, int buflen);

#endif /* _CEREBROD_CLUSTERLIST_UTIL_H */
