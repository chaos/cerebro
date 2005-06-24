/*****************************************************************************\
 *  $Id: cerebrod_heartbeat_protocol.h,v 1.2 2005-06-24 20:42:28 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_HEARTBEAT_PROTOCOL_H
#define _CEREBROD_HEARTBEAT_PROTOCOL_H

#include <sys/types.h>

#include <cerebro/cerebro_constants.h>

#define CEREBROD_HEARTBEAT_PROTOCOL_VERSION 2

/*
 * struct cerebrod_heartbeat_metric
 *
 * defines heartbeat metric data
 */
struct cerebrod_heartbeat_metric
{
  char metric_name[CEREBRO_MAX_METRIC_NAME_LEN];
  u_int32_t metric_value_type;
  u_int32_t metric_value_len;
  void *metric_value;
};

#define CEREBROD_HEARTBEAT_METRIC_HEADER_LEN  (CEREBRO_MAX_METRIC_NAME_LEN \
                                               + sizeof(u_int32_t) \
                                               + sizeof(u_int32_t))

/* 
 * struct cerebrod_heartbeat
 *
 * defines heartbeat data sent/received from each cerebrod daemon
 */
struct cerebrod_heartbeat 
{
  int32_t version;
  char nodename[CEREBRO_MAX_NODENAME_LEN];
  u_int32_t metrics_len;
  struct cerebrod_heartbeat_metric **metrics;
};

#define CEREBROD_HEARTBEAT_HEADER_LEN  (sizeof(int32_t) \
                                        + CEREBRO_MAX_NODENAME_LEN \
                                        + sizeof(u_int32_t))

#endif /* _CEREBROD_HEARTBEAT_PROTOCOL_H */
