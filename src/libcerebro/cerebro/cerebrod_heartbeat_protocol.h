/*****************************************************************************\
 *  $Id: cerebrod_heartbeat_protocol.h,v 1.1 2005-05-05 16:12:57 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_HEARTBEAT_PROTOCOL_H
#define _CEREBROD_HEARTBEAT_PROTOCOL_H

#include <sys/types.h>

#include <cerebro/cerebro_constants.h>

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
