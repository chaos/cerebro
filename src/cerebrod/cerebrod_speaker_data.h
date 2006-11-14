/*****************************************************************************\
 *  $Id: cerebrod_speaker_data.h,v 1.12.4.1 2006-11-14 04:16:05 chu11 Exp $
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

#ifndef _CEREBROD_SPEAKER_DATA_H
#define _CEREBROD_SPEAKER_DATA_H

#include <sys/types.h>
 
#include "cerebro/cerebro_metric_module.h"
#include "cerebro/cerebrod_heartbeat_protocol.h"

/* 
 * Flags to define where a metric comes from.  Either it is monitored
 * by default by Cerebro, comes from a metric module, or has come from
 * userspace from a tool or lib.
 */
#define CEREBROD_METRIC_SPEAKER_ORIGIN_DEFAULT    0x00000001
#define CEREBROD_METRIC_SPEAKER_ORIGIN_MODULE     0x00000002
#define CEREBROD_METRIC_SPEAKER_ORIGIN_USERSPACE  0x00000004

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
  u_int32_t metric_flags;
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
