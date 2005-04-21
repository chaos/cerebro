/*****************************************************************************\
 *  $Id: cerebro_clusterlist_genders_util.h,v 1.2 2005-04-21 17:59:15 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_CLUSTERLIST_GENDERS_UTIL_H
#define _CEREBRO_CLUSTERLIST_GENDERS_UTIL_H

/* 
 * cerebro_clusterlist_genders_init
 *
 * common init function for genders and gendersllnl init
 */
int cerebro_clusterlist_genders_init(genders_t *handle, 
                                     char *file, 
                                     char *clusterlist_module_name);

/* 
 * cerebro_clusterlist_genders_finish
 *
 * common finish function for genders and gendersllnl finish
 */
int cerebro_clusterlist_genders_finish(genders_t *handle, 
                                       char **file, 
                                       char *clusterlist_module_name);

/* 
 * cerebro_clusterlist_genders_get_all_nodes
 *
 * common get_all_nodes function for genders and gendersllnl get_all_nodes
 */
int cerebro_clusterlist_genders_get_all_nodes(genders_t handle, 
                                              char **nodes, 
                                              unsigned int nodeslen, 
                                              char *clusterlist_module_name);

/* 
 * cerebro_clusterlist_genders_numnodes
 *
 * common numnodes function for genders and gendersllnl numnodes
 */
int cerebro_clusterlist_genders_numnodes(genders_t handle, 
                                         char *clusterlist_module_name);

#endif /* _CEREBRO_CLUSTERLIST_GENDERS_UTIL_H */
