/*****************************************************************************\
 *  $Id: cerebrod_event_update.h,v 1.11 2010-04-06 22:10:11 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2010 Lawrence Livermore National Security, LLC.
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
 *  with Cerebro.  If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#ifndef _CEREBROD_EVENT_UPDATE_H
#define _CEREBROD_EVENT_UPDATE_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_PTHREAD_H
#include <pthread.h>
#endif /* HAVE_PTHREAD_H */

#include <sys/types.h>

#include "cerebro/cerebrod_message_protocol.h"
#include "cerebrod_listener_data.h"

#include "list.h"

/* 
 * struct cerebrod_event_module_list
 *
 * Contains list of event module info list and a lock for thread
 * safety.
 */
struct cerebrod_event_module_list
{
  List event_list;
  pthread_mutex_t event_list_lock;
};

/*
 * struct cerebrod_event_module_info
 *
 * contains cerebrod event module metric information
 */
struct cerebrod_event_module_info
{
  char *metric_names;
  char *event_names;
  int index;
  pthread_mutex_t event_lock;
};

/* 
 * struct cerebrod_event_node_timeout_data
 *
 * contains information needed for timeout calculations
 */
struct cerebrod_event_node_timeout_data
{
  char *nodename;
  u_int32_t last_received_time;
  int timeout_occurred;
};

/* 
 * struct cerebrod_event_module_timeout_data
 *
 * Store a timeout and it's string
 */
struct cerebrod_event_module_timeout_data
{
  unsigned int timeout;
  char *timeout_str;
};

/*
 * cerebrod_event_to_send_destroy
 *
 * List destroy callback for destroying structs of type
 * cerebrod_event_to_send.
 */
void cerebrod_event_to_send_destroy(void *x);

/*
 * cerebrod_event_modules_setup
 *
 * Setup event modules.
 *
 * Return 1 if modules are loaded, 0 if not, -1 on error
 */
int cerebrod_event_modules_setup(void);

/* 
 * cerebrod_event_add_node_timeout_data
 *
 * Called to add another node timeout data element to appropriate
 * data structures if a new node was discovered.
 */
void cerebrod_event_add_node_timeout_data(struct cerebrod_node_data *nd,
                                          u_int32_t received_time);

/* 
 * cerebrod_event_update_node_received_time
 *
 * Called to update node timeout data is a cerebrod message was
 * recently received.
 */
void cerebrod_event_update_node_received_time(struct cerebrod_node_data *nd,
                                              u_int32_t received_time);

/*
 * cerebrod_event_update
 *
 * Accepts recently gathered metric data to be passed to event modules.
 */
void cerebrod_event_modules_update(const char *nodename,
                                   struct cerebrod_node_data *nd,
                                   const char *metric_name,
                                   struct cerebrod_message_metric *mm);

#endif /* _CEREBROD_EVENT_UPDATE_H */
