/*****************************************************************************\
 *  $Id: cerebrod_clusterlist_genders_util.h,v 1.2 2005-03-20 22:17:17 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_CLUSTERLIST_GENDERS_UTIL_H
#define _CEREBROD_CLUSTERLIST_GENDERS_UTIL_H

/* 
 * cerebrod_clusterlist_genders_init
 *
 * common init function for genders and gendersllnl init
 */
int cerebrod_clusterlist_genders_init(genders_t *handle, char *file);

/* 
 * cerebrod_clusterlist_genders_finish
 *
 * common finish function for genders and gendersllnl finish
 */
int cerebrod_clusterlist_genders_finish(genders_t *handle, char **file);

/* 
 * cerebrod_clusterlist_genders_get_all_nodes
 *
 * common get_all_nodes function for genders and gendersllnl get_all_nodes
 */
int cerebrod_clusterlist_genders_get_all_nodes(genders_t handle, char **nodes, unsigned int nodeslen);

/* 
 * cerebrod_clusterlist_genders_numnodes
 *
 * common numnodes function for genders and gendersllnl numnodes
 */
int cerebrod_clusterlist_genders_numnodes(genders_t handle);

#endif /* _CEREBROD_CLUSTERLIST_GENDERS_UTIL_H */
