/*****************************************************************************\
 *  $Id: cerebrod_clusterlist_genders_util.h,v 1.3 2005-04-20 19:43:22 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_CLUSTERLIST_GENDERS_UTIL_H
#define _CEREBROD_CLUSTERLIST_GENDERS_UTIL_H

/* 
 * cerebrod_clusterlist_genders_init
 *
 * common init function for genders and gendersllnl init
 */
int cerebrod_clusterlist_genders_init(genders_t *handle, char *file, char *clusterlist_module_name);

/* 
 * cerebrod_clusterlist_genders_finish
 *
 * common finish function for genders and gendersllnl finish
 */
int cerebrod_clusterlist_genders_finish(genders_t *handle, char **file, char *clusterlist_module_name);

/* 
 * cerebrod_clusterlist_genders_get_all_nodes
 *
 * common get_all_nodes function for genders and gendersllnl get_all_nodes
 */
int cerebrod_clusterlist_genders_get_all_nodes(genders_t handle, char **nodes, unsigned int nodeslen, char *clusterlist_module_name);

/* 
 * cerebrod_clusterlist_genders_numnodes
 *
 * common numnodes function for genders and gendersllnl numnodes
 */
int cerebrod_clusterlist_genders_numnodes(genders_t handle, char *clusterlist_module_name);

#endif /* _CEREBROD_CLUSTERLIST_GENDERS_UTIL_H */
