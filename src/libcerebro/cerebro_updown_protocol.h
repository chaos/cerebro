/*****************************************************************************\
 *  $Id: cerebro_updown_protocol.h,v 1.6 2005-03-28 17:40:10 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_UPDOWN_PROTOCOL_H
#define _CEREBRO_UPDOWN_PROTOCOL_H

#include <sys/types.h>
#include "cerebro_defs.h"

/* Updown server protocol
 *
 * Client -> Server: Cerebro updown request.
 * 
 * Server -> Client: Stream of updown responses indicating 
 *                   up/down status of each node.
 */

#define CEREBRO_UPDOWN_PROTOCOL_VERSION                1
#define CEREBRO_UPDOWN_PROTOCOL_TIMEOUT_LEN            5

#define CEREBRO_UPDOWN_REQUEST_UP_NODES                0
#define CEREBRO_UPDOWN_REQUEST_DOWN_NODES              1
#define CEREBRO_UPDOWN_REQUEST_UP_AND_DOWN_NODES       2

#define CEREBRO_UPDOWN_TIMEOUT_LEN_DEFAULT             60 

#define CEREBRO_UPDOWN_ERR_CODE_SUCCESS                0
#define CEREBRO_UPDOWN_ERR_CODE_VERSION_INVALID        1
#define CEREBRO_UPDOWN_ERR_CODE_UPDOWN_REQUEST_INVALID 2
#define CEREBRO_UPDOWN_ERR_CODE_TIMEOUT_INVALID        3
#define CEREBRO_UPDOWN_ERR_CODE_NO_NODES_FOUND         4
#define CEREBRO_UPDOWN_ERR_CODE_INTERNAL_SYSTEM_ERROR  5

#define CEREBRO_UPDOWN_STATE_NODE_UP                   0
#define CEREBRO_UPDOWN_STATE_NODE_DOWN                 1

#define CEREBRO_UPDOWN_IS_LAST_RESPONSE                1
#define CEREBRO_UPDOWN_IS_NOT_LAST_RESPONSE            0

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
  char hostname[CEREBRO_MAXHOSTNAMELEN];
  u_int8_t updown_state;
  u_int8_t is_last_response;
};
  
#define CEREBRO_UPDOWN_RESPONSE_LEN  (sizeof(int32_t) \
                                      + sizeof(u_int32_t) \
                                      + CEREBRO_MAXHOSTNAMELEN \
                                      + sizeof(u_int8_t) \
                                      + sizeof(u_int8_t))

#endif /* _CEREBRO_UPDOWN_PROTOCOL_H */
