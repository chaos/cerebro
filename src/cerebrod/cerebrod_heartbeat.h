/*****************************************************************************\
 *  $Id: cerebrod_heartbeat.h,v 1.4 2005-03-17 22:59:27 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_HEARTBEAT_H
#define _CEREBROD_HEARTBEAT_H

#include "cerebrod.h"

struct cerebrod_heartbeat
  {
    int32_t version;
    char hostname[CEREBROD_MAXHOSTNAMELEN];
    u_int32_t starttime;
    u_int32_t boottime;
  };

#define CEREBROD_HEARTBEAT_LEN  (sizeof(int32_t) \
                                 + CEREBROD_MAXHOSTNAMELEN \
                                 + sizeof(u_int32_t) \
                                 + sizeof(u_int32_t))

void cerebrod_heartbeat_construct(struct cerebrod_heartbeat *hb);
void cerebrod_heartbeat_dump(struct cerebrod_heartbeat *hb);
int cerebrod_heartbeat_marshall(struct cerebrod_heartbeat *hb, char *buffer, int len);
int cerebrod_heartbeat_unmarshall(struct cerebrod_heartbeat *hb, char *buffer, int len);

#endif /* _CEREBROD_HEARTBEAT_H */
