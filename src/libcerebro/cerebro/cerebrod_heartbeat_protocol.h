/*****************************************************************************\
 *  $Id: cerebrod_heartbeat_protocol.h,v 1.6 2005-06-07 17:26:50 achu Exp $
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
  char metric_name[CEREBRO_METRIC_NAME_MAXLEN];
  u_int32_t metric_value_type;
  u_int32_t metric_value_len;
  void *metric_value;
};

/* 
 * struct cerebrod_heartbeat
 *
 * defines heartbeat data sent/received from each cerebrod daemon
 */
struct cerebrod_heartbeat 
{
  int32_t version;
  char nodename[CEREBRO_MAXNODENAMELEN];
  u_int32_t metrics_len;
  struct cerebrod_heartbeat_metric **metrics;
};

#define CEREBROD_HEARTBEAT_HEADER_LEN  (sizeof(int32_t) \
                                        + CEREBRO_MAXNODENAMELEN \
                                        + sizeof(u_int32_t))

#endif /* _CEREBROD_HEARTBEAT_PROTOCOL_H */
