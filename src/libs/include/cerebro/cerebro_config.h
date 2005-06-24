/*****************************************************************************\
 *  $Id: cerebro_config.h,v 1.2 2005-06-24 20:42:28 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_CONFIG_H
#define _CEREBRO_CONFIG_H

#include <cerebro/cerebro_constants.h>

#define CEREBRO_CONFIG_HOSTNAMES_MAX  16

/*  
 * struct cerebro_config
 *
 * Stores configuration info from config file.
 */
struct cerebro_config
{
  /* 
   * Libcerebro configuration
   */
  char cerebro_hostnames[CEREBRO_CONFIG_HOSTNAMES_MAX][CEREBRO_MAX_HOSTNAME_LEN+1];
  int cerebro_hostnames_len;
  int cerebro_hostnames_flag;
  unsigned int cerebro_port;
  int cerebro_port_flag;
  unsigned int cerebro_timeout_len;
  int cerebro_timeout_len_flag;
  int cerebro_flags;
  int cerebro_flags_flag;

  /* 
   * Cerebrod configuration
   */
  unsigned int cerebrod_heartbeat_frequency_min;
  unsigned int cerebrod_heartbeat_frequency_max;
  int cerebrod_heartbeat_frequency_flag;
  int cerebrod_heartbeat_source_port;
  int cerebrod_heartbeat_source_port_flag;
  int cerebrod_heartbeat_destination_port;
  int cerebrod_heartbeat_destination_port_flag;
  char cerebrod_heartbeat_destination_ip[CEREBRO_MAX_IPADDR_LEN+1];
  int cerebrod_heartbeat_destination_ip_flag;
  char cerebrod_heartbeat_network_interface[CEREBRO_MAX_NETWORK_INTERFACE_LEN+1];
  int cerebrod_heartbeat_network_interface_flag;
  int cerebrod_heartbeat_ttl;
  int cerebrod_heartbeat_ttl_flag;
  int cerebrod_speak;
  int cerebrod_speak_flag;
  int cerebrod_listen;
  int cerebrod_listen_flag;
  int cerebrod_listen_threads;
  int cerebrod_listen_threads_flag;
  int cerebrod_metric_server;
  int cerebrod_metric_server_flag;
  int cerebrod_metric_server_port;
  int cerebrod_metric_server_port_flag;
  int cerebrod_metric_max;
  int cerebrod_metric_max_flag;
  int cerebrod_monitor_max;
  int cerebrod_monitor_max_flag;
#if CEREBRO_DEBUG
  int cerebrod_speak_debug;
  int cerebrod_speak_debug_flag;
  int cerebrod_listen_debug;
  int cerebrod_listen_debug_flag;
  int cerebrod_metric_server_debug;
  int cerebrod_metric_server_debug_flag;
#endif /* CEREBRO_DEBUG */
};

#endif /* _CEREBRO_CONFIG_H */
