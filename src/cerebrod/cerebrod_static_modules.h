/*****************************************************************************\
 *  $Id: cerebrod_static_modules.h,v 1.1 2005-03-22 02:52:56 achu Exp $
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

#endif /* WITH_STATIC_MODULES */

#endif /* _CEREBROD_STATIC_MODULES_H */
