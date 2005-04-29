/*****************************************************************************\
 *  $Id: cerebro_module.h,v 1.2 2005-04-29 06:33:38 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_MODULE_H
#define _CEREBRO_MODULE_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "cerebro_clusterlist_module.h"
#include "cerebro_config_module.h"

#if WITH_STATIC_MODULES                                                                                     
#else !WITH_STATIC_MODULES

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

#endif /* _CEREBRO_MODULE_H */
