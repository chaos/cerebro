/*****************************************************************************\
 *  $Id: cerebro_config.h,v 1.21 2010-02-02 01:01:20 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2011 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2005-2007 The Regents of the University of California.
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
 *  with Cerebro. If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#ifndef _CEREBRO_CONFIG_H
#define _CEREBRO_CONFIG_H

#include <cerebro/cerebro_constants.h>

#define CEREBRO_CONFIG_CEREBRO_METRIC_SERVERS_MAX 128

#define CEREBRO_CONFIG_CEREBRO_EVENT_SERVERS_MAX  128

#define CEREBRO_CONFIG_SPEAK_MESSAGE_CONFIG_MAX   128

#define CEREBRO_CONFIG_LISTEN_MESSAGE_CONFIG_MAX  128

#define CEREBRO_CONFIG_FORWARD_MESSAGE_CONFIG_MAX 128

#define CEREBRO_CONFIG_FORWARD_HOST_MAX           64

#define CEREBRO_CONFIG_FORWARD_HOST_ACCEPT_MAX    512

#define CEREBRO_CONFIG_METRIC_MODULE_EXCLUDE_MAX  64

#define CEREBRO_CONFIG_MONITOR_MODULE_EXCLUDE_MAX 64

#define CEREBRO_CONFIG_EVENT_MODULE_EXCLUDE_MAX   64

#define CEREBRO_CONFIG_PORT_DEFAULT               0

#define CEREBRO_CONFIG_IP_DEFAULT                 "0.0.0.0"

#define CEREBRO_CONFIG_HOST_INPUT_MAX             128

struct cerebro_config_server {
  char hostname[CEREBRO_MAX_HOSTNAME_LEN+1];
  unsigned int port;
};

struct cerebrod_config_speak_message_config {
  char ip[CEREBRO_MAX_HOSTNAME_LEN+1];
  int destination_port;
  int source_port;
  char network_interface[CEREBRO_MAX_NETWORK_INTERFACE_LEN+1];
};

struct cerebrod_config_listen_message_config {
  char ip[CEREBRO_MAX_HOSTNAME_LEN+1];
  int port;
  char network_interface[CEREBRO_MAX_NETWORK_INTERFACE_LEN+1];
};

struct cerebrod_config_forward_message_config {
  char ip[CEREBRO_MAX_HOSTNAME_LEN+1];
  int destination_port;
  int source_port;
  char network_interface[CEREBRO_MAX_NETWORK_INTERFACE_LEN+1];
  char host[CEREBRO_CONFIG_FORWARD_HOST_MAX][CEREBRO_CONFIG_HOST_INPUT_MAX+1];
  unsigned int host_len;
};

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
  struct cerebro_config_server cerebro_metric_server[CEREBRO_CONFIG_CEREBRO_METRIC_SERVERS_MAX];
  int cerebro_metric_server_len;
  int cerebro_metric_server_flag;
  struct cerebro_config_server cerebro_event_server[CEREBRO_CONFIG_CEREBRO_EVENT_SERVERS_MAX];
  int cerebro_event_server_len;
  int cerebro_event_server_flag;
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
  int cerebrod_speak;
  int cerebrod_speak_flag;
  struct cerebrod_config_speak_message_config cerebrod_speak_message_config[CEREBRO_CONFIG_SPEAK_MESSAGE_CONFIG_MAX];
  int cerebrod_speak_message_config_len;
  int cerebrod_speak_message_config_flag;
  int cerebrod_speak_message_ttl;
  int cerebrod_speak_message_ttl_flag;
  int cerebrod_listen;
  int cerebrod_listen_flag;
  int cerebrod_listen_threads;
  int cerebrod_listen_threads_flag;
  struct cerebrod_config_listen_message_config cerebrod_listen_message_config[CEREBRO_CONFIG_LISTEN_MESSAGE_CONFIG_MAX];
  int cerebrod_listen_message_config_len;
  int cerebrod_listen_message_config_flag;
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
  struct cerebrod_config_forward_message_config cerebrod_forward_message_config[CEREBRO_CONFIG_FORWARD_MESSAGE_CONFIG_MAX];
  int cerebrod_forward_message_config_len;
  int cerebrod_forward_message_config_flag;
  int cerebrod_forward_message_ttl;
  int cerebrod_forward_message_ttl_flag;
  char cerebrod_forward_host_accept[CEREBRO_CONFIG_FORWARD_HOST_ACCEPT_MAX][CEREBRO_CONFIG_HOST_INPUT_MAX+1];
  int cerebrod_forward_host_accept_len;
  int cerebrod_forward_host_accept_flag;
  char cerebrod_metric_module_exclude[CEREBRO_CONFIG_METRIC_MODULE_EXCLUDE_MAX][CEREBRO_MAX_MODULE_NAME_LEN+1];
  int cerebrod_metric_module_exclude_len;
  int cerebrod_metric_module_exclude_flag;
  char cerebrod_monitor_module_exclude[CEREBRO_CONFIG_MONITOR_MODULE_EXCLUDE_MAX][CEREBRO_MAX_MODULE_NAME_LEN+1];
  int cerebrod_monitor_module_exclude_len;
  int cerebrod_monitor_module_exclude_flag;
  char cerebrod_event_module_exclude[CEREBRO_CONFIG_EVENT_MODULE_EXCLUDE_MAX][CEREBRO_MAX_MODULE_NAME_LEN+1];
  int cerebrod_event_module_exclude_len;
  int cerebrod_event_module_exclude_flag;
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
  int cerebrod_gettimeofday_workaround;
  int cerebrod_gettimeofday_workaround_flag;
#if CEREBRO_DEBUG
  char cerebrod_alternate_hostname[CEREBRO_MAX_HOSTNAME_LEN+1];
  int cerebrod_alternate_hostname_flag;
#endif /* CEREBRO_DEBUG */
};

#endif /* _CEREBRO_CONFIG_H */
