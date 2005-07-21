/*****************************************************************************\
 *  $Id: cerebrod_speaker_data.h,v 1.8 2005-07-21 20:15:45 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_SPEAKER_DATA_H
#define _CEREBROD_SPEAKER_DATA_H

#include <sys/types.h>
 
#include "cerebro/cerebro_metric_module.h"
#include "cerebro/cerebrod_heartbeat_protocol.h"

/* 
 * Flags to define where a metric comes from.  Either it is monitored
 * by default by Cerebro, comes from a metric module, or has come from
 * userspace from a tool or lib calls.
 */
#define CEREBROD_METRIC_ORIGIN_DEFAULT    0x00000001
#define CEREBROD_METRIC_ORIGIN_MODULE     0x00000002
#define CEREBROD_METRIC_ORIGIN_USERSPACE  0x00000004

/*
 * struct cerebrod_speaker_metric_info
 *
 * contains cerebrod metric information
 */
struct cerebrod_speaker_metric_info
{
  char *metric_name;
  u_int32_t metric_origin;
  u_int32_t next_call_time;

  /* For Metric Modules */
  int metric_period;
  int index;

  /* For Metrics from userspace */
  u_int32_t metric_value_type;
  u_int32_t metric_value_len;
  void *metric_value;
};

/*
 * cerebrod_speaker_data_initialize
 *
 * Initialize speaker_data structures
 */
void cerebrod_speaker_data_initialize(void);

/* 
 * cerebrod_speaker_data_metric_list_sort
 *
 * Sort the metric list
 */
void cerebrod_speaker_data_metric_list_sort(void);

/*
 * cerebrod_speaker_data_get_metric_data
 *
 * Store metric data into the heartbeat
 */
void cerebrod_speaker_data_get_metric_data(struct cerebrod_heartbeat *hb, 
                                           unsigned int *heartbeat_len);

#endif /* _CEREBROD_SPEAKER_DATA_H */
