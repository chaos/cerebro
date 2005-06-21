/*****************************************************************************\
 *  $Id: cerebrod_speaker_data.h,v 1.2 2005-06-21 20:49:02 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_SPEAKER_DATA_H
#define _CEREBROD_SPEAKER_DATA_H

#include <sys/types.h>
 
#include "cerebro/cerebrod_heartbeat_protocol.h"

/*
 * struct cerebrod_speaker_metric_module
 *
 * contains cerebrod metric information
 */
struct cerebrod_speaker_metric_module
{
  char *metric_name;
  u_int32_t metric_period;
  int index;
  u_int32_t next_call_time;
};

/*
 * cerebrod_speaker_data_initialize
 *
 * Initialize speaker_data structures
 */
void cerebrod_speaker_data_initialize(void);

/*
 * cerebrod_speaker_data_get_metric_data
 *
 * Store metric data into the heartbeat
 */
void cerebrod_speaker_data_get_metric_data(struct cerebrod_heartbeat *hb, 
                                           unsigned int *heartbeat_len);

#endif /* _CEREBROD_SPEAKER_DATA_H */
