/*****************************************************************************\
 *  $Id: cerebrod_listener_data.h,v 1.15 2007-10-16 22:43:15 chu11 Exp $
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
 *  with Cerebro; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
\*****************************************************************************/

#ifndef _CEREBROD_LISTENER_DATA_H
#define _CEREBROD_LISTENER_DATA_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_PTHREAD_H
#include <pthread.h>
#endif /* HAVE_PTHREAD_H */

#include <sys/types.h>

#include "cerebro/cerebrod_message_protocol.h"

#include "hash.h"

/* 
 * Flags to define if a metric is a default metric, or something
 * the listener received.
 */
#define CEREBROD_METRIC_LISTENER_ORIGIN_DEFAULT   0x00000001
#define CEREBROD_METRIC_LISTENER_ORIGIN_MONITORED 0x00000002

/* 
 * struct cerebrod_metric_data
 *
 * Contains metric data for a node
 */
struct cerebrod_metric_data
{
  char *metric_name;
  u_int32_t metric_value_received_time;
  u_int32_t metric_value_type;
  u_int32_t metric_value_len;
  void *metric_value;
};

/*
 * struct cerebrod_node_data
 *
 * contains cerebrod node data
 */
struct cerebrod_node_data
{
  char *nodename;
  int discovered;
  u_int32_t last_received_time;
  pthread_mutex_t node_data_lock;
  hash_t metric_data;
  int metric_data_size;
  int metric_data_count;
};

/*
 * struct cerebrod_event_module
 *
 * contains cerebrod event module metric information
 */
struct cerebrod_event_module
{
  char *metric_names;
  char *event_names;
  int index;
  pthread_mutex_t event_lock;
};

/* 
 * struct cerebrod_metric_name_data
 *
 * contains metric name and origin
 */
struct cerebrod_metric_name_data
{
  char *metric_name;
  u_int32_t metric_origin;
};

/* 
 * struct cerebrod_event_node_timeout
 *
 * contains information needed for timeout calculations
 */
struct cerebrod_event_node_timeout
{
  char *nodename;
  u_int32_t last_received_time;
  unsigned int timeout_occurred;
};

/* 
 * struct cerebrod_timeout_data
 *
 * Store a timeout and it's string
 */
struct cerebrod_timeout_data
{
  unsigned int timeout;
  char *timeout_str;
};

/* 
 * metric_data_create
 *
 * create metric name data
 *
 * Returns pointer on success, NULL on error
 */
struct cerebrod_metric_data *metric_data_create(const char *metric_name);

/* 
 * metric_data_destroy
 *
 * destroy metric name data
 */
void metric_data_destroy(void *data);

/* 
 * metric_name_data_create
 *
 * create metric name data
 *
 * Returns pointer on success, NULL on error
 */
struct cerebrod_metric_name_data *metric_name_data_create(const char *metric_name,
                                                          u_int32_t metric_origin);

/* 
 * metric_name_data_destroy
 *
 * destroy metric name data
 */
void metric_name_data_destroy(void *data);

/* 
 * cerebrod_listener_data_initialize
 *
 * Initialize listener_data structures
 */
void cerebrod_listener_data_initialize(void);

/* 
 * cerebrod_listener_data_update
 *
 * Update listener_data with information from a cerebrod message
 */
void cerebrod_listener_data_update(char *nodename,
                                   struct cerebrod_message *msg,
                                   u_int32_t received_time);

#endif /* _CEREBROD_LISTENER_DATA_H */
