/*****************************************************************************\
 *  $Id: cerebrod_heartbeat.h,v 1.2 2004-11-17 00:49:31 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_HEARTBEAT_H
#define _CEREBROD_HEARTBEAT_H

#include <sys/param.h>

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

struct cerebrod_heartbeat_t
  {
    int32_t version;
    char hostname[MAXHOSTNAMELEN];
    u_int32_t boottime;
  };

#define CEREBROD_HEARTBEAT_LEN  (sizeof(int32_t) + MAXHOSTNAMELEN \
                                 + sizeof(u_int32_t))

int cerebrod_heartbeat_marshall(struct cerebrod_heartbeat_t *cb, char *buffer, int len);
int cerebrod_heartbeat_unmarshall(struct cerebrod_heartbeat_t *cb, char *buffer, int len);

#endif /* _CEREBROD_HEARTBEAT_H */
