/*****************************************************************************\
 *  $Id: cerebrod_listener_data.h,v 1.1 2005-06-21 17:02:22 achu Exp $
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

#include "cerebro/cerebrod_heartbeat_protocol.h"

#include "hash.h"

/* 
 * struct cerebrod_metric_data
 *
 * Contains metric data for a node
 */
struct cerebrod_metric_data
{
  char *metric_name;
  u_int32_t last_received_time;
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
  int metric_data_count;
};

/*
 * struct cerebrod_metric_monitor
 *
 * contains cerebrod monitor module metric information
 */
struct cerebrod_metric_monitor
{
  char *metric_name;
  int index;
  pthread_mutex_t monitor_lock;
};

/* 
 * cerebrod_listener_data_initialize
 *
 * Initialize cluster_data structures
 */
void cerebrod_listener_data_initialize(void);

/* 
 * cerebrod_listener_data_update
 *
 * Update cluster_data with information from a heartbeat
 */
void cerebrod_listener_data_update(char *nodename,
                                   struct cerebrod_heartbeat *hb,
                                   u_int32_t received_time);

#endif /* _CEREBROD_LISTENER_DATA_H */
