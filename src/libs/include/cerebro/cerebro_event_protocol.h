/*****************************************************************************\
 *  $Id: cerebro_event_protocol.h,v 1.1.2.5 2006-11-04 07:45:59 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2005 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <http://www.llnl.gov/linux/cerebro/>.
 *
 *  Cerebro is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  Cerebro is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with Genders; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
\*****************************************************************************/

#ifndef _CEREBRO_EVENT_PROTOCOL_H
#define _CEREBRO_EVENT_PROTOCOL_H

#include <sys/types.h>
#include <cerebro/cerebro_constants.h>

#define CEREBRO_EVENT_PROTOCOL_VERSION 1

/* 
 * struct cerebro_event
 *
 * defines an event that will be returned to listeners
 */
struct cerebro_event 
{
  int32_t version;
  char nodename[CEREBRO_MAX_NODENAME_LEN];
  char event_name[CEREBRO_MAX_EVENT_NAME_LEN];
  u_int32_t event_value_type;
  u_int32_t event_value_len;
  void *event_value;
};

#define CEREBRO_EVENT_HEADER_LEN  (sizeof(int32_t) \
                                   + CEREBRO_MAX_NODENAME_LEN \
                                   + CEREBRO_MAX_EVENT_NAME_LEN \
                                   + sizeof(u_int32_t) \
                                   + sizeof(u_int32_t))

/* Event server protocol
 *
 * Client -> Server
 * - Event request.
 *
 * Server -> Client
 * - When events occur, respones with event data will be returned.
 *   Client is responsible for polling appropriately.
 *
 * - On errors, an error packet will contain the error code and
 *   the server will close the connection automatically.
 *
 * Client -> Server
 * - Client can close the connection at any time by closing the 
 *   TCP connection.
 *
 * Notes:
 * - Require a smaller timeout len for the server than the client,
 *   so the server can respond to the client with a meaningful error
 *   message.
 */

#define CEREBRO_EVENT_SERVER_PROTOCOL_VERSION               1
#define CEREBRO_EVENT_SERVER_PROTOCOL_SERVER_TIMEOUT_LEN    3
#define CEREBRO_EVENT_SERVER_PROTOCOL_CLIENT_TIMEOUT_LEN    5
#define CEREBRO_EVENT_SERVER_PROTOCOL_CONNECT_TIMEOUT_LEN   5

#define CEREBRO_EVENT_SERVER_PROTOCOL_ERR_SUCCESS           0
#define CEREBRO_EVENT_SERVER_PROTOCOL_ERR_VERSION_INVALID   1
#define CEREBRO_EVENT_SERVER_PROTOCOL_ERR_EVENT_INVALID     2
#define CEREBRO_EVENT_SERVER_PROTOCOL_ERR_PARAMETER_INVALID 3
#define CEREBRO_EVENT_SERVER_PROTOCOL_ERR_PACKET_INVALID    4
#define CEREBRO_EVENT_SERVER_PROTOCOL_ERR_INTERNAL_ERROR    5

#endif /* _CEREBRO_EVENT_PROTOCOL_H */
