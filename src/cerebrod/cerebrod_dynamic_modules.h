/*****************************************************************************\
 *  $Id: cerebrod_dynamic_modules.h,v 1.1 2005-03-22 20:56:40 achu Exp $
\*****************************************************************************/
 
#ifndef _CEREBROD_STATIC_MODULES_H
#define _CEREBROD_STATIC_MODULES_H
 
#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if !WITH_STATIC_MODULES

#include "cerebrod_config.h"
#include "cerebrod_clusterlist.h"

extern char *dynamic_config_modules[];
extern int dynamic_config_modules_len;

extern char *dynamic_clusterlist_modules[];
extern int dynamic_clusterlist_modules_len;

/*
 * Cerebrod_load_module
 *
 * function prototype for loading a module. Passed a module
 * file to load.
 *
 * Returns 1 on loading success, 0 on loading failure, -1 on fatal error
 */
typedef int (*Cerebrod_load_module)(char *);

/*
 * cerebrod_search_dir_for_module
 *
 * search a directory for a module
 *
 * - search_dir - directory to search
 * - modules_list - list of modules to search for
 * - modules_list_len - length of list
 * - load_module - function to call when module is found
 *
 * Returns 1 when a module is found, 0 when one is not.
 */
int cerebrod_search_dir_for_module(char *search_dir,
                                   char **modules_list,
                                   int modules_list_len,
                                   Cerebrod_load_module load_module);
#endif /* !WITH_STATIC_MODULES */

#endif /* _CEREBROD_STATIC_MODULES_H */
