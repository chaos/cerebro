/*****************************************************************************\
 *  $Id: cerebrod_clusterlist_genders_util.h,v 1.1 2005-03-17 22:32:03 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_CLUSTERLIST_GENDERS_UTIL_H
#define _CEREBROD_CLUSTERLIST_GENDERS_UTIL_H

int cerebrod_clusterlist_genders_init(genders_t *handle, char *file);

int cerebrod_clusterlist_genders_finish(genders_t *handle, char **file);

int cerebrod_clusterlist_genders_get_all_nodes(genders_t handle, char **nodes, unsigned int nodeslen);

int cerebrod_clusterlist_genders_numnodes(genders_t handle);

#endif /* _CEREBROD_CLUSTERLIST_GENDERS_UTIL_H */
