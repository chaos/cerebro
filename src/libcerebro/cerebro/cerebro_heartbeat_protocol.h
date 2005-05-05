/*****************************************************************************\
 *  $Id: cerebro_heartbeat_protocol.h,v 1.1 2005-05-05 16:24:26 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_HEARTBEAT_PROTOCOL_H
#define _CEREBRO_HEARTBEAT_PROTOCOL_H

#include <sys/types.h>

#include <cerebro/cerebro_constants.h>

#define CEREBRO_HEARTBEAT_PROTOCOL_VERSION 1

/*
 * struct cerebro_heartbeat
 *
 * defines heartbeat data sent/received from each cerebrod daemon
 */
struct cerebro_heartbeat
{
  int32_t version;
  char nodename[CEREBRO_MAXNODENAMELEN];
  u_int32_t starttime;
  u_int32_t boottime;
};
 
#define CEREBRO_HEARTBEAT_LEN  (sizeof(int32_t) \
                                + CEREBRO_MAXNODENAMELEN \
                                + sizeof(u_int32_t) \
                                + sizeof(u_int32_t))


#endif /* _CEREBRO_HEARTBEAT_PROTOCOL_H */
