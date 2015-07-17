/*****************************************************************************\
 *  $Id: cerebro_event_module.h,v 1.8 2010-02-02 01:01:20 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2015 Lawrence Livermore National Security, LLC.
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

#ifndef _CEREBRO_EVENT_MODULE_H
#define _CEREBRO_EVENT_MODULE_H

#include <cerebro/cerebro_event_protocol.h>

#define CEREBRO_EVENT_INTERFACE_VERSION 1

/* 
 * Cerebro_event_interface_version
 *
 * function prototype for event module function to return the
 * current event interface version.  Should always return
 * current value of macro CEREBRO_EVENT_INTERFACE_VERSION.
 *
 * Returns version number on success, -1 one error
 */
typedef int (*Cerebro_event_interface_version)(void);

/*
 * Cerebro_event_setup
 *
 * function prototype for event module function to setup the
 * module.  Required to be defined by each event module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_event_setup)(void);

/*
 * Cerebro_event_cleanup
 *
 * function prototype for event module function to cleanup.  Required
 * to be defined by each event module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_event_cleanup)(void);

/*
 * Cerebro_event_event_names
 *
 * function prototype for event module to return the event name(s)
 * this module may generate.  Separate event names are comma separated.
 *
 * Returns event name(s) on success, -1 on error
 */
typedef char *(*Cerebro_event_event_names)(void);

/*
 * Cerebro_event_metric_names
 *
 * function prototype for event module to return the metric name(s)
 * this module needs.  Separate metric names are comma separated.
 *
 * Returns metric name(s) on success, -1 on error
 */
typedef char *(*Cerebro_event_metric_names)(void);

/*
 * Cerebro_event_timeout_length
 *
 * function prototype for event module to return the timeout length in
 * seconds this module wishes to be notified of if a node times out.
 * Required to be defined by each event module.  Return 0 if a timeout
 * notification is not desired.
 *
 * Returns timeout on success, -1 on error
 */
typedef int (*Cerebro_event_timeout_length)(void);

/*
 * Cerebro_event_node_timeout
 *
 * function prototype for event module function to be called when a
 * node timeout occurs.  Required to be defined by each event module.
 * If necessary, will return event data to be sent to listeners.
 *
 * Returns 1 if event generated, 0 if not, -1 on error
 */
typedef int (*Cerebro_event_node_timeout)(const char *nodename,
                                          struct cerebro_event **event);

/*
 * Cerebro_event_metric_update
 *
 * function prototype for event module function to be given updated
 * metric data.  Required to be defined by each event module.  If
 * necessary, will return event data to be sent to listeners.
 *
 * Returns 1 if event generated, 0 if not, -1 on error
 */
typedef int (*Cerebro_event_metric_update)(const char *nodename,
                                           const char *metric_name,
                                           unsigned int metric_value_type,
                                           unsigned int metric_value_len,
                                           void *metric_value,
                                           struct cerebro_event **event);

/* 
 * Cerebro_event_destroy
 *
 * function prototype for event module to destroy an event generated
 * by a node_timeout of metric_update call.  Required to be defined by
 * each event module.
 */
typedef void (*Cerebro_event_destroy)(struct cerebro_event *event);

/*
 * struct cerebro_event_module_info 
 * 
 * contains event module information and operations.  Required to be
 * defined in each event module.
 */
struct cerebro_event_module_info
{
  char *event_module_name;
  Cerebro_event_interface_version interface_version;
  Cerebro_event_setup setup;
  Cerebro_event_cleanup cleanup;
  Cerebro_event_event_names event_names;
  Cerebro_event_metric_names metric_names;
  Cerebro_event_timeout_length timeout_length;
  Cerebro_event_node_timeout node_timeout;
  Cerebro_event_metric_update metric_update;
  Cerebro_event_destroy destroy;
};

#endif /* _CEREBRO_EVENT_MODULE_H */
