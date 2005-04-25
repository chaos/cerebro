/*****************************************************************************\
 *  $Id: cerebrod_dynamic_modules.h,v 1.4 2005-04-25 15:33:05 achu Exp $
\*****************************************************************************/
 
#ifndef _CEREBROD_STATIC_MODULES_H
#define _CEREBROD_STATIC_MODULES_H
 
#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if !WITH_STATIC_MODULES

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
 * cerebrod_lookup_module
 *
 * search a directory and find a module from a list of modules
 *
 * - search_dir - directory to search
 * - modules_list - list of modules to search for
 * - modules_list_len - length of list
 * - load_module - function to call when a module is found
 *
 * Returns 1 when a module is found, 0 when one is not.
 */
int cerebrod_lookup_module(char *search_dir,
			   char **modules_list,
			   int modules_list_len,
			   Cerebrod_load_module load_module);

/*
 * cerebrod_search_for_module
 *
 * search a directory for a new module, one currently unknown to
 * cerebrod
 *
 * - search_dir - directory to search
 * - signature - filename signature indicating it is a module we want
     to try and load
 * - load_module - function to call when a module is found
 *
 * Returns 1 when a module is found, 0 when one is not.
 */
int cerebrod_search_for_module(char *search_dir,
			       char *signature,
			       Cerebrod_load_module load_module);
#endif /* !WITH_STATIC_MODULES */

#endif /* _CEREBROD_STATIC_MODULES_H */
