/*****************************************************************************\
 *  $Id: cerebrod_node_data.h,v 1.5 2005-05-25 17:04:07 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_NODE_DATA_H
#define _CEREBROD_NODE_DATA_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_PTHREAD_H
#include <pthread.h>
#endif /* HAVE_PTHREAD_H */

#include <sys/types.h>

#include "cerebro/cerebro_metric_protocol.h"

#include "cerebro/cerebrod_heartbeat_protocol.h"

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
  cerebro_metric_type_t metric_type;
  cerebro_metric_value_t metric_value;
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
 * cerebrod_node_data_initialize
 *
 * Initialize node_data structures
 */
void cerebrod_node_data_initialize(void);

/* 
 * cerebrod_node_data_update
 *
 * Update node_data with more up to date information
 */
void cerebrod_node_data_update(char *nodename,
                               struct cerebrod_heartbeat *hb,
                               u_int32_t received_time);

#endif /* _CEREBROD_NODE_DATA_H */
