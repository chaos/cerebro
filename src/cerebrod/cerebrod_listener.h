/*****************************************************************************\
 *  $Id: cerebrod_listener.h,v 1.8 2005-04-27 18:11:35 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_LISTENER_H
#define _CEREBROD_LISTENER_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */
 
#if HAVE_PTHREAD_H
#include <pthread.h>
#endif /* HAVE_PTHREAD_H */

#include "cerebrod.h"

#define CEREBROD_LISTENER_REINITIALIZE_WAIT 2

/*  
 * struct cerebrod_node_data
 *
 * contains cerebrod listener node data
 */
struct cerebrod_node_data
  {
    u_int32_t starttime;
    u_int32_t boottime;
    u_int32_t last_received;
    pthread_mutex_t node_data_lock;
  };

/* 
 * cerebrod_listener
 *
 * Runs a cerebrod listening thread
 *
 * Passed no argument
 * 
 * Executed in detached state, no return value.
 */
void *cerebrod_listener(void *);

#endif /* _CEREBROD_LISTENER_H */
