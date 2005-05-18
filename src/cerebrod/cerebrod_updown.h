/*****************************************************************************\
 *  $Id: cerebrod_updown.h,v 1.10 2005-05-18 00:53:53 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_UPDOWN_H
#define _CEREBROD_UPDOWN_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_PTHREAD_H
#include <pthread.h>
#endif /* HAVE_PTHREAD_H */

#include "list.h"

#define CEREBROD_UPDOWN_REINITIALIZE_WAIT 2

/*
 * struct cerebrod_updown_node_data
 *
 * contains cerebrod updown node data
 */
struct cerebrod_updown_node_data
{
  char *nodename;
  int discovered;
  u_int32_t last_received_time;
  pthread_mutex_t updown_node_data_lock;
};

/* 
 * struct cerebrod_updown_evaluation_data
 *
 * Holds data for callback function when evaluating updown state.
 */
struct cerebrod_updown_evaluation_data
{
  int client_fd;
  u_int32_t updown_request;
  u_int32_t timeout_len;
  u_int32_t time_now;
  List node_responses;
};

/*
 * cerebrod_updown
 *
 * Runs the cerebrod updown server thread
 *
 * Passed no argument
 *
 * Executed in detached state, no return value.
 */
void *cerebrod_updown(void *);

/* 
 * cerebrod_updown_update_data
 *
 * Update updown server with received time for a specific cluster node
 */
void cerebrod_updown_update_data(char *nodename, u_int32_t received_time);

#endif /* _CEREBROD_UPDOWN_H */
