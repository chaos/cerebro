/*****************************************************************************\
 *  $Id: cerebrod_config.h,v 1.23 2005-03-19 08:40:14 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_CONFIG_H
#define _CEREBROD_CONFIG_H

#include <netinet/in.h>

/* Later Configuration
 * - Max status allowed
 */

#define CEREBROD_DEBUG_DEFAULT                        0
#define CEREBROD_HEARTBEAT_FREQUENCY_MIN_DEFAULT      10
#define CEREBROD_HEARTBEAT_FREQUENCY_MAX_DEFAULT      20
#define CEREBROD_HEARTBEAT_SOURCE_PORT_DEFAULT        8650
#define CEREBROD_HEARTBEAT_DESTINATION_PORT_DEFAULT   8651
#define CEREBROD_HEARTBEAT_DESTINATION_IP_DEFAULT     "239.2.11.72"
#define CEREBROD_HEARTBEAT_NETWORK_INTERFACE_DEFAULT  NULL
#define CEREBROD_HEARTBEAT_TTL_DEFAULT                1
#define CEREBROD_SPEAK_DEFAULT                        1
#define CEREBROD_LISTEN_DEFAULT                       1
#define CEREBROD_LISTEN_THREADS_DEFAULT               2
#define CEREBROD_UPDOWN_SERVER_DEFAULT                1
#define CEREBROD_UPDOWN_SERVER_PORT_DEFAULT           8652
#define CEREBROD_CLUSTERLIST_MODULE_DEFAULT           NULL
#define CEREBROD_CLUSTERLIST_MODULE_OPTIONS_DEFAULT   NULL
#define CEREBROD_SPEAK_DEBUG_DEFAULT                  0
#define CEREBROD_LISTEN_DEBUG_DEFAULT                 0
#define CEREBROD_UPDOWN_SERVER_DEBUG_DEFAULT          0

/* Configuration
 *
 * Can be set by user:
 * heartbeat_frequency
 * - heartbeat frequency
 * - 1 val for fixed frequency
 * - 2 vals for range (val1 must be < val2)
 * heartbeat_source_port
 * - the port to speak from
 * heartbeat_destination_port
 * - the port to send heartbeats to
 * heartbeat_destination_ip
 * - to ip to send heartbeats to, may be remote IP or multicast
 * heartbeat_network_interface
 * - not specified - we pick an interface
 * - network interface - will be checked.
 * - ip address - will be checked
 * - ip address/subnet - will be found/checked
 * heartbeat_ttl
 * - num
 * speak
 * - on/off
 * listen
 * - on/off
 * listen_threads
 * - num
 * clusterlist_module
 * - can be any string to file
 * clusterlist_module_options
 * - key=val combinations
 *
 * updown_server
 * - on/off
 * updown_server_port
 * - num
 */

struct cerebrod_config
{
  /* Set by the user on the command line */
  int debug;
  char *configfile;
  char *configmodule;

  /* Set by the user in the configuration file */
  unsigned int heartbeat_frequency_min;
  unsigned int heartbeat_frequency_max;
  int heartbeat_source_port;
  int heartbeat_destination_port;
  char *heartbeat_destination_ip;
  char *heartbeat_network_interface;
  int heartbeat_ttl;

  int speak;

  int listen;
  int listen_threads;

  int updown_server;
  int updown_server_port;

  char *clusterlist_module;
  char **clusterlist_module_options;

  int speak_debug;
  int listen_debug;
  int updown_server_debug;

  /* Determined by cerebrod based on configuration */

  int multicast;
  int heartbeat_frequency_ranged;
  struct in_addr heartbeat_destination_ip_in_addr;
  struct in_addr heartbeat_network_interface_in_addr;
  int heartbeat_interface_index;

  char *clusterlist_module_file;
};

void cerebrod_config(int argc, char **argv);

typedef int (*Cerebrod_config_load_default)(struct cerebrod_config *conf);

struct cerebrod_config_module_info
{
  char *config_module_name;
};
 
struct cerebrod_config_module_ops
{
  Cerebrod_config_load_default load_default;
};


#endif /* _CEREBROD_CONFIG_H */
