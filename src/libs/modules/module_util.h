/*****************************************************************************\
 *  $Id: module_util.h,v 1.1 2005-06-18 18:48:30 achu Exp $
\*****************************************************************************/

#ifndef _MODULE_UTIL_H
#define _MODULE_UTIL_H

/*
 * Module_loader
 *
 * function prototype for loading a module. Passed a module handle and
 * file/module to load.
 *
 * Returns 1 on loading success, 0 on loading failure, -1 on fatal error
 */
typedef int (*Module_loader)(void *, char *);

/*
 * find_known_module
 *
 * Try to find a known module from the modules list in the search
 * directory.
 *
 * - search_dir - directory to search
 * - modules_list - list of modules to search for
 * - modules_list_len - length of list
 * - load_module - function to call when a module is found
 * - handle - pointer to module handle
 *
 * Returns 1 if module is loaded, 0 if it isn't, -1 on fatal error
 */
int find_known_module(char *search_dir,
		      char **modules_list,
		      int modules_list_len,
		      Module_loader load_module,
		      void *handle);

/*
 * find_modules
 *
 * Search a directory for modules
 *
 * - search_dir - directory to search
 * - signature - filename signature indicating if the filename is a
 *               module we want to try and load
 * - load_module - function to call when a module is found
 * - handle - pointer to module handle
 * - modules_max - maximum modules that can be found
 *
 * Returns 1 when a module(s) are found, 0 if not, -1 on fatal error
 */
int find_modules(char *search_dir,
		 char *signature,
		 Module_loader load_module,
		 void *handle,
		 unsigned int modules_max);

/* 
 * module_setup
 *
 * Setup library for module loading
 */
int module_setup(void);

/* 
 * module_cleanup
 *
 * Cleanup library from module loading
 */
int module_cleanup(void);

#endif /* _MODULE_UTIL_H */
