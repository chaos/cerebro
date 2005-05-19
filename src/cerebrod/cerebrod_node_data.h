/*****************************************************************************\
 *  $Id: cerebrod_node_data.h,v 1.1 2005-05-19 17:31:05 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_NODE_DATA_H
#define _CEREBROD_NODE_DATA_H

#if 0

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_PTHREAD_H
#include <pthread.h>
#endif /* HAVE_PTHREAD_H */

#include "list.h"

#define CEREBROD_NODE_DATA_REINITIALIZE_WAIT 2

/*
 * struct cerebrod_node_data_node_data
 *
 * contains cerebrod node_data node data
 */
struct cerebrod_node_data_node_data
{
  char *nodename;
  int discovered;
  u_int32_t last_received_time;
  pthread_mutex_t node_data_node_data_lock;
};

/* 
 * struct cerebrod_node_data_evaluation_data
 *
 * Holds data for callback function when evaluating node_data state.
 */
struct cerebrod_node_data_evaluation_data
{
  int client_fd;
  u_int32_t node_data_request;
  u_int32_t timeout_len;
  u_int32_t time_now;
  List node_responses;
};

/*
 * cerebrod_node_data
 *
 * Runs the cerebrod node_data server thread
 *
 * Passed no argument
 *
 * Executed in detached state, no return value.
 */
void *cerebrod_node_data(void *);

/* 
 * cerebrod_node_data_update_data
 *
 * Update node_data server with received time for a specific cluster node
 */
void cerebrod_node_data_update_data(char *nodename, u_int32_t received_time);

#endif /* 0 */

#endif /* _CEREBROD_NODE_DATA_H */
