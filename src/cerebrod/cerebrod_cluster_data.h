/*****************************************************************************\
 *  $Id: cerebrod_cluster_data.h,v 1.1 2005-06-10 22:05:24 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_CLUSTER_DATA_H
#define _CEREBROD_CLUSTER_DATA_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_PTHREAD_H
#include <pthread.h>
#endif /* HAVE_PTHREAD_H */

#include <sys/types.h>

#include "cerebro_metric_protocol.h"
#include "cerebrod_heartbeat_protocol.h"

#include "hash.h"

/* 
 * struct cerebrod_metric_data
 *
 * Contains a single piece of metric data
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
  hash_t metric_data;
  int metric_data_count;
  u_int32_t last_received_time;
  pthread_mutex_t node_data_lock;
};

/* 
 * cerebrod_cluster_data_initialize
 *
 * Initialize cluster_data structures
 */
void cerebrod_cluster_data_initialize(void);

/* 
 * cerebrod_cluster_data_update
 *
 * Update cluster_data with more up to date information
 */
void cerebrod_cluster_data_update(char *nodename,
                                  struct cerebrod_heartbeat *hb,
                                  u_int32_t received_time);

#endif /* _CEREBROD_CLUSTER_DATA_H */
