/*****************************************************************************\
 *  $Id: cerebrod_metric.h,v 1.3 2005-05-25 22:24:33 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_METRIC_H
#define _CEREBROD_METRIC_H

#include "list.h"
 
/*
 * struct cerebrod_metric_evaluation_data
 *
 * Holds data for callback functions
 */
struct cerebrod_metric_evaluation_data
{
  int client_fd;
  struct cerebro_metric_request *req;
  u_int32_t time_now;
  List node_responses;
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

#endif /* _CEREBROD_METRIC_H */
