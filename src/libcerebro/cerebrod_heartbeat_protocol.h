/*****************************************************************************\
 *  $Id: cerebrod_heartbeat_protocol.h,v 1.3 2005-03-30 18:46:58 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_HEARTBEAT_PROTOCOL_H
#define _CEREBROD_HEARTBEAT_PROTOCOL_H

#include <sys/types.h>
#include "cerebro_defs.h"

#define CEREBROD_HEARTBEAT_PROTOCOL_VERSION 1

/*
 * struct cerebrod_heartbeat
 *
 * defines heartbeat data sent/received from each cerebrod daemon
 */
struct cerebrod_heartbeat
{
  int32_t version;
  char nodename[CEREBRO_MAXNODENAMELEN];
  u_int32_t starttime;
  u_int32_t boottime;
};
 
#define CEREBROD_HEARTBEAT_LEN  (sizeof(int32_t) \
                                 + CEREBRO_MAXNODENAMELEN \
                                 + sizeof(u_int32_t) \
                                 + sizeof(u_int32_t))


#endif /* _CEREBROD_HEARTBEAT_PROTOCOL_H */
