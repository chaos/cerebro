/*****************************************************************************\
 *  $Id: cerebrod_config.h,v 1.55 2006-06-29 23:48:41 chu11 Exp $
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

#ifndef _CEREBROD_CONFIG_H
#define _CEREBROD_CONFIG_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <netinet/in.h>

#include "cerebro.h"

#define CEREBROD_DEBUG_DEFAULT                        0
#define CEREBROD_HEARTBEAT_FREQUENCY_MIN_DEFAULT      10
#define CEREBROD_HEARTBEAT_FREQUENCY_MAX_DEFAULT      20
#define CEREBROD_HEARTBEAT_SOURCE_PORT_DEFAULT        8850
#define CEREBROD_HEARTBEAT_SOURCE_NETWORK_INTERFACE_DEFAULT  NULL
#define CEREBROD_HEARTBEAT_DESTINATION_PORT_DEFAULT   8851
#define CEREBROD_HEARTBEAT_DESTINATION_IP_DEFAULT     "239.2.11.72"
#define CEREBROD_HEARTBEAT_TTL_DEFAULT                1
#define CEREBROD_SPEAK_DEFAULT                        1
#define CEREBROD_LISTEN_DEFAULT                       1
#define CEREBROD_LISTEN_THREADS_DEFAULT               2
#define CEREBROD_METRIC_CONTROLLER_DEFAULT            1
#define CEREBROD_METRIC_SERVER_DEFAULT                1
#define CEREBROD_METRIC_SERVER_PORT_DEFAULT           CEREBRO_METRIC_SERVER_PORT
#define CEREBROD_METRIC_MAX_DEFAULT                   8
#define CEREBROD_MONITOR_MAX_DEFAULT                  8
#define CEREBROD_SPEAK_DEBUG_DEFAULT                  0
#define CEREBROD_LISTEN_DEBUG_DEFAULT                 0
#define CEREBROD_METRIC_CONTROLLER_DEBUG_DEFAULT      0
#define CEREBROD_METRIC_SERVER_DEBUG_DEFAULT          0

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
  int heartbeat_source_port;
  char *heartbeat_source_network_interface;
  int heartbeat_destination_port;
  char *heartbeat_destination_ip;
  int heartbeat_ttl;

  int speak;

  int listen;
  int listen_threads;

  int metric_controller;
  int metric_server;
  int metric_server_port;
  int metric_max;
  int monitor_max;

#if CEREBRO_DEBUG
  int speak_debug;
  int listen_debug;
  int metric_controller_debug;
  int metric_server_debug;
#endif /* CEREBRO_DEBUG */

  /* Determined by cerebrod based on configuration */

  int destination_ip_is_multicast;
  int heartbeat_frequency_ranged;
  struct in_addr heartbeat_source_network_interface_in_addr;
  struct in_addr heartbeat_destination_ip_in_addr;
  int heartbeat_source_network_interface_index;
};

/*
 * cerebrod_config_setup
 * 
 * perform all cerebrod configuration.  Includes command line parsing,
 * config module loading, and configuration file parsing
 */
void cerebrod_config_setup(int argc, char **argv);

#endif /* _CEREBROD_CONFIG_H */
