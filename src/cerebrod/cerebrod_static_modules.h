/*****************************************************************************\
 *  $Id: cerebrod_static_modules.h,v 1.2 2005-03-22 20:56:40 achu Exp $
\*****************************************************************************/
 
#ifndef _CEREBROD_STATIC_MODULES_H
#define _CEREBROD_STATIC_MODULES_H
 
#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if WITH_STATIC_MODULES

#include "cerebrod_config.h"
#include "cerebrod_clusterlist.h"

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
extern struct cerebrod_clusterlist_module_info gendersllnl_clusterlist_module_info;
#endif

#if WITH_GENDERS
extern struct cerebrod_clusterlist_module_info genders_clusterlist_module_info;
#endif

extern struct cerebrod_clusterlist_module_info hostsfile_clusterlist_module_info;
extern struct cerebrod_clusterlist_module_info none_clusterlist_module_info;

/*
 * Module arrays
 */
extern struct cerebrod_config_module_info *static_config_modules[];
extern struct cerebrod_clusterlist_module_info *static_clusterlist_modules[];

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
 * Returns pointer to cerebrod_clusterlist_module_info structure with
 * the name, NULL if one is not found
 */
struct cerebrod_clusterlist_module_info *cerebrod_find_static_clusterlist_module(char *name);

#endif /* WITH_STATIC_MODULES */

#endif /* _CEREBROD_STATIC_MODULES_H */
