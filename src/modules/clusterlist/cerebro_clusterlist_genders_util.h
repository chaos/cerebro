/*****************************************************************************\
 *  $Id: cerebro_clusterlist_genders_util.h,v 1.7 2005-05-10 18:18:52 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_CLUSTERLIST_GENDERS_UTIL_H
#define _CEREBRO_CLUSTERLIST_GENDERS_UTIL_H

/* 
 * cerebro_clusterlist_genders_setup
 *
 * common setup function for genders and gendersllnl setup
 */
int cerebro_clusterlist_genders_setup(genders_t *handle);

/* 
 * cerebro_clusterlist_genders_cleanup
 *
 * common cleanup function for genders and gendersllnl cleanup
 */
int cerebro_clusterlist_genders_cleanup(genders_t *handle);

/* 
 * cerebro_clusterlist_genders_numnodes
 *
 * common numnodes function for genders and gendersllnl numnodes
 */
int cerebro_clusterlist_genders_numnodes(genders_t handle);

/* 
 * cerebro_clusterlist_genders_get_all_nodes
 *
 * common get_all_nodes function for genders and gendersllnl get_all_nodes
 */
int cerebro_clusterlist_genders_get_all_nodes(genders_t handle, 
                                              char ***nodes);

#endif /* _CEREBRO_CLUSTERLIST_GENDERS_UTIL_H */
