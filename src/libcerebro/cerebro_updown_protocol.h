/*****************************************************************************\
 *  $Id: cerebro_updown_protocol.h,v 1.11 2005-04-27 00:01:30 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_UPDOWN_PROTOCOL_H
#define _CEREBRO_UPDOWN_PROTOCOL_H

#include <sys/types.h>
#include "cerebro_defs.h"

/* Updown server protocol
 *
 * Client -> Server
 * - Cerebro updown request.
 * 
 * Server -> Client
 * - Stream of updown responses indicating up/down status of each
 *   node.  Stream of nodes returned will depend on the request.
 *   After the stream of nodes is complete, an "end of responses"
 *   response will indicate the end of stream and completion of
 *   the updown request.  This "end of responses" response will be
 *   sent even if no nodes are returned (i.e. only down nodes are
 *   requested, but all nodes are up).
 */

#define CEREBRO_UPDOWN_PROTOCOL_VERSION                    1
#define CEREBRO_UPDOWN_PROTOCOL_CONNECT_TIMEOUT_LEN        5

#define CEREBRO_UPDOWN_PROTOCOL_ERR_SUCCESS                0
#define CEREBRO_UPDOWN_PROTOCOL_ERR_VERSION_INVALID        1
#define CEREBRO_UPDOWN_PROTOCOL_ERR_UPDOWN_REQUEST_INVALID 2
#define CEREBRO_UPDOWN_PROTOCOL_ERR_TIMEOUT_INVALID        3
#define CEREBRO_UPDOWN_PROTOCOL_ERR_INTERNAL_SYSTEM_ERROR  4

#define CEREBRO_UPDOWN_PROTOCOL_REQUEST_UP_NODES           0
#define CEREBRO_UPDOWN_PROTOCOL_REQUEST_DOWN_NODES         1
#define CEREBRO_UPDOWN_PROTOCOL_REQUEST_UP_AND_DOWN_NODES  2

#define CEREBRO_UPDOWN_PROTOCOL_STATE_NODE_UP              1
#define CEREBRO_UPDOWN_PROTOCOL_STATE_NODE_DOWN            0

#define CEREBRO_UPDOWN_PROTOCOL_IS_LAST_RESPONSE           1
#define CEREBRO_UPDOWN_PROTOCOL_IS_NOT_LAST_RESPONSE       0

#define CEREBRO_UPDOWN_SERVER_PORT                         8852

#define CEREBRO_UPDOWN_TIMEOUT_LEN_DEFAULT                 60 

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

#endif /* _CEREBRO_UPDOWN_PROTOCOL_H */
