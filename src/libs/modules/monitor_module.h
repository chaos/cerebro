/*****************************************************************************\
 *  $Id: monitor_module.h,v 1.14 2010-02-02 01:01:20 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2018 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2005-2007 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <https://github.com/chaos/cerebro>.
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

#ifndef _MONITOR_MODULE_H
#define _MONITOR_MODULE_H

typedef struct monitor_module *monitor_modules_t;

/*
 * monitor_modules_load
 *
 * Find and load the monitor modules.
 *
 * Returns monitor module handle on success, NULL on error
 */
monitor_modules_t monitor_modules_load(void);

/*
 * monitor_modules_unload
 *
 * Destroy/Unload the monitor module specified by the handle
 *
 * Returns 0 on success, -1 on error
 */
int monitor_modules_unload(monitor_modules_t handle);

/*
 * monitor_modules_count
 *
 * Return number of monitoring modules loaded, -1 on error
 */
int monitor_modules_count(monitor_modules_t handle);

/*
 * monitor_module_name
 *
 * Return monitor module name
 */
char *monitor_module_name(monitor_modules_t handle, unsigned int index);

/*
 * monitor_module_interface_version
 *
 * Return monitor interface version
 */
int monitor_module_interface_version(monitor_modules_t handle, unsigned int index);

/*
 * monitor_module_setup
 *
 * call monitor module setup function
 */
int monitor_module_setup(monitor_modules_t handle, unsigned int index);

/*
 * monitor_module_cleanup
 *
 * call monitor module cleanup function
 */
int monitor_module_cleanup(monitor_modules_t handle, unsigned int index);

/*
 * monitor_module_metric_names
 *
 * call monitor module metric_names function
 */
char *monitor_module_metric_names(monitor_modules_t handle, unsigned int index);

/*
 * monitor_module_metric_update
 *
 * call monitor module metric_update function
 */
int monitor_module_metric_update(monitor_modules_t handle,
                                 unsigned int index,
                                 const char *nodename,
                                 const char *metric_name,
                                 unsigned int metric_value_type,
                                 unsigned int metric_value_len,
                                 void *metric_value);

#endif /* _MONITOR_MODULE_H */
