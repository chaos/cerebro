/*****************************************************************************\
 *  $Id: cerebrod_heartbeat.h,v 1.9 2005-04-21 17:59:15 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_HEARTBEAT_H
#define _CEREBROD_HEARTBEAT_H

#include "cerebrod.h"

/* 
 * cerebrod_heartbeat_construct
 *
 * construct a heartbeat packet
 */
void cerebrod_heartbeat_construct(struct cerebrod_heartbeat *hb);

/* 
 * cerebrod_heartbeat_dump
 *
 * dump contents of a heartbeat packet.  Should be called with
 * debug_output_mutex held.
 */
void cerebrod_heartbeat_dump(struct cerebrod_heartbeat *hb);

/* 
 * cerebrod_heartbeat_marshall
 *
 * marshall contents of a heartbeat packet.
 *
 * Returns length of data copied into buffer, -1 on error 
 */
int cerebrod_heartbeat_marshall(struct cerebrod_heartbeat *hb, 
                                char *buffer, 
                                int len);

/* 
 * cerebrod_heartbeat_unmarshall
 *
 * unmarshall contents of a heartbeat packet buffer
 *
 * Returns 0 on success, -1 on error 
 */
int cerebrod_heartbeat_unmarshall(struct cerebrod_heartbeat *hb, 
                                  char *buffer, 
                                  int len);

#endif /* _CEREBROD_HEARTBEAT_H */
