/*****************************************************************************\
 *  $Id: cerebrod_config.h,v 1.6 2005-01-04 23:45:53 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_CONFIG_H
#define _CEREBROD_CONFIG_H

#include <netinet/in.h>

/* Later Configuration
 * - max cluster nodes?
 * - Max status allowed
 * - setuid
 * - ttl
 * - listen port
 * - listen interface
 */

#define CEREBROD_CONFIGFILE_DEFAULT                  "/etc/cerebrod.conf"
#define CEREBROD_HEARTBEAT_FREQUENCY_MIN_DEFAULT     10
#define CEREBROD_HEARTBEAT_FREQUENCY_MAX_DEFAULT     20
#define CEREBROD_LISTEN_DEFAULT                      1
#define CEREBROD_SPEAK_DEFAULT                       1
#define CEREBROD_SPEAK_TO_IP_DEFAULT                 "239.2.11.72"
#define CEREBROD_SPEAK_FROM_PORT_DEFAULT             8650
#define CEREBROD_LISTEN_THREADS_DEFAULT              2

/* Configuration
 *
 * Can be set by user:
 * heartbeat_frequency
 * - heartbeat frequency
 * - 1 val for fixed frequency
 * - 2 vals for range (val1 must be < val2)
 * listen
 * - on/off
 * speak
 * - on/off
 * speak_to_ip
 * - to speak to, may be remote IP or multicast
 * speak_from_port
 * - the port
 * speak_from_network_interface
 * - not specified - we pick an interface
 * - network interface - will be checked.
 * - ip address - will be checked
 * - ip address/subnet - will be found/checked
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
  int listen;
  int speak;
  char *speak_to_ip;
  int speak_from_port;
  char *speak_from_network_interface;
  unsigned int listen_threads;

  /* Determined by cerebrod based on configuration */
  int multicast;
  int heartbeat_frequency_ranged;
  struct in_addr speak_to_in_addr;
  struct in_addr speak_from_in_addr;
  int speak_from_interface_index;
};

void cerebrod_config(int argc, char **argv);

#endif /* _CEREBROD_CONFIG_H */
