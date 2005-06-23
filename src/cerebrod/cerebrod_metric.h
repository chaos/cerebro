/*****************************************************************************\
 *  $Id: cerebrod_metric.h,v 1.6 2005-06-23 21:37:22 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_METRIC_H
#define _CEREBROD_METRIC_H

#include "list.h"
 
/*
 * struct cerebrod_metric_name_evaluation_data
 *
 * Holds data for callback functions
 */
struct cerebrod_metric_name_evaluation_data
{
  int client_fd;
  List metric_name_responses;
};

/*
 * struct cerebrod_node_metric_evaluation_data
 *
 * Holds data for callback functions
 */
struct cerebrod_node_metric_evaluation_data
{
  int client_fd;
  struct cerebro_metric_request *req;
  u_int32_t time_now;
  char *metric_name;
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
