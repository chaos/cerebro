/*****************************************************************************\
 *  $Id: cerebrod_config.h,v 1.4 2004-07-27 14:28:14 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_CONFIG_H
#define _CEREBROD_CONFIG_H

/* Later Configuration
 * - Max status allowed
 * - setuid
 */

#define CEREBROD_CONFIGFILE_DEFAULT             "/etc/cerebrod.conf"
#define CEREBROD_HEARTBEAT_FREQ_MIN_DEFAULT     10
#define CEREBROD_HEARTBEAT_FREQ_MAX_DEFAULT     20
#define CEREBROD_LISTEN_DEFAULT                 1
#define CEREBROD_SPEAK_DEFAULT                  1
#define CEREBROD_MCAST_IP_DEFAULT               "239.2.11.72"
#define CEREBROD_MCAST_PORT_DEFAULT             8650
#define CEREBROD_MCAST_LISTEN_THREADS_DEFAULT   2

struct cerebrod_config
{
  /* Set by the user on the command line */

  int debug;
  char *configfile;

  /* Set by the user in the configuration file */
  int listen;
  int speak;
  unsigned int heartbeat_freq_min;
  unsigned int heartbeat_freq_max;
  char *network_interface;
  char *mcast_ip;
  int mcast_port;
  unsigned int mcast_listen_threads;

  /* Determined by cerebrod based on configuration */
  char *multicast_interface;
};

void cerebrod_config_default(void);
void cerebrod_cmdline_parse(int argc, char **argv);
void cerebrod_config_parse(void);
void cerebrod_calculate_configuration(void);

#endif /* _CEREBROD_CONFIG_H */
