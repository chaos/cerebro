/*****************************************************************************\
 *  $Id: cerebrod_listener.h,v 1.6 2005-03-15 23:14:39 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_LISTENER_H
#define _CEREBROD_LISTENER_H

#include <pthread.h>
#include "cerebrod.h"

struct cerebrod_node_data
  {
    u_int32_t starttime;
    u_int32_t boottime;
    u_int32_t last_received;
    pthread_mutex_t node_data_lock;
  };

#define CEREBROD_LISTENER_REINITIALIZE_WAIT 2

void *cerebrod_listener(void *);

#endif /* _CEREBROD_LISTENER_H */
