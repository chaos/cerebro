/*****************************************************************************\
 *  $Id: cerebro_config.h,v 1.5 2005-05-04 17:24:05 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_CONFIG_H
#define _CEREBRO_CONFIG_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "cerebro_constants.h"

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
 * cerebro_config_load_config_module
 *
 * Find and load config module
 *
 * Returns data in structure and 0 on success, -1 on error
 */
int cerebro_config_load_config_module(struct cerebro_config *conf);

/* 
 * cerebro_config_load_config_file
 *
 * Read and load configuration file
 *
 * Returns data in structure and 0 on success, -1 on error
 */
int cerebro_config_load_config_file(struct cerebro_config *conf);

/* 
 * cerebro_config_merge_cerebro_config
 *
 * Merge contents of module_conf and config_file_conf into conf.  The
 * config file conf takes precedence.
 */
int cerebro_config_merge_cerebro_config(struct cerebro_config *conf,
					struct cerebro_config *module_conf,
					struct cerebro_config *config_file_conf);

/* 
 * cerebro_config_load
 *
 * Wrapper that calls cerebro_config_load_config_module,
 * cerebro_config_load_config_file, and
 * cerebro_config_merge_cerebro_config.
 *
 *
 * Returns data in structure and 0 on success, -1 on error
 */
int cerebro_config_load(struct cerebro_config *conf);
#endif /* _CEREBRO_CONFIG_H */
