/*****************************************************************************\
 *  $Id: cerebro_config.h,v 1.9 2006-11-08 00:34:04 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2005 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <http://www.llnl.gov/linux/cerebro/>.
 *
 *  Cerebro is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  Cerebro is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with Genders; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
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
  char cerebrod_heartbeat_source_network_interface[CEREBRO_MAX_NETWORK_INTERFACE_LEN+1];
  int cerebrod_heartbeat_source_network_interface_flag;
  int cerebrod_heartbeat_destination_port;
  int cerebrod_heartbeat_destination_port_flag;
  char cerebrod_heartbeat_destination_ip[CEREBRO_MAX_IPADDR_LEN+1];
  int cerebrod_heartbeat_destination_ip_flag;
  int cerebrod_heartbeat_ttl;
  int cerebrod_heartbeat_ttl_flag;
  int cerebrod_speak;
  int cerebrod_speak_flag;
  int cerebrod_listen;
  int cerebrod_listen_flag;
  int cerebrod_listen_threads;
  int cerebrod_listen_threads_flag;
  int cerebrod_listen_ports[CEREBRO_MAX_LISTENERS];
  int cerebrod_listen_ports_len;
  int cerebrod_listen_ports_flag;
  char cerebrod_listen_ips[CEREBRO_MAX_LISTENERS][CEREBRO_MAX_IPADDR_LEN+1];
  int cerebrod_listen_ips_len;
  int cerebrod_listen_ips_flag;
  char cerebrod_listen_network_interfaces[CEREBRO_MAX_LISTENERS][CEREBRO_MAX_NETWORK_INTERFACE_LEN+1];
  int cerebrod_listen_network_interfaces_len;
  int cerebrod_listen_network_interfaces_flag;
  int cerebrod_metric_controller;
  int cerebrod_metric_controller_flag;
  int cerebrod_metric_server;
  int cerebrod_metric_server_flag;
  int cerebrod_metric_server_port;
  int cerebrod_metric_server_port_flag;
  int cerebrod_event_server;
  int cerebrod_event_server_flag;
  int cerebrod_event_server_port;
  int cerebrod_event_server_port_flag;
#if CEREBRO_DEBUG
  int cerebrod_speak_debug;
  int cerebrod_speak_debug_flag;
  int cerebrod_listen_debug;
  int cerebrod_listen_debug_flag;
  int cerebrod_metric_controller_debug;
  int cerebrod_metric_controller_debug_flag;
  int cerebrod_metric_server_debug;
  int cerebrod_metric_server_debug_flag;
  int cerebrod_event_server_debug;
  int cerebrod_event_server_debug_flag;
#endif /* CEREBRO_DEBUG */
};

#endif /* _CEREBRO_CONFIG_H */
