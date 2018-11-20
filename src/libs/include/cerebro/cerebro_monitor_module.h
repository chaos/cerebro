/*****************************************************************************\
 *  $Id: cerebro_monitor_module.h,v 1.12 2010-02-02 01:01:20 chu11 Exp $
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

#ifndef _CEREBRO_MONITOR_MODULE_H
#define _CEREBRO_MONITOR_MODULE_H

#define CEREBRO_MONITOR_INTERFACE_VERSION 1

/* 
 * Cerebro_monitor_interface_version
 *
 * function prototype for monitor module function to return the
 * current monitor interface version.  Should always return
 * current value of macro CEREBRO_MONITOR_INTERFACE_VERSION.
 *
 * Returns version number on success, -1 one error
 */
typedef int (*Cerebro_monitor_interface_version)(void);

/*
 * Cerebro_monitor_setup
 *
 * function prototype for monitor module function to setup the
 * module.  Required to be defined by each monitor module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_monitor_setup)(void);

/*
 * Cerebro_monitor_cleanup
 *
 * function prototype for monitor module function to cleanup.  Required
 * to be defined by each monitor module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_monitor_cleanup)(void);

/*
 * Cerebro_monitor_metric_names
 *
 * function prototype for monitor module to return the metric name(s)
 * this module wishes to monitor.  Separate metric names are comma
 * separated.
 *
 * Returns metric name(s) on success, -1 on error
 */
typedef char *(*Cerebro_monitor_metric_names)(void);

/*
 * Cerebro_monitor_metric_update
 *
 * function prototype for monitor module function to be updated with a
 * new metric value.  Required to be defined by each monitor module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_monitor_metric_update)(const char *nodename,
                                             const char *metric_name,
                                             unsigned int metric_value_type,
                                             unsigned int metric_value_len,
                                             void *metric_value);

/*
 * struct cerebro_monitor_module_info 
 * 
 * contains monitor module information and operations.  Required to be
 * defined in each monitor module.
 */
struct cerebro_monitor_module_info
{
  char *monitor_module_name;
  Cerebro_monitor_interface_version interface_version;
  Cerebro_monitor_setup setup;
  Cerebro_monitor_cleanup cleanup;
  Cerebro_monitor_metric_names metric_names;
  Cerebro_monitor_metric_update metric_update;
};

#endif /* _CEREBRO_MONITOR_MODULE_H */
