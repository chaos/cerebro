/*****************************************************************************\
 *  $Id: cerebrod_config.h,v 1.45 2005-06-08 00:30:38 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_CONFIG_H
#define _CEREBROD_CONFIG_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <netinet/in.h>

#include "cerebro_metric_protocol.h"

/*
 * Cerebrod default configuration values
 */

#define CEREBROD_DEBUG_DEFAULT                        0
#define CEREBROD_HEARTBEAT_FREQUENCY_MIN_DEFAULT      10
#define CEREBROD_HEARTBEAT_FREQUENCY_MAX_DEFAULT      20
#define CEREBROD_HEARTBEAT_SOURCE_PORT_DEFAULT        8850
#define CEREBROD_HEARTBEAT_DESTINATION_PORT_DEFAULT   8851
#define CEREBROD_HEARTBEAT_DESTINATION_IP_DEFAULT     "239.2.11.72"
#define CEREBROD_HEARTBEAT_NETWORK_INTERFACE_DEFAULT  NULL
#define CEREBROD_HEARTBEAT_TTL_DEFAULT                1
#define CEREBROD_SPEAK_DEFAULT                        1
#define CEREBROD_LISTEN_DEFAULT                       1
#define CEREBROD_LISTEN_THREADS_DEFAULT               2
#define CEREBROD_METRIC_SERVER_DEFAULT                1
#define CEREBROD_METRIC_SERVER_PORT_DEFAULT           CEREBRO_METRIC_SERVER_PORT
#define CEREBROD_METRIC_MAX_DEFAULT                   4
#define CEREBROD_CLUSTERLIST_MODULE_DEFAULT           NULL
#define CEREBROD_CLUSTERLIST_MODULE_OPTIONS_DEFAULT   NULL
#define CEREBROD_SPEAK_DEBUG_DEFAULT                  0
#define CEREBROD_LISTEN_DEBUG_DEFAULT                 0
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
#endif /* CEREBRO_DEBUG */
  char *config_file;

  /* Set by the user in the configuration file */
  int heartbeat_frequency_min;
  int heartbeat_frequency_max;
  int heartbeat_source_port;
  int heartbeat_destination_port;
  char *heartbeat_destination_ip;
  char *heartbeat_network_interface;
  int heartbeat_ttl;

  int speak;

  int listen;
  int listen_threads;

  int metric_server;
  int metric_server_port;
  int metric_max;

#if CEREBRO_DEBUG
  int speak_debug;
  int listen_debug;
  int metric_server_debug;
#endif /* CEREBRO_DEBUG */

  /* Determined by cerebrod based on configuration */

  int multicast;
  int heartbeat_frequency_ranged;
  struct in_addr heartbeat_destination_ip_in_addr;
  struct in_addr heartbeat_network_interface_in_addr;
  int heartbeat_interface_index;
};

/*
 * cerebrod_config_setup
 * 
 * perform all cerebrod configuration.  Includes command line parsing,
 * config module loading, and configuration file parsing
 */
void cerebrod_config_setup(int argc, char **argv);

#endif /* _CEREBROD_CONFIG_H */
