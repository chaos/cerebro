/*****************************************************************************\
 *  $Id: cerebrod_listener.h,v 1.5 2005-02-10 00:58:40 achu Exp $
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

#define CEREBROD_NODE_DATA_LEN  (sizeof(u_int32_t) \
                                 + sizeof(u_int32_t) \
                                 + sizeof(u_int32_t))

#define CEREBROD_REINITIALIZE_WAIT 2

void *cerebrod_listener(void *);

#endif /* _CEREBROD_LISTENER_H */
