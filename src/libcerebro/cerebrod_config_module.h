/*****************************************************************************\
 *  $Id: cerebrod_config_module.h,v 1.1 2005-03-24 01:29:21 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_CONFIG_MODULE_H
#define _CEREBROD_CONFIG_MODULE_H

/*  
 * struct cerebrod_module_config
 *
 * passed to config module to load alternate default values
 */
struct cerebrod_module_config
{
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
};

/*
 * Cerebrod_config_load_default
 *
 * function prototype for config module function to alter default
 * configuration values
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebrod_config_load_default)(struct cerebrod_module_config *conf);

/*
 * struct cerebrod_config_module_info 
 * 
 * contains config module information and operations.  Required to be
 * defined in each config module.
 */
struct cerebrod_config_module_info
{
  char *config_module_name;
  Cerebrod_config_load_default load_default;
};

#endif /* _CEREBROD_CONFIG_MODULE_H */
