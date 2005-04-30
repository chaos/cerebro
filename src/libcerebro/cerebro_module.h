/*****************************************************************************\
 *  $Id: cerebro_module.h,v 1.8 2005-04-30 17:09:10 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_MODULE_H
#define _CEREBRO_MODULE_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "cerebro_clusterlist_module.h"
#include "cerebro_config_module.h"

/* 
 * cerebro_module_setup
 *
 * Initialize library for loading modules
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_module_setup(void);

/* 
 * cerebro_module_is_setup
 *
 * Return 1 if module lib has been initialized, 0 if not
 */
int cerebro_module_is_setup(void);

/* 
 * cerebro_module_cleanup
 *
 * Cleanup library from loading modules
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_module_cleanup(void);

/*
 * cerebro_load_clusterlist_module
 *
 * If compiled statically, load the clusterlist module specified by the module name.
 * 
 * If compiled dynamically, load the clusterlist module specified by the module path.
 *
 * Returns 1 if module is loaded, 0 if it isn't, -1 on fatal error
 */
int cerebro_load_clusterlist_module(char *module);

/*
 * cerebro_unload_clusterlist_module
 *
 * Unload the clusterlist module.
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_unload_clusterlist_module(void);

/*
 * cerebro_find_clusterlist_module
 *
 * find a clusterlist module from a list of modules
 *
 * Returns 1 if a module is found, 0 if one is not, -1 on error
 */
int cerebro_find_clusterlist_module(void);

/*
 * cerebro_load_config_module
 *
 * If compiled statically, load the config module specified by the module name.
 * 
 * If compiled dynamically, load the config module specified by the module path.
 *
 * Returns 1 if module is loaded, 0 if it isn't, -1 on fatal error
 */
int cerebro_load_config_module(char *module);

/*
 * cerebro_unload_config_module
 *
 * Unload the config module.
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_unload_config_module(void);

/*
 * cerebro_find_config_module
 *
 * find a config module from a list of modules
 *
 * Returns 1 if a module is found, 0 if one is not, -1 on error
 */
int cerebro_find_config_module(void);

/* 
 * cerebro_clusterlist_is_loaded
 * 
 * Return 1 if a clusterlist module has been loaded, 0 if not
 */
int cerebro_clusterlist_is_loaded(void);

/* 
 * cerebro_config_is_loaded
 * 
 * Return 1 if a config module has been loaded, 0 if not
 */
int cerebro_config_is_loaded(void);

/*
 * cerebro_clusterlist_module_name
 *
 * Return clusterlist module name
 */
char *cerebro_clusterlist_module_name(void);

/*
 * cerebro_clusterlist_setup
 *
 * call clusterlist module setup function
 */
int cerebro_clusterlist_setup(void);
                                                                                     
/*
 * cerebro_clusterlist_cleanup
 *
 * call clusterlist module parse cleanup function
 */
int cerebro_clusterlist_cleanup(void);
                                                                                     
/*
 * cerebro_clusterlist_get_all_nodes
 *
 * call clusterlist module get all nodes function
 */
int cerebro_clusterlist_get_all_nodes(char **nodes, unsigned int nodeslen);

/*
 * cerebro_clusterlist_numnodes
 *
 * call clusterlist module numnodes function
 */
int cerebro_clusterlist_numnodes(void);
                                                                                     
/*
 * cerebro_clusterlist_node_in_cluster
 *
 * call clusterlist module node in cluster function
 */
int cerebro_clusterlist_node_in_cluster(const char *node);
                                                                                     
/*
 * cerebro_clusterlist_get_nodename
 *
 * call clusterlist module get nodename function
 */
int cerebro_clusterlist_get_nodename(const char *node, char *buf, unsigned int buflen);

/*
 * cerebro_config_module_name
 *
 * Return config module name
 */
char *cerebro_config_module_name(void);
                                                                                     
/*
 * cerebro_config_setup
 *
 * call config module setup function
 */
int cerebro_config_setup(void);
                                                                                     
/*
 * cerebro_config_cleanup
 *
 * call config module parse cleanup function
 */
int cerebro_config_cleanup(void);
                                                                                     
/*
 * cerebro_config_load_cerebrod_default
 *
 * call config module get all nodes function
 */
int cerebro_config_load_cerebrod_default(struct cerebrod_module_config *conf);

#endif /* _CEREBRO_MODULE_H */
