/*****************************************************************************\
 *  $Id: cerebro_module.h,v 1.3 2005-04-29 06:53:35 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_MODULE_H
#define _CEREBRO_MODULE_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "cerebro_clusterlist_module.h"
#include "cerebro_config_module.h"

#if WITH_STATIC_MODULES                                                                                     
#else /* !WITH_STATIC_MODULES */

/* 
 * cerebro_module_setup
 *
 * Initialize library for loading modules
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_module_setup(void);

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
 * Load the clusterlist module specified by the path.
 *
 * Returns 1 if module is loaded, 0 if it isn't, -1 on fatal error
 */
int cerebro_load_clusterlist_module(char *module_path);

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
 * Load the config module specified by the path.
 *
 * Returns 1 if module is loaded, 0 if it isn't, -1 on fatal error
 */
int cerebro_load_config_module(char *module_path);

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

#endif /* !WITH_STATIC_MODULES */

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
 * cerebro_clusterlist_parse_options
 *
 * call clusterlist module parse options function
 */
int cerebro_clusterlist_parse_options(char **options);
                                                                                     
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
int cerebro_clusterlist_node_in_cluster(char *node);
                                                                                     
/*
 * cerebro_clusterlist_get_nodename
 *
 * call clusterlist module get nodename function
 */
int cerebro_clusterlist_get_nodename(char *node, char *buf, unsigned int buflen);

/*
 * cerebro_config_module_name
 *
 * Return config module name
 */
char *cerebro_config_module_name(void);
                                                                                     
/*
 * cerebro_config_parse_options
 *
 * call config module parse options function
 */
int cerebro_config_parse_options(char **options);
                                                                                     
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
