/*****************************************************************************\
 *  $Id: cerebrod_updown.h,v 1.1 2005-03-15 23:14:39 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_UPDOWN_H
#define _CEREBROD_UPDOWN_H

#include <pthread.h>
#include "cerebrod.h"

struct cerebrod_updown_node_data
{
  char *node;
  int discovered;
  u_int32_t last_received;
  pthread_mutex_t updown_node_data_lock;
};

#define CEREBROD_UPDOWN_REINITIALIZE_WAIT 2

void *cerebrod_updown(void *);

#endif /* _CEREBROD_UPDOWN_H */
