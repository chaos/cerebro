/*****************************************************************************\
 *  $Id: cerebrod_clusterlist_util.h,v 1.3 2005-03-21 14:36:47 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_CLUSTERLIST_UTIL_H
#define _CEREBROD_CLUSTERLIST_UTIL_H

int cerebrod_clusterlist_parse_filename(char **options, char **filename);

int cerebrod_clusterlist_copy_nodename(char *node, char *buf, unsigned int buflen);

#endif /* _CEREBROD_CLUSTERLIST_UTIL_H */
