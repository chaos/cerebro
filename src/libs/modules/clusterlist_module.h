/*****************************************************************************\
 *  $Id: clusterlist_module.h,v 1.2 2005-07-01 17:13:50 achu Exp $
\*****************************************************************************/

#ifndef _CLUSTERLIST_MODULE_H
#define _CLUSTERLIST_MODULE_H

typedef struct clusterlist_module *clusterlist_module_t;

/*
 * clusterlist_module_load
 *
 * Find and load the clusterlist module.  If none is found, will
 * assume a default clusterlist module.
 * 
 * Returns clusterlist module handle on success, NULL on error
 */
clusterlist_module_t clusterlist_module_load(void);

/*
 * clusterlist_module_unload
 *
 * Unload/cleanup the clusterlist module specified by the handle.
 *
 * Returns 0 on success, -1 on error
 */
int clusterlist_module_unload(clusterlist_module_t handle);

/*
 * clusterlist_module_name
 *
 * Return clusterlist module name
 */
char *clusterlist_module_name(clusterlist_module_t handle);

/*
 * clusterlist_module_setup
 *
 * call clusterlist module setup function
 */
int clusterlist_module_setup(clusterlist_module_t handle);

/*
 * clusterlist_module_cleanup
 *
 * call clusterlist module cleanup function
 */
int clusterlist_module_cleanup(clusterlist_module_t handle);

/*
 * clusterlist_module_numnodes
 *
 * call clusterlist module numnodes function
 */
int clusterlist_module_numnodes(clusterlist_module_t handle);

/*
 * clusterlist_module_get_all_nodes
 *
 * call clusterlist module get all nodes function
 */
int clusterlist_module_get_all_nodes(clusterlist_module_t handle, char ***nodes);

/*
 * clusterlist_module_node_in_cluster
 *
 * call clusterlist module node in cluster function
 */
int clusterlist_module_node_in_cluster(clusterlist_module_t handle, 
                                       const char *node);

/*
 * clusterlist_module_get_nodename
 *
 * call clusterlist module get nodename function
 */
int clusterlist_module_get_nodename(clusterlist_module_t handle,
				    const char *node, 
				    char *buf, 
				    unsigned int buflen);

#endif /* _CLUSTERLIST_MODULE_H */
