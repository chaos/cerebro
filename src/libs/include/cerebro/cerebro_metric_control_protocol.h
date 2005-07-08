/*****************************************************************************\
 *  $Id: cerebro_metric_control_protocol.h,v 1.1 2005-07-08 23:54:37 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_METRIC_CONTROL_PROTOCOL_H
#define _CEREBRO_METRIC_CONTROL_PROTOCOL_H

#include <sys/types.h>
#include <cerebro/cerebro_constants.h>

/* Metric Control Protocol
 *
 * Client -> Server
 * - Metric request.
 * 
 * Server -> Client
 * - Metric response.
 *
 */

#define CEREBRO_METRIC_CONTROL_PROTOCOL_VERSION               4

#define CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_SUCCESS           0
#define CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_VERSION_INVALID   1
#define CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_COMMAND_INVALID   2
#define CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_PARAMETER_INVALID 3
#define CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_PACKET_INVALID    4
#define CEREBRO_METRIC_CONTROL_PROTOCOL_ERR_INTERNAL_ERROR    5

#define CEREBRO_METRIC_CONTROL_PROTOCOL_CMD_REGISTER          0
#define CEREBRO_METRIC_CONTROL_PROTOCOL_CMD_UNREGISTER        1
#define CEREBRO_METRIC_CONTROL_PROTOCOL_CMD_UPDATE            2

#define CEREBRO_METRIC_CONTROL_PATH

/*
 * struct cerebro_metric_control_request
 *
 * defines a metric control request
 */
struct cerebro_metric_control_request
{
  int32_t version;
  int32_t command;
  char metric_name[CEREBRO_MAX_METRIC_NAME_LEN];
  u_int32_t metric_value_type;
  u_int32_t metric_value_len;
  void *metric_value;
};

#define CEREBRO_METRIC_SERVER_REQUEST_HEADER_LEN  (sizeof(int32_t) \
                                                   sizeof(int32_t) \
                                                   + CEREBRO_MAX_METRIC_NAME_LEN \
                                                   + sizeof(u_int32_t) \
                                                   + sizeof(u_int32_t))

/*
 * struct cerebro_metric_control_response
 *
 * defines a metric control response.
 */
struct cerebro_metric_control_response
{
  int32_t version;
  u_int32_t err_code;
};
  
#define CEREBRO_METRIC_CONTROL_RESPONSE_LEN  (sizeof(int32_t) \
                                             + sizeof(u_int32_t))

#endif /* _CEREBRO_METRIC_CONTROL_PROTOCOL_H */
