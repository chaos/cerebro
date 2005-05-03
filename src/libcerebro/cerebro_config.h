/*****************************************************************************\
 *  $Id: cerebro_config.h,v 1.2 2005-05-03 21:47:39 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_CONFIG_H
#define _CEREBRO_CONFIG_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "cerebro_defs.h"

#define CEREBRO_CONFIG_UPDOWN_HOSTNAMES_MAX  16

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
  char cerebro_updown_hostnames[CEREBRO_CONFIG_UPDOWN_HOSTNAMES_MAX][CEREBRO_MAXNODENAMELEN+1];
  int cerebro_updown_hostnames_len;
  int cerebro_updown_hostnames_flag;
  unsigned int cerebro_updown_port;
  int cerebro_updown_port_flag;
  unsigned int cerebro_updown_timeout_len;
  int cerebro_updown_timeout_len_flag;
  int cerebro_updown_flags;
  int cerebro_updown_flags_flag;

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
  char cerebrod_heartbeat_destination_ip[CEREBRO_IPADDRSTRLEN+1];
  int cerebrod_heartbeat_destination_ip_flag;
  char cerebrod_heartbeat_network_interface[CEREBRO_MAXNETWORKINTERFACE+1];
  int cerebrod_heartbeat_network_interface_flag;
  int cerebrod_heartbeat_ttl;
  int cerebrod_heartbeat_ttl_flag;
  int cerebrod_speak;
  int cerebrod_speak_flag;
  int cerebrod_listen;
  int cerebrod_listen_flag;
  int cerebrod_listen_threads;
  int cerebrod_listen_threads_flag;
  int cerebrod_updown_server;
  int cerebrod_updown_server_flag;
  int cerebrod_updown_server_port;
  int cerebrod_updown_server_port_flag;
#ifndef NDEBUG
  int cerebrod_speak_debug;
  int cerebrod_speak_debug_flag;
  int cerebrod_listen_debug;
  int cerebrod_listen_debug_flag;
  int cerebrod_updown_server_debug;
  int cerebrod_updown_server_debug_flag;
#endif /* NDEBUG */
};

/* 
 * cerebro_load_config
 *
 * Load the cerebro config module and parse the cerebro config file
 * library and load its contents.
 *
 * Returns data in structure and 0 on success, -1 on error
 */
int cerebro_load_config(struct cerebro_config *conf);

#endif /* _CEREBRO_CONFIG_H */
