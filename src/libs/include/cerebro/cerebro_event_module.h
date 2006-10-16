/*****************************************************************************\
 *  $Id: cerebro_event_module.h,v 1.1.2.1 2006-10-16 14:50:21 chu11 Exp $
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

#ifndef _CEREBRO_EVENT_MODULE_H
#define _CEREBRO_EVENT_MODULE_H

#include <cerebro/cerebro_event_protocol.h>

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
 * Cerebro_event_metric_names
 *
 * function prototype for event module to return the metric name(s)
 * this module wishes to event.  Separate metric names are comma
 * separated.
 *
 * Returns metric name on success, -1 on error
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
                                          struct cerebro_event *event);

/*
 * Cerebro_event_metric_data
 *
 * function prototype for event module function to be given updated
 * metric data.  Required to be defined by each event module.  If
 * necessary, will return event data to be sent to listeners.
 *
 * Returns 1 if event generated, 0 if not, -1 on error
 */
typedef int (*Cerebro_event_metric_data)(const char *nodename,
                                         const char *metric_name,
                                         unsigned int metric_value_type,
                                         unsigned int metric_value_len,
                                         void *metric_value,
                                         struct cerebro_event *event);

/*
 * struct cerebro_event_module_info 
 * 
 * contains event module information and operations.  Required to be
 * defined in each event module.
 */
struct cerebro_event_module_info
{
  char *event_module_name;
  Cerebro_event_setup setup;
  Cerebro_event_cleanup cleanup;
  Cerebro_event_metric_names metric_names;
  Cerebro_event_timeout_length timeout_length;
  Cerebro_event_node_timeout node_timeout;
  Cerebro_event_metric_data metric_data;
};

#endif /* _CEREBRO_EVENT_MODULE_H */
