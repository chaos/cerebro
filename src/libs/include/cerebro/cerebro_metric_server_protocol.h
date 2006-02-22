/*****************************************************************************\
 *  $Id: cerebro_metric_server_protocol.h,v 1.7 2006-02-22 06:08:28 chu11 Exp $
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

#ifndef _CEREBRO_METRIC_SERVER_PROTOCOL_H
#define _CEREBRO_METRIC_SERVER_PROTOCOL_H

#include <sys/types.h>
#include <cerebro/cerebro_constants.h>

/* Metric server protocol
 *
 * Client -> Server
 * - Metric request.
 * 
 * Server -> Client

 * - Stream of responses indicating metric_names or nodenames and
 *   metric values.  Respones may depend on the request flags.  Met ric
 *   receive times will be 0 for metrics in which a receive time makes no
 *   sense (i.e. cluster node name). After the stream of is complete,
 *   an "end of responses" response will indicate the end of stream
 *   and completion of the request.  This "end of responses" response
 *   will be sent even if no data is returned (i.e. only down nodes
 *   are requested, but all nodes are up, or no nodes are monitoring
 *   the indicated metric).
 *
 * - On errors, an error packet will contain the error code.
 *
 * Notes:
 * - Require a smaller timeout len for the server than the client, 
 *   so the server can respond to the client with a meaningful error
 *   message.
 */

#define CEREBRO_METRIC_SERVER_PROTOCOL_VERSION               5
#define CEREBRO_METRIC_SERVER_PROTOCOL_SERVER_TIMEOUT_LEN    3
#define CEREBRO_METRIC_SERVER_PROTOCOL_CLIENT_TIMEOUT_LEN    5
#define CEREBRO_METRIC_SERVER_PROTOCOL_CONNECT_TIMEOUT_LEN   5

#define CEREBRO_METRIC_SERVER_PROTOCOL_ERR_SUCCESS           0
#define CEREBRO_METRIC_SERVER_PROTOCOL_ERR_VERSION_INVALID   1
#define CEREBRO_METRIC_SERVER_PROTOCOL_ERR_METRIC_INVALID    2
#define CEREBRO_METRIC_SERVER_PROTOCOL_ERR_PARAMETER_INVALID 3
#define CEREBRO_METRIC_SERVER_PROTOCOL_ERR_PACKET_INVALID    4
#define CEREBRO_METRIC_SERVER_PROTOCOL_ERR_INTERNAL_ERROR    5

#define CEREBRO_METRIC_SERVER_PROTOCOL_IS_LAST_RESPONSE      1
#define CEREBRO_METRIC_SERVER_PROTOCOL_IS_NOT_LAST_RESPONSE  0

/*
 * struct cerebro_metric_server_request
 *
 * defines a metric server data request
 */
struct cerebro_metric_server_request
{
  int32_t version;
  char metric_name[CEREBRO_MAX_METRIC_NAME_LEN];
  u_int32_t timeout_len;
  u_int32_t flags;
};
  
#define CEREBRO_METRIC_SERVER_REQUEST_PACKET_LEN  (sizeof(int32_t) \
                                                   + CEREBRO_MAX_METRIC_NAME_LEN \
                                                   + sizeof(u_int32_t) \
                                                   + sizeof(u_int32_t))

/*
 * struct cerebro_metric_server_response
 *
 * defines a metric server data response.  The 'name' field may
 * contain a metric name or a nodename.
 */
struct cerebro_metric_server_response
{
  int32_t version;
  u_int32_t err_code;
  u_int8_t end;
  char name[CEREBRO_MAX_NAME_LEN];
  u_int32_t metric_value_received_time;
  u_int32_t metric_value_type;
  u_int32_t metric_value_len;
  void *metric_value;
};
  
#define CEREBRO_METRIC_SERVER_RESPONSE_HEADER_LEN  (sizeof(int32_t) \
                                                    + sizeof(u_int32_t) \
                                                    + sizeof(u_int8_t) \
                                                    + CEREBRO_MAX_NAME_LEN \
                                                    + sizeof(u_int32_t) \
                                                    + sizeof(u_int32_t) \
                                                    + sizeof(u_int32_t))

/*
 * struct cerebro_metric_server_err_response
 *
 * defines a metric server invalid version or packet response
 */
struct cerebro_metric_server_err_response
{
  int32_t version;
  u_int32_t err_code;
};
  
#define CEREBRO_METRIC_SERVER_ERR_RESPONSE_LEN  (sizeof(int32_t) \
                                                 + sizeof(u_int32_t))

#endif /* _CEREBRO_METRIC_SERVER_PROTOCOL_H */
