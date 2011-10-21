/*****************************************************************************\
 *  $Id: module_util.h,v 1.9 2010-02-02 01:01:20 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2011 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2005-2007 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <http://www.llnl.gov/linux/cerebro/>.
 *
 *  Cerebro is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  Cerebro is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with Cerebro. If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#ifndef _MODULE_UTIL_H
#define _MODULE_UTIL_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

/*
 * Module_callback
 *
 * function prototype for checking and storing module info. Passed a
 * module handle, dl_handle and module_info.
 *
 * Returns 1 on success, 0 on failure, -1 on fatal error
 */
#if WITH_STATIC_MODULES
typedef int (*Module_callback)(void *handle, void *module_info);
#else  /* !WITH_STATIC_MODULES */
typedef int (*Module_callback)(void *handle, void *dl_handle, void *module_info);
#endif /* !WITH_STATIC_MODULES */

/*
 * find_and_load_modules
 *
 * Find and load modules
 *
 * Returns 1 if modules are loaded, 0 if not, -1 on error
 */
#if WITH_STATIC_MODULES
int find_and_load_modules(void **modules_list,
                          Module_callback module_cb,
                          void *handle,
                          unsigned int modules_max);
#else /* !WITH_STATIC_MODULES */
int find_and_load_modules(char *module_dir,
                          char **modules_list,
                          char *signature,
                          Module_callback module_cb,
                          char *module_info_sym,
                          void *handle,
                          unsigned int modules_max);
#endif /* !WITH_STATIC_MODULES */

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
