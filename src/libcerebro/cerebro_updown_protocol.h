/*****************************************************************************\
 *  $Id: cerebro_updown_protocol.h,v 1.4 2005-03-26 17:22:31 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_UPDOWN_PROTOCOL_H
#define _CEREBRO_UPDOWN_PROTOCOL_H

#include <sys/types.h>
#include "cerebro_defs.h"

#define CEREBRO_UPDOWN_PROTOCOL_VERSION                     1
 
#define CEREBRO_UPDOWN_REQUEST_UP_NODES                     0
#define CEREBRO_UPDOWN_REQUEST_DOWN_NODES                   1
#define CEREBRO_UPDOWN_REQUEST_UP_AND_DOWN_NODES            2

#define CEREBRO_UPDOWN_RESPONSE_CODE_SUCCESS                0
#define CEREBRO_UPDOWN_RESPONSE_CODE_VERSION_INVALID        1
#define CEREBRO_UPDOWN_RESPONSE_CODE_UPDOWN_REQUEST_INVALID 2
#define CEREBRO_UPDOWN_RESPONSE_CODE_TIMEOUT_INVALID        3
#define CEREBRO_UPDOWN_RESPONSE_CODE_SYSTEM_ERROR           4
#define CEREBRO_UPDOWN_RESPONSE_CODE_UNKNOWN_ERROR          5

#define CEREBRO_UPDOWN_RESPONSE_NODE_UP                     0
#define CEREBRO_UPDOWN_RESPONSE_NODE_DOWN                   1

/*
 * struct cerebrod_updown_request
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
 * struct cerebrod_updown_response
 *
 * defines a updown server data response
 */
struct cerebro_updown_response
{
  int32_t version;
  u_int32_t updown_response_code;
  char hostname[CEREBRO_MAXHOSTNAMELEN];
  u_int32_t updown_state;
};
  
#define CEREBRO_UPDOWN_RESPONSE_LEN  (sizeof(int32_t) \
                                      sizeof(u_int32_t) \
                                      + CEREBRO_MAXHOSTNAMELEN \
                                      + sizeof(u_int32_t))

#endif /* _CEREBRO_UPDOWN_PROTOCOL_H */
