/*****************************************************************************\
 *  $Id: cerebrod_metric_server.h,v 1.3 2005-07-08 21:59:26 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_METRIC_SERVER_H
#define _CEREBROD_METRIC_SERVER_H

#include "cerebro/cerebro_metric_server_protocol.h"

#include "list.h"
 
/*
 * struct cerebrod_metric_name_evaluation_data
 *
 * Holds data for callback functions
 */
struct cerebrod_metric_name_evaluation_data
{
  int fd;
  List responses;
};

/*
 * struct cerebrod_metric_data_evaluation_data
 *
 * Holds data for callback functions
 */
struct cerebrod_metric_data_evaluation_data
{
  int fd;
  struct cerebro_metric_server_request *req;
  u_int32_t time_now;
  char *metric_name;
  List responses;
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
void *cerebrod_metric_server(void *);

#endif /* _CEREBROD_METRIC_SERVER_H */
