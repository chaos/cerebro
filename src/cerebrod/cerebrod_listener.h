/*****************************************************************************\
 *  $Id: cerebrod_listener.h,v 1.3 2005-02-02 01:27:51 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_LISTENER_H
#define _CEREBROD_LISTENER_H

#include "cerebrod.h"

struct cerebrod_node_data
  {
    u_int32_t starttime;
    u_int32_t boottime;
    u_int32_t last_recveived_heartbeat_time;
  };

#define CEREBROD_NODE_DATA_LEN  (sizeof(u_int32_t) \
                                 + sizeof(u_int32_t) \
                                 + sizeof(u_int32_t))

#define CEREBROD_REINITIALIZE_WAIT 2

void *cerebrod_listener(void *);

#endif /* _CEREBROD_LISTENER_H */
