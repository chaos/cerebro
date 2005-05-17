/*****************************************************************************\
 *  $Id: cerebrod_status.h,v 1.2 2005-05-17 20:53:59 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_STATUS_H
#define _CEREBROD_STATUS_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_PTHREAD_H
#include <pthread.h>
#endif /* HAVE_PTHREAD_H */

#include "list.h"

#define CEREBROD_STATUS_REINITIALIZE_WAIT 2

/*
 * struct cerebrod_status_node_data
 *
 * contains cerebrod status node data
 */
struct cerebrod_status_node_data
{
  char *nodename;
  u_int32_t starttime;
  u_int32_t boottime;
  pthread_mutex_t status_node_data_lock;
};

/*
 * cerebrod_status
 *
 * Runs the cerebrod status server thread
 *
 * Passed no argument
 *
 * Executed in detached state, no return value.
 */
void *cerebrod_status(void *);

/* 
 * cerebrod_status_update_data
 *
 * Update status server with last_received time for a specific cluster
 * node
 */
void cerebrod_status_update_data(char *nodename, u_int32_t last_received);

#endif /* _CEREBROD_STATUS_H */
