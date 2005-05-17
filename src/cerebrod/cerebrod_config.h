/*****************************************************************************\
 *  $Id: cerebrod_config.h,v 1.40 2005-05-17 20:53:59 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_CONFIG_H
#define _CEREBROD_CONFIG_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <netinet/in.h>

#include "cerebro/cerebro_status_protocol.h"
#include "cerebro/cerebro_updown_protocol.h"

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
#define CEREBROD_UPDOWN_SERVER_DEFAULT                1
#define CEREBROD_UPDOWN_SERVER_PORT_DEFAULT           CEREBRO_UPDOWN_SERVER_PORT
#define CEREBROD_STATUS_SERVER_DEFAULT                1
#define CEREBROD_STATUS_SERVER_PORT_DEFAULT           CEREBRO_STATUS_SERVER_PORT
#define CEREBROD_CLUSTERLIST_MODULE_DEFAULT           NULL
#define CEREBROD_CLUSTERLIST_MODULE_OPTIONS_DEFAULT   NULL
#define CEREBROD_SPEAK_DEBUG_DEFAULT                  0
#define CEREBROD_LISTEN_DEBUG_DEFAULT                 0
#define CEREBROD_UPDOWN_SERVER_DEBUG_DEFAULT          0
#define CEREBROD_STATUS_SERVER_DEBUG_DEFAULT          0

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
  unsigned int heartbeat_frequency_min;
  unsigned int heartbeat_frequency_max;
  unsigned int heartbeat_source_port;
  unsigned int heartbeat_destination_port;
  char *heartbeat_destination_ip;
  char *heartbeat_network_interface;
  int heartbeat_ttl;

  int speak;

  int listen;
  int listen_threads;

  int updown_server;
  unsigned int updown_server_port;
  int status_server;
  unsigned int status_server_port;

#if CEREBRO_DEBUG
  int speak_debug;
  int listen_debug;
  int updown_server_debug;
  int status_server_debug;
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
