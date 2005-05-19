/*****************************************************************************\
 *  $Id: cerebrod_node_data.h,v 1.3 2005-05-19 22:21:10 achu Exp $
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

#include "cerebro/cerebrod_heartbeat_protocol.h"

#include "hash.h"

/*
 * struct cerebrod_metric_node_data
 *
 * contains cerebrod metric node data
 */
struct cerebrod_metric_node_data
{
  char *nodename;
  hash_t metric_data;
  int metric_data_count;
  u_int32_t last_received_time;
  pthread_mutex_t metric_node_data_lock;
};

/* 
 * cerebrod_metric_type_t
 *
 * type for metric type
 */
typedef enum {
  CEREBROD_METRIC_TYPE_BOOL = 0,
  CEREBROD_METRIC_TYPE_INT32 = 1,
  CEREBROD_METRIC_TYPE_UNSIGNED_INT32 = 2,
  CEREBROD_METRIC_TYPE_FLOAT = 3,
  CEREBROD_METRIC_TYPE_DOUBLE = 4,
  CEREBROD_METRIC_TYPE_STRING = 5
} cerebrod_metric_type_t;

/* 
 * cerebrod_metric_type_t
 *
 * type for metric value
 */
typedef union {
  char      val_bool;
  int32_t   val_int32;
  u_int32_t val_unsigned_int32;
  float     val_float;
  double    val_double;
  char      val_string[CEREBRO_METRIC_STRING_MAXLEN];
} cerebrod_metric_value_t;

/* 
 * struct cerebrod_metric_data
 *
 * Contains a single piece of metric data
 */
struct cerebrod_metric_data
{
  char *metric_name;
  u_int32_t last_received_time;
  cerebrod_metric_type_t metric_type;
  cerebrod_metric_value_t metric_value;
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
  hash_t metric_data;
  int metric_data_count;
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
