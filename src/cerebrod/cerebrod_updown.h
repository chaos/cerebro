/*****************************************************************************\
 *  $Id: cerebrod_updown.h,v 1.2 2005-03-16 00:25:13 achu Exp $
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
void cerebrod_updown_update_data(char *node, u_int32_t last_received);

#endif /* _CEREBROD_UPDOWN_H */
