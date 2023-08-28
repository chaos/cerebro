/*****************************************************************************\
 *  $Id: event_module.h,v 1.9 2010-02-02 01:01:20 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2018 Lawrence Livermore National Security, LLC.
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

#ifndef _EVENT_MODULE_H
#define _EVENT_MODULE_H

#include "cerebro/cerebro_event_protocol.h"

typedef struct event_module *event_modules_t;

/*
 * event_modules_load
 *
 * Find and load the event modules.
 *
 * Returns event module handle on success, NULL on error
 */
event_modules_t event_modules_load(void);

/*
 * event_modules_unload
 *
 * Destroy/Unload the event module specified by the handle
 *
 * Returns 0 on success, -1 on error
 */
int event_modules_unload(event_modules_t handle);

/*
 * event_modules_count
 *
 * Return number of eventing modules loaded, -1 on error
 */
int event_modules_count(event_modules_t handle);

/*
 * event_module_name
 *
 * Return event module name
 */
char *event_module_name(event_modules_t handle, unsigned int index);

/*
 * event_module_interface_version
 *
 * Return event interface version
 */
int event_module_interface_version(event_modules_t handle, unsigned int index);

/*
 * event_module_setup
 *
 * call event module setup function
 */
int event_module_setup(event_modules_t handle, unsigned int index);

/*
 * event_module_cleanup
 *
 * call event module cleanup function
 */
int event_module_cleanup(event_modules_t handle, unsigned int index);

/*
 * event_module_event_names
 *
 * call event module event_names function
 */
char *event_module_event_names(event_modules_t handle, unsigned int index);

/*
 * event_module_metric_names
 *
 * call event module metric_names function
 */
char *event_module_metric_names(event_modules_t handle, unsigned int index);

/*
 * event_module_timeout_length
 *
 * call event module timeout_length function
 */
int event_module_timeout_length(event_modules_t handle, unsigned int index);

/*
 * event_module_node_timeout
 *
 * call event module node_timeout function
 */
int event_module_node_timeout(event_modules_t handle,
                              unsigned int index,
                              const char *nodename,
                              struct cerebro_event **event);

/*
 * event_module_metric_update
 *
 * call event module metric_update function
 */
int event_module_metric_update(event_modules_t handle,
                               unsigned int index,
                               const char *nodename,
                               const char *metric_name,
                               unsigned int metric_value_type,
                               unsigned int metric_value_len,
                               void *metric_value,
                               struct cerebro_event **event);

/*
 * event_module_destroy
 *
 * call event module destroy function
 */
void event_module_destroy(event_modules_t handle,
                          unsigned int index,
                          struct cerebro_event *event);

#endif /* _EVENT_MODULE_H */
