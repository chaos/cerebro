/*****************************************************************************\
 *  $Id: module_util.h,v 1.2 2005-06-27 23:27:06 achu Exp $
\*****************************************************************************/

#ifndef _MODULE_UTIL_H
#define _MODULE_UTIL_H

/*
 * Module_callback
 *
 * function prototype for checking and storing module info. Passed a
 * module handle, dl_handle and module_info.
 *
 * Returns 1 on success, 0 on failure, -1 on fatal error
 */
typedef int (*Module_callback)(void *handle, void *dl_handle, void *module_info);

/*
 * find_and_load_modules
 *
 * Find and load modules
 *
 * Returns 1 if modules are loaded, 0 if not, -1 on error
 */
int find_and_load_modules(char *module_dir,
                          char **modules_list,
                          int modules_list_len,
                          char *signature,
                          Module_callback module_cb,
                          char *module_info_sym,
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
