/*****************************************************************************\
 *  $Id: cerebro_module_clusterlist.h,v 1.1 2005-06-18 06:47:06 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_MODULE_CLUSTERLIST_H
#define _CEREBRO_MODULE_CLUSTERLIST_H

typedef struct cerebro_clusterlist_module *cerebro_clusterlist_module_t;

/*
 * _cerebro_module_load_clusterlist_module
 *
 * Find and load the clusterlist module.  If none is found, cerebro
 * library will assume a default clusterlist module.
 * 
 * Returns clusterlist module handle on success, NULL on error
 */
cerebro_clusterlist_module_t _cerebro_module_load_clusterlist_module(void);

/*
 * _cerebro_module_destroy_clusterlist_handle
 *
 * Destroy/Unload the clusterlist module specified by the handle.
 *
 * Returns 0 on success, -1 on error
 */
int _cerebro_module_destroy_clusterlist_handle(cerebro_clusterlist_module_t clusterlist_handle);

/*
 * _cerebro_clusterlist_module_name
 *
 * Return clusterlist module name
 */
char *_cerebro_clusterlist_module_name(cerebro_clusterlist_module_t clusterlist_handle);

/*
 * _cerebro_clusterlist_module_setup
 *
 * call clusterlist module setup function
 */
int _cerebro_clusterlist_module_setup(cerebro_clusterlist_module_t clusterlist_handle);

/*
 * _cerebro_clusterlist_module_cleanup
 *
 * call clusterlist module cleanup function
 */
int _cerebro_clusterlist_module_cleanup(cerebro_clusterlist_module_t clusterlist_handle);

/*
 * _cerebro_clusterlist_module_numnodes
 *
 * call clusterlist module numnodes function
 */
int _cerebro_clusterlist_module_numnodes(cerebro_clusterlist_module_t clusterlist_handle);

/*
 * _cerebro_clusterlist_module_get_all_nodes
 *
 * call clusterlist module get all nodes function
 */
int _cerebro_clusterlist_module_get_all_nodes(cerebro_clusterlist_module_t clusterlist_handle,
                                              char ***nodes);

/*
 * _cerebro_clusterlist_module_node_in_cluster
 *
 * call clusterlist module node in cluster function
 */
int _cerebro_clusterlist_module_node_in_cluster(cerebro_clusterlist_module_t clusterlist_handle,
                                                const char *node);

/*
 * _cerebro_clusterlist_module_get_nodename
 *
 * call clusterlist module get nodename function
 */
int _cerebro_clusterlist_module_get_nodename(cerebro_clusterlist_module_t clusterlist_handle,
                                             const char *node, 
                                             char *buf, 
                                             unsigned int buflen);

#endif /* _CEREBRO_MODULE_CLUSTERLIST_H */
