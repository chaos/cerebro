/*****************************************************************************\
 *  $Id: cerebrod_heartbeat.h,v 1.1 2004-11-08 19:07:51 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_HEARTBEAT_H
#define _CEREBROD_HEARTBEAT_H

#include <sys/param.h>

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

struct cerebro_heartbeat_t
  {
    int32_t version;
    char hostname[MAXHOSTNAMELEN];
    u_int32_t boottime;
  };

int cerebrod_heartbeat_marshall(struct cerebro_heartbeat_t *cb, char *buffer, int len);
int cerebrod_heartbeat_unmarshall(struct cerebro_heartbeat_t *cb, char *buffer, int len);

#endif /* _CEREBROD_HEARTBEAT_H */
