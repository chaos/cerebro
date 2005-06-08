/*****************************************************************************\
 *  $Id: cerebro_metric_protocol.h,v 1.3 2005-06-08 15:56:13 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_METRIC_PROTOCOL_H
#define _CEREBRO_METRIC_PROTOCOL_H

#include <sys/types.h>
#include <cerebro/cerebro_constants.h>

/* Metric server protocol
 *
 * Client -> Server
 * - Metric request.
 * 
 * Server -> Client
 * - Stream of responses indicating nodenames and metric values.
 *   Nodes returned will depend on the request flags.  After the stream of
 *   nodes is complete, an "end of responses" response will indicate the
 *   end of stream and completion of the request.  This "end of
 *   responses" response will be sent even if no nodes are returned
 *   (i.e. only down nodes are requested, but all nodes are up, or no
 *   nodes are monitoring the indicated metric).
 *
 * - On "normal" errors, the "end of responses" packet will contain 
 *   the error code.
 *
 * - On version incompatability errors and invalid packet format
 *   errors, an eight byte packet containing only the version and error
 *   code will be the response.
 *
 * Notes:
 * - Require a smaller timeout len for the server than the client, 
 *   so the server can respond to the client with a meaningful error
 *   message.
 */

#define CEREBRO_METRIC_PROTOCOL_VERSION                    2
#define CEREBRO_METRIC_PROTOCOL_SERVER_TIMEOUT_LEN         3
#define CEREBRO_METRIC_PROTOCOL_CLIENT_TIMEOUT_LEN         5
#define CEREBRO_METRIC_PROTOCOL_CONNECT_TIMEOUT_LEN        5

#define CEREBRO_METRIC_PROTOCOL_ERR_SUCCESS                0
#define CEREBRO_METRIC_PROTOCOL_ERR_VERSION_INVALID        1
#define CEREBRO_METRIC_PROTOCOL_ERR_METRIC_UNKNOWN         2
#define CEREBRO_METRIC_PROTOCOL_ERR_PARAMETER_INVALID      3
#define CEREBRO_METRIC_PROTOCOL_ERR_PACKET_INVALID         4
#define CEREBRO_METRIC_PROTOCOL_ERR_INTERNAL_SYSTEM_ERROR  5

#define CEREBRO_METRIC_PROTOCOL_IS_LAST_RESPONSE           1
#define CEREBRO_METRIC_PROTOCOL_IS_NOT_LAST_RESPONSE       0

#define CEREBRO_METRIC_UPDOWN_TIMEOUT_LEN_DEFAULT 60

#define CEREBRO_METRIC_SERVER_PORT                8852

/*
 * struct cerebro_metric_request
 *
 * defines a metric server data request
 */
struct cerebro_metric_request
{
  int32_t version;
  char metric_name[CEREBRO_METRIC_NAME_MAXLEN];
  u_int32_t timeout_len;
  u_int32_t flags;
};
  
#define CEREBRO_METRIC_REQUEST_PACKET_LEN  (sizeof(int32_t) \
                                            + CEREBRO_METRIC_NAME_MAXLEN \
                                            + sizeof(u_int32_t) \
                                            + sizeof(u_int32_t))

/*
 * struct cerebro_metric_response
 *
 * defines a metric server data response
 */
struct cerebro_metric_response
{
  int32_t version;
  u_int32_t metric_err_code;
  u_int8_t end_of_responses;
  char nodename[CEREBRO_MAXNODENAMELEN];
  u_int32_t metric_value_type;
  u_int32_t metric_value_len;
  void *metric_value;
};
  
#define CEREBRO_METRIC_RESPONSE_HEADER_LEN  (sizeof(int32_t) \
                                             + sizeof(u_int32_t) \
                                             + sizeof(u_int8_t) \
                                             + CEREBRO_MAXNODENAMELEN \
                                             + sizeof(u_int32_t) \
                                             + sizeof(u_int32_t))

/*
 * struct cerebro_metric_err_response
 *
 * defines a metric server invalid version or packet response
 */
struct cerebro_metric_err_response
{
  int32_t version;
  u_int32_t metric_err_code;
};
  
#define CEREBRO_METRIC_ERR_RESPONSE_LEN  (sizeof(int32_t) \
                                          + sizeof(u_int32_t))

#endif /* _CEREBRO_METRIC_PROTOCOL_H */
