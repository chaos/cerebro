/*****************************************************************************\
 *  $Id: cerebrod_config.h,v 1.10 2005-02-01 00:44:05 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_CONFIG_H
#define _CEREBROD_CONFIG_H

#include <netinet/in.h>

/* Later Configuration
 * - Max status allowed
 * - setuid
 * - listen interface?
 */

#define CEREBROD_CONFIGFILE_DEFAULT                  "/etc/cerebrod.conf"
#define CEREBROD_HEARTBEAT_FREQUENCY_MIN_DEFAULT     10
#define CEREBROD_HEARTBEAT_FREQUENCY_MAX_DEFAULT     20
#define CEREBROD_HEARTBEAT_DESTINATION_PORT_DEFAULT  8650
#define CEREBROD_HEARTBEAT_DESTINATION_IP_DEFAULT    "239.2.11.72"
#define CEREBROD_LISTEN_DEFAULT                      1
#define CEREBROD_SPEAK_DEFAULT                       1
#define CEREBROD_SPEAK_FROM_PORT_DEFAULT             8651
#define CEREBROD_SPEAK_TTL_DEFAULT                   1
#define CEREBROD_LISTEN_THREADS_DEFAULT              2

/* Configuration
 *
 * Can be set by user:
 * heartbeat_frequency
 * - heartbeat frequency
 * - 1 val for fixed frequency
 * - 2 vals for range (val1 must be < val2)
 * heartbeat_destination_port
 - - the port to send heartbeats to
 * heartbeat_destination_ip
 * - to ip to send heartbeats to, may be remote IP or multicast
 * listen
 * - on/off
 * speak
 * - on/off
 * speak_from_port
 * - the port to speak from
 * speak_from_network_interface
 * - not specified - we pick an interface
 * - network interface - will be checked.
 * - ip address - will be checked
 * - ip address/subnet - will be found/checked
 * speak_ttl
 * - num
 * listen_threads
 * - num
 *
 * Calculated by cerebrod:
 * multicast
 * - on/off
 * speak_to_in_addr
 * speak_from_in_addr
 * - both in network byte order
 * speak_from_interface_index;
 *
 */

struct cerebrod_config
{
  /* Set by the user on the command line */
  int debug;
  char *configfile;

  /* Set by the user in the configuration file */
  unsigned int heartbeat_frequency_min;
  unsigned int heartbeat_frequency_max;
  int heartbeat_destination_port;
  char *heartbeat_destination_ip;

  int listen;
  int speak;
  int speak_from_port;
  char *speak_from_network_interface;
  int speak_ttl;
  int listen_threads;

  /* Determined by cerebrod based on configuration */
  int multicast;
  int heartbeat_frequency_ranged;
  struct in_addr heartbeat_destination_in_addr;
  struct in_addr speak_from_in_addr;
  int speak_from_interface_index;
};

void cerebrod_config(int argc, char **argv);

#endif /* _CEREBROD_CONFIG_H */
