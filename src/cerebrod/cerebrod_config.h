/*****************************************************************************\
 *  $Id: cerebrod_config.h,v 1.62 2007-09-05 18:15:55 chu11 Exp $
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
 *  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
\*****************************************************************************/

#ifndef _CEREBROD_CONFIG_H
#define _CEREBROD_CONFIG_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <netinet/in.h>

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"

#define CEREBROD_DEBUG_DEFAULT                               0
#define CEREBROD_HEARTBEAT_FREQUENCY_MIN_DEFAULT             10
#define CEREBROD_HEARTBEAT_FREQUENCY_MAX_DEFAULT             20
#define CEREBROD_MESSAGE_SOURCE_PORT_DEFAULT                 8850
#define CEREBROD_MESSAGE_SOURCE_NETWORK_INTERFACE_DEFAULT    NULL
#define CEREBROD_MESSAGE_DESTINATION_PORT_DEFAULT            8851
#define CEREBROD_MESSAGE_DESTINATION_IP_DEFAULT              "239.2.11.72"
#define CEREBROD_MESSAGE_TTL_DEFAULT                         1
#define CEREBROD_SPEAK_DEFAULT                               1
#define CEREBROD_LISTEN_DEFAULT                              1
#define CEREBROD_LISTEN_THREADS_DEFAULT                      2
#define CEREBROD_LISTEN_PORTS_DEFAULT                        8851
#define CEREBROD_LISTEN_IPS_DEFAULT                          "239.2.11.72"
#define CEREBROD_LISTEN_NETWORK_INTERFACE_DEFAULT            NULL
#define CEREBROD_METRIC_CONTROLLER_DEFAULT                   1
#define CEREBROD_METRIC_SERVER_DEFAULT                       1
#define CEREBROD_METRIC_SERVER_PORT_DEFAULT                  CEREBRO_METRIC_SERVER_PORT
#define CEREBROD_EVENT_SERVER_DEFAULT                        1
#define CEREBROD_EVENT_SERVER_PORT_DEFAULT                   CEREBRO_EVENT_SERVER_PORT
#define CEREBROD_SPEAK_DEBUG_DEFAULT                         0
#define CEREBROD_LISTEN_DEBUG_DEFAULT                        0
#define CEREBROD_METRIC_CONTROLLER_DEBUG_DEFAULT             0
#define CEREBROD_METRIC_SERVER_DEBUG_DEFAULT                 0
#define CEREBROD_EVENT_SERVER_DEBUG_DEFAULT                  0

/*
 * struct cerebrod_config 
 *
 * configuration structure and data used by all of cerebrod.
 */
struct cerebrod_config
{
  /* Set by the user on the command line */
#if CEREBRO_DEBUG
  int debug;
  char *config_file;
#endif /* CEREBRO_DEBUG */

  /* Set by the user in the configuration file */
  int heartbeat_frequency_min;
  int heartbeat_frequency_max;
  int message_source_port;
  char *message_source_network_interface;
  int message_destination_port;
  char *message_destination_ip;
  int message_ttl;

  int speak;

  int listen;
  int listen_threads;
  int listen_ports[CEREBRO_MAX_LISTENERS];
  int listen_ports_len;
  char *listen_ips[CEREBRO_MAX_LISTENERS];
  int listen_ips_len;
  char *listen_network_interfaces[CEREBRO_MAX_LISTENERS];
  int listen_network_interfaces_len;
  int metric_controller;
  int metric_server;
  int metric_server_port;
  int event_server;
  int event_server_port;

#if CEREBRO_DEBUG
  int speak_debug;
  int listen_debug;
  int metric_controller_debug;
  int metric_server_debug;
  int event_server_debug;
#endif /* CEREBRO_DEBUG */

  /* Determined by cerebrod based on configuration */

  int destination_ip_is_multicast;
  int heartbeat_frequency_ranged;
  struct in_addr message_source_network_interface_in_addr;
  struct in_addr message_destination_ip_in_addr;
  int message_source_network_interface_index;
  int listen_ips_is_multicast[CEREBRO_MAX_LISTENERS];
  struct in_addr listen_ips_in_addr[CEREBRO_MAX_LISTENERS];
  struct in_addr listen_network_interfaces_in_addr[CEREBRO_MAX_LISTENERS];
  int listen_network_interfaces_index[CEREBRO_MAX_LISTENERS];
  int listen_len;
};

/*
 * cerebrod_config_setup
 * 
 * perform all cerebrod configuration.  Includes command line parsing,
 * config module loading, and configuration file parsing
 */
void cerebrod_config_setup(int argc, char **argv);

#endif /* _CEREBROD_CONFIG_H */
