/*****************************************************************************\
 *  $Id: cerebrod_config.h,v 1.1.1.1 2004-07-02 22:31:29 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_CONFIG_H
#define _CEREBROD_CONFIG_H

#define CEREBROD_CONFIGFILE_DEFAULT             "/etc/cerebrod.conf"
#define CEREBROD_HEARTBEAT_FREQ_MIN_DEFAULT     10
#define CEREBROD_HEARTBEAT_FREQ_MAX_DEFAULT     20
#define CEREBROD_MCAST_IP_DEFAULT               "239.2.11.72"
#define CEREBROD_MCAST_PORT_DEFAULT             8650
#define CEREBROD_MCAST_LISTEN_THREADS_DEFAULT   2

struct cerebrod_config
{
  int debug;
  char *configfile;
  unsigned int heartbeat_freq_min;
  unsigned int heartbeat_freq_max;
  char *network_interface;
  char *mcast_ip;
  int mcast_port;
  unsigned int mcast_listen_threads;
};

void cerebrod_config_default(void);
void cerebrod_cmdline_parse(int argc, char **argv);
void cerebrod_config_parse(void);

#endif /* _CEREBROD_CONFIG_H */
