/*****************************************************************************\
 *  $Id: cerebro_metric_protocol.h,v 1.1 2005-05-19 16:40:40 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_METRIC_PROTOCOL_H
#define _CEREBRO_METRIC_PROTOCOL_H

#if 0

#include <sys/types.h>

#include <cerebro/cerebro_constants.h>

/* Updown server protocol
 *
 * Client -> Server
 * - Cerebro updown request.
 * 
 * Server -> Client
 * - Stream of updown responses indicating up/down metric of each
 *   node.  Stream of nodes returned will depend on the request.
 *   After the stream of nodes is complete, an "end of responses"
 *   response will indicate the end of stream and completion of
 *   the updown request.  This "end of responses" response will be
 *   sent even if no nodes are returned (i.e. only down nodes are
 *   requested, but all nodes are up).
 * - On "normal" errors, the "end of responses" packet will contain 
 *   the error code.
 * - On version incompatability errors, older version responses
 *   will be sent from the server containing the error code.
 * - On version incompatability errors in which an older version does
 *   not exist and invalid packet format errors, an eight byte packet
 *   containing only the version and error code will be the response.
 *
 * Notes:
 * - Require a smaller timeout len for the server than the client, 
 *   so the server can respond to the client with a meaningful error
 *   message.
 */
#endif /* 0 */

#define CEREBRO_METRIC_PROTOCOL_VERSION                    1
#define CEREBRO_METRIC_PROTOCOL_SERVER_TIMEOUT_LEN         3
#define CEREBRO_METRIC_PROTOCOL_CLIENT_TIMEOUT_LEN         5
#define CEREBRO_METRIC_PROTOCOL_CONNECT_TIMEOUT_LEN        5

#define CEREBRO_METRIC_STARTTIME                           "starttime"
#define CEREBRO_METRIC_BOOTTIME                            "boottime"

#if 0

#define CEREBRO_UPDOWN_PROTOCOL_ERR_SUCCESS                0
#define CEREBRO_UPDOWN_PROTOCOL_ERR_VERSION_INVALID        1
#define CEREBRO_UPDOWN_PROTOCOL_ERR_UPDOWN_REQUEST_INVALID 2
#define CEREBRO_UPDOWN_PROTOCOL_ERR_TIMEOUT_INVALID        3
#define CEREBRO_UPDOWN_PROTOCOL_ERR_PACKET_INVALID         4
#define CEREBRO_UPDOWN_PROTOCOL_ERR_INTERNAL_SYSTEM_ERROR  5

#define CEREBRO_UPDOWN_PROTOCOL_REQUEST_UP_NODES           0
#define CEREBRO_UPDOWN_PROTOCOL_REQUEST_DOWN_NODES         1
#define CEREBRO_UPDOWN_PROTOCOL_REQUEST_UP_AND_DOWN_NODES  2

#define CEREBRO_UPDOWN_PROTOCOL_STATE_NODE_UP              1
#define CEREBRO_UPDOWN_PROTOCOL_STATE_NODE_DOWN            0

#define CEREBRO_UPDOWN_PROTOCOL_IS_LAST_RESPONSE           1
#define CEREBRO_UPDOWN_PROTOCOL_IS_NOT_LAST_RESPONSE       0

#endif /* 0 */

#define CEREBRO_METRIC_SERVER_PORT  8853

#define CEREBRO_METRIC_MAX          16

#if 0

/*
 * struct cerebro_updown_request
 *
 * defines a updown server data request
 */
struct cerebro_updown_request
{
  int32_t version;
  u_int32_t updown_request;
  u_int32_t timeout_len;
};
  
#define CEREBRO_UPDOWN_REQUEST_LEN  (sizeof(int32_t) \
                                     + sizeof(u_int32_t) \
                                     + sizeof(u_int32_t))

/*
 * struct cerebro_updown_response
 *
 * defines a updown server data response
 */
struct cerebro_updown_response
{
  int32_t version;
  u_int32_t updown_err_code;
  u_int8_t end_of_responses;
  char nodename[CEREBRO_MAXNODENAMELEN];
  u_int8_t updown_state;
};
  
#define CEREBRO_UPDOWN_RESPONSE_LEN  (sizeof(int32_t) \
                                      + sizeof(u_int32_t) \
                                      + sizeof(u_int8_t) \
                                      + CEREBRO_MAXNODENAMELEN \
                                      + sizeof(u_int8_t))

/*
 * struct cerebro_updown_err_response
 *
 * defines a updown server invalid version or packet response
 */
struct cerebro_updown_err_response
{
  int32_t version;
  u_int32_t updown_err_code;
};
  
#define CEREBRO_UPDOWN_ERR_RESPONSE_LEN  (sizeof(int32_t) \
                                          + sizeof(u_int32_t))


#endif /* 0 */

#endif /* _CEREBRO_METRIC_PROTOCOL_H */
