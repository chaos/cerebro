/*****************************************************************************\
 *  $Id: cerebrod_heartbeat.h,v 1.5 2005-03-20 20:21:18 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_HEARTBEAT_H
#define _CEREBROD_HEARTBEAT_H

#include "cerebrod.h"

/* 
 * struct cerebrod_heartbeat
 *
 * defines heartbeat data sent/received from each cerebrod daemon
 */
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

/* 
 * cerebrod_heartbeat_construct
 *
 * construct a heartbeat packet
 */
void cerebrod_heartbeat_construct(struct cerebrod_heartbeat *hb);

/* 
 * cerebrod_heartbeat_dump
 *
 * dump contents of a heartbeat packet
 */
void cerebrod_heartbeat_dump(struct cerebrod_heartbeat *hb);

/* 
 * cerebrod_heartbeat_marshall
 *
 * marshall contents of a heartbeat packet.
 *
 * Returns length of data copied into buffer, -1 on error 
 */
int cerebrod_heartbeat_marshall(struct cerebrod_heartbeat *hb, char *buffer, int len);

/* 
 * cerebrod_heartbeat_unmarshall
 *
 * unmarshall contents of a packet buffer
 *
 * Returns 0 on success, -1 on error 
 */
int cerebrod_heartbeat_unmarshall(struct cerebrod_heartbeat *hb, char *buffer, int len);

#endif /* _CEREBROD_HEARTBEAT_H */
