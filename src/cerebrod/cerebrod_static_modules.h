/*****************************************************************************\
 *  $Id: cerebrod_static_modules.h,v 1.4 2005-04-20 23:36:26 achu Exp $
\*****************************************************************************/
 
#ifndef _CEREBROD_STATIC_MODULES_H
#define _CEREBROD_STATIC_MODULES_H
 
#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if WITH_STATIC_MODULES

#include "cerebrod_clusterlist_module.h"
#include "cerebrod_config_module.h"

/* 
 * Config modules
 */

#if WITH_GENDERSLLNL
extern struct cerebrod_config_module_info gendersllnl_config_module_info;
#endif

/* 
 * Clusterlist modules
 */
#if WITH_GENDERSLLNL
extern struct cerebro_clusterlist_module_info gendersllnl_clusterlist_module_info;
#endif

#if WITH_GENDERS
extern struct cerebro_clusterlist_module_info genders_clusterlist_module_info;
#endif

extern struct cerebro_clusterlist_module_info hostsfile_clusterlist_module_info;
extern struct cerebro_clusterlist_module_info none_clusterlist_module_info;

/*
 * Module arrays
 */
extern struct cerebrod_config_module_info *static_config_modules[];
extern struct cerebro_clusterlist_module_info *static_clusterlist_modules[];

/*
 * cerebrod_find_static_config_module
 *
 * Returns pointer to cerebrod_config_module_info structure with the
 * name, NULL if one is not found
 */
struct cerebrod_config_module_info *cerebrod_find_static_config_module(char *name);

/*
 * cerebrod_find_static_clusterlist_module
 *
 * Returns pointer to cerebro_clusterlist_module_info structure with
 * the name, NULL if one is not found
 */
struct cerebro_clusterlist_module_info *cerebrod_find_static_clusterlist_module(char *name);

#endif /* WITH_STATIC_MODULES */

#endif /* _CEREBROD_STATIC_MODULES_H */
