/*****************************************************************************\
 *  $Id: metric_module.h,v 1.8 2006-10-29 19:02:13 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2005 The Regents of the University of California.
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
 *  with Genders; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
\*****************************************************************************/

#ifndef _METRIC_MODULE_H
#define _METRIC_MODULE_H

typedef struct metric_module *metric_modules_t; 

/*
 * metric_modules_load
 *
 * Find and load the metric modules.
 * 
 * Returns metric module handle on success, NULL on error
 */
metric_modules_t metric_modules_load(void);

/*
 * metric_modules_unload
 *
 * Destroy/Unload the metric module specified by the handle
 *
 * Returns 0 on success, -1 on error
 */
int metric_modules_unload(metric_modules_t handle);

/*
 * metric_modules_count
 *
 * Return number of metricing modules loaded, -1 on error
 */
int metric_modules_count(metric_modules_t handle);

/*
 * metric_module_name
 *
 * Return metric module name
 */
char *metric_module_name(metric_modules_t handle, unsigned int index);

/*
 * metric_module_setup
 *
 * call metric module setup function
 */
int metric_module_setup(metric_modules_t handle, unsigned int index);

/*
 * metric_module_cleanup
 *
 * call metric module cleanup function
 */
int metric_module_cleanup(metric_modules_t handle, unsigned int index);

/*
 * metric_module_get_metric_name
 *
 * call metric module get_metric_name function
 */
char *metric_module_get_metric_name(metric_modules_t handle, unsigned int index);

/*
 * metric_module_get_metric_period
 *
 * call metric module get_metric_period function
 */
int metric_module_get_metric_period(metric_modules_t handle, 
                                    unsigned int index,
                                    int *period);

/*
 * metric_module_get_metric_value
 *
 * call metric module get_metric_value function
 */
int metric_module_get_metric_value(metric_modules_t handle,
				   unsigned int index,
				   unsigned int *metric_value_type,
				   unsigned int *metric_value_len,
				   void **metric_value);

/*
 * metric_module_destroy_metric_value
 *
 * call metric module destroy_metric_value function
 */
int metric_module_destroy_metric_value(metric_modules_t handle,
				       unsigned int index,
				       void *metric_value);

/*
 * metric_module_get_metric_thread
 *
 * call metric module get_metric_thread function
 *
 * Returns 0 on success, -1 on error
 */
Cerebro_metric_thread_pointer metric_module_get_metric_thread(metric_modules_t handle,
                                                              unsigned int index);

/*
 * metric_module_send_heartbeat_function_pointer
 *
 * call metric module send_heartbeat_function_pointer function
 *
 * Returns 0 on success, -1 on error
 */
int metric_module_send_heartbeat_function_pointer(metric_modules_t handle,
						  unsigned int index,
						  Cerebro_metric_send_heartbeat function_pointer);

#endif /* _METRIC_MODULE_H */
