/*****************************************************************************\
 *  $Id: cerebrod_heartbeat.h,v 1.11 2005-06-08 00:10:49 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_HEARTBEAT_H
#define _CEREBROD_HEARTBEAT_H

#include "cerebro/cerebrod_heartbeat_protocol.h"

/* 
 * cerebrod_heartbeat_dump
 *
 * dump contents of a heartbeat packet.  Should be called with
 * debug_output_mutex held.
 */
void cerebrod_heartbeat_dump(struct cerebrod_heartbeat *hb);

/*
 * cerebrod_heartbeat_destroy
 *
 * destroy a heartbeat packet
 */
void cerebrod_heartbeat_destroy(struct cerebrod_heartbeat *hb);

#endif /* _CEREBROD_HEARTBEAT_H */
