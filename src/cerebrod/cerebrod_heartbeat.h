/*****************************************************************************\
 *  $Id: cerebrod_heartbeat.h,v 1.10 2005-04-26 17:31:35 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_HEARTBEAT_H
#define _CEREBROD_HEARTBEAT_H

/* 
 * cerebrod_heartbeat_dump
 *
 * dump contents of a heartbeat packet.  Should be called with
 * debug_output_mutex held.
 */
void cerebrod_heartbeat_dump(struct cerebrod_heartbeat *hb);

#endif /* _CEREBROD_HEARTBEAT_H */
