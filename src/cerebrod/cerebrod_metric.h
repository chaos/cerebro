/*****************************************************************************\
 *  $Id: cerebrod_metric.h,v 1.1 2005-05-19 16:40:40 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_METRIC_H
#define _CEREBROD_METRIC_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_PTHREAD_H
#include <pthread.h>
#endif /* HAVE_PTHREAD_H */

#include <sys/types.h>

#include "cerebro/cerebrod_heartbeat_protocol.h"

#include "hash.h"

#define CEREBROD_METRIC_REINITIALIZE_WAIT 2

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
  CEREBROD_METRIC_TYPE_INT32_T = 1,
  CEREBROD_METRIC_TYPE_U_INT32_T = 2,
} cerebrod_metric_type_t;

/* 
 * cerebrod_metric_type_t
 *
 * type for metric value
 */
typedef union {
  int32_t   val_int32;
  u_int32_t val_u_int32;
} cerebrod_metric_value_t;

/* 
 * struct cerebrod_metric_data
 *
 * Contains a single piece of metric data
 */
struct cerebrod_metric_data
{
  char *metric_name;
  cerebrod_metric_type_t metric_type;
  cerebrod_metric_value_t metric_value;
};

/*
 * cerebrod_metric
 *
 * Runs the cerebrod metric server thread
 *
 * Passed no argument
 *
 * Executed in detached state, no return value.
 */
void *cerebrod_metric(void *);

/* 
 * cerebrod_metric_update_data
 *
 * Update metric server with metric data
 */
void cerebrod_metric_update_data(char *nodename, 
                                 struct cerebrod_heartbeat *hb,
                                 u_int32_t received_time);

#endif /* _CEREBROD_METRIC_H */
