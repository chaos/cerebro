/*****************************************************************************\
 *  $Id: config_util.c,v 1.15.2.2 2006-10-30 22:02:14 chu11 Exp $
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

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"

#include "config_util.h"

#include "conffile.h"
#include "config_module.h"
#include "debug.h"

#if CEREBRO_DEBUG
char *config_debug_config_file = NULL;
int config_debug_output = 0;
#endif /* CEREBRO_DEBUG */

/* 
 * _load_config_module
 *
 * Find and load config module
 *
 * Returns data in structure and 0 on success, -1 on error
 */
static int
_load_config_module(struct cerebro_config *conf, unsigned int *errnum)
{
  int rv = -1;
  config_module_t config_handle = NULL;

  if (!(config_handle = config_module_load()))
    {
      if (errnum)
        *errnum = CEREBRO_ERR_CONFIG_MODULE;
      goto cleanup;
    }

  if (config_module_setup(config_handle) < 0)
    {
      if (errnum)
        *errnum = CEREBRO_ERR_CONFIG_MODULE;
      goto cleanup;
    }

#if CEREBRO_DEBUG  
  if (config_debug_output)
    {
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Cerebro Config Configuration:\n");
      fprintf(stderr, "* -----------------------\n");
      fprintf(stderr, "* Loading config from module: %s\n",
              config_module_name(config_handle));
      fprintf(stderr, "**************************************\n");
    }
#endif /* CEREBRO_DEBUG */

  if (config_module_load_config(config_handle, conf) < 0)
    {
      if (errnum)
        *errnum = CEREBRO_ERR_CONFIG_MODULE;
      goto cleanup;
    }

  rv = 0;
 cleanup:
  config_module_cleanup(config_handle);
  config_module_unload(config_handle);
  return rv;
}

/*
 * _cb_heartbeat_freq
 *
 * conffile callback function that parses and stores heartbeat
 * configuration data
 *
 * Returns 0 on success, -1 on error
 */
static int
_cb_cerebrod_heartbeat_freq(conffile_t cf, struct conffile_data *data,
			    char *optionname, int option_type, void *option_ptr,
			    int option_data, void *app_ptr, int app_data)
{
  struct cerebro_config *conf;

  if (!option_ptr)
    {
      conffile_seterrnum(cf, CONFFILE_ERR_PARAMETERS);
      return -1;
    }

  conf = (struct cerebro_config *)option_ptr;
  if (data->intlist_len == 1)
    {
      conf->cerebrod_heartbeat_frequency_min = data->intlist[0];
      conf->cerebrod_heartbeat_frequency_max = 0;
    }
  else if (data->intlist_len == 2)
    {
      if (data->intlist[0] > data->intlist[1])
        {
          conffile_seterrnum(cf, CONFFILE_ERR_PARAMETERS);
          return -1;
        }
      conf->cerebrod_heartbeat_frequency_min = data->intlist[0];
      conf->cerebrod_heartbeat_frequency_max = data->intlist[1];
    }
  else
    {
      conffile_seterrnum(cf, CONFFILE_ERR_PARSE_ARG_TOOMANY);
      return -1;
    }

  return 0;
}

/*
 * _cb_cerebro_hostnames
 *
 * callback function that parses and stores a list of hostnames
 *
 * Returns 0 on success, -1 on error
 */
static int
_cb_cerebro_hostnames(conffile_t cf, struct conffile_data *data,
		      char *optionname, int option_type, void *option_ptr,
		      int option_data, void *app_ptr, int app_data)
{
  struct cerebro_config *conf;

  if (!option_ptr)
    {
      conffile_seterrnum(cf, CONFFILE_ERR_PARAMETERS);
      return -1;
    }
  
  conf = (struct cerebro_config *)option_ptr;
  
  if (data->stringlist_len > 0)
    {
      int i;

      if (data->stringlist_len > CEREBRO_CONFIG_HOSTNAMES_MAX)
	{
          conffile_seterrnum(cf, CONFFILE_ERR_PARSE_ARG_TOOMANY);
          return -1;
	}
      
      for (i = 0; i < data->stringlist_len; i++)
        {
	  if (strlen(data->stringlist[i]) > CEREBRO_MAX_NODENAME_LEN)
	    {
	      conffile_seterrnum(cf, CONFFILE_ERR_PARSE_OVERFLOW_ARGLEN);
	      return -1;
	    }

	  strcpy(conf->cerebro_hostnames[i], data->stringlist[i]);
        }
      conf->cerebro_hostnames_len = data->stringlist_len;
    }

  return 0;
}

/*
 * _cb_cerebrod_listen_ports
 *
 * callback function that parses and stores a list of ports
 *
 * Returns 0 on success, -1 on error
 */
static int
_cb_cerebrod_listen_ports(conffile_t cf, struct conffile_data *data,
                          char *optionname, int option_type, void *option_ptr,
                          int option_data, void *app_ptr, int app_data)
{
  struct cerebro_config *conf;

  if (!option_ptr)
    {
      conffile_seterrnum(cf, CONFFILE_ERR_PARAMETERS);
      return -1;
    }
  
  conf = (struct cerebro_config *)option_ptr;
  
  if (data->intlist_len > 0)
    {
      int i;

      if (data->intlist_len > CEREBRO_MAX_LISTENERS)
	{
          conffile_seterrnum(cf, CONFFILE_ERR_PARSE_ARG_TOOMANY);
          return -1;
	}
      
      for (i = 0; i < data->intlist_len; i++)
        conf->cerebrod_listen_ports[i] = data->intlist[i];
      conf->cerebrod_listen_ports_len = data->intlist_len;
    }

  return 0;
}

/*
 * _cb_cerebrod_listen_ips
 *
 * callback function that parses and stores a list of ips
 *
 * Returns 0 on success, -1 on error
 */
static int
_cb_cerebrod_listen_ips(conffile_t cf, struct conffile_data *data,
                        char *optionname, int option_type, void *option_ptr,
                        int option_data, void *app_ptr, int app_data)
{
  struct cerebro_config *conf;

  if (!option_ptr)
    {
      conffile_seterrnum(cf, CONFFILE_ERR_PARAMETERS);
      return -1;
    }
  
  conf = (struct cerebro_config *)option_ptr;
  
  if (data->stringlist_len > 0)
    {
      int i;

      if (data->stringlist_len > CEREBRO_MAX_LISTENERS)
	{
          conffile_seterrnum(cf, CONFFILE_ERR_PARSE_ARG_TOOMANY);
          return -1;
	}
      
      for (i = 0; i < data->stringlist_len; i++)
        {
	  if (strlen(data->stringlist[i]) > CEREBRO_MAX_IPADDR_LEN)
	    {
	      conffile_seterrnum(cf, CONFFILE_ERR_PARSE_OVERFLOW_ARGLEN);
	      return -1;
	    }
          
	  strcpy(conf->cerebrod_listen_ips[i], data->stringlist[i]);
        }
      conf->cerebrod_listen_ips_len = data->stringlist_len;
    }

  return 0;
}

/*
 * _cb_cerebrod_listen_network_interfaces
 *
 * callback function that parses and stores a list of network_interfaces
 *
 * Returns 0 on success, -1 on error
 */
static int
_cb_cerebrod_listen_network_interfaces(conffile_t cf, struct conffile_data *data,
                        char *optionname, int option_type, void *option_ptr,
                        int option_data, void *app_ptr, int app_data)
{
  struct cerebro_config *conf;

  if (!option_ptr)
    {
      conffile_seterrnum(cf, CONFFILE_ERR_PARAMETERS);
      return -1;
    }
  
  conf = (struct cerebro_config *)option_ptr;
  
  if (data->stringlist_len > 0)
    {
      int i;
      
      if (data->stringlist_len > CEREBRO_MAX_LISTENERS)
	{
          conffile_seterrnum(cf, CONFFILE_ERR_PARSE_ARG_TOOMANY);
          return -1;
	}
      
      for (i = 0; i < data->stringlist_len; i++)
        {
	  if (strlen(data->stringlist[i]) > CEREBRO_MAX_IPADDR_LEN)
	    {
	      conffile_seterrnum(cf, CONFFILE_ERR_PARSE_OVERFLOW_ARGLEN);
	      return -1;
	    }
          
	  strcpy(conf->cerebrod_listen_network_interfaces[i], data->stringlist[i]);
        }
      conf->cerebrod_listen_network_interfaces_len = data->stringlist_len;
    }
  
  return 0;
}

/* 
 * _load_config_file
 *
 * Read and load configuration file
 *
 * Returns data in structure and 0 on success, -1 on error
 */
static int
_load_config_file(struct cerebro_config *conf, unsigned int *errnum)
{
  char *config_file = NULL;

  struct conffile_option options[] =
    {
      /*
       * Libcerebro configuration
       */
      {
	"cerebro_hostnames", 
	CONFFILE_OPTION_LIST_STRING, 
	-1,
	_cb_cerebro_hostnames, 
	1, 
	0, 
	&(conf->cerebro_hostnames_flag), 
	conf, 
	0
      },
      {
	"cerebro_port", 
	CONFFILE_OPTION_INT, 
	-1,
	conffile_int, 
	1, 
	0, 
	&(conf->cerebro_port_flag), 
	&(conf->cerebro_port), 
	0
      },
      {
	"cerebro_timeout_len", 
	CONFFILE_OPTION_INT, 
	-1,
	conffile_int, 
	1, 
	0, 
	&(conf->cerebro_timeout_len_flag),
	&(conf->cerebro_timeout_len), 
	0
      },
      {
	"cerebro_flags", 
	CONFFILE_OPTION_INT, 
	-1,
	conffile_int, 
	1, 
	0, 
	&(conf->cerebro_flags_flag),
	&(conf->cerebro_flags), 
	0
      },
      /*
       * Cerebrod configuration
       */
      {
	"cerebrod_heartbeat_frequency", 
	CONFFILE_OPTION_LIST_INT, 
	-1,
	_cb_cerebrod_heartbeat_freq, 
	1, 
	0, 
	&(conf->cerebrod_heartbeat_frequency_flag),
	conf, 
	0
      },
      {
	"cerebrod_heartbeat_source_port",
	CONFFILE_OPTION_INT, 
	-1,
	conffile_int, 
	1, 
	0, 
	&(conf->cerebrod_heartbeat_source_port_flag),
	&(conf->cerebrod_heartbeat_source_port), 
	0
      },
      {
	"cerebrod_heartbeat_source_network_interface", 
	CONFFILE_OPTION_STRING, 
	-1,
	conffile_string, 
	1, 
	0, 
	&(conf->cerebrod_heartbeat_source_network_interface_flag),
	conf->cerebrod_heartbeat_source_network_interface, 
	CEREBRO_MAX_NETWORK_INTERFACE_LEN
      },
      {
	"cerebrod_heartbeat_destination_port", 
	CONFFILE_OPTION_INT, 
	-1,
	conffile_int, 
	1, 
	0, 
	&(conf->cerebrod_heartbeat_destination_port_flag),
	&(conf->cerebrod_heartbeat_destination_port), 
	0
      },
      {
	"cerebrod_heartbeat_destination_ip", 
	CONFFILE_OPTION_STRING, 
	-1,
	conffile_string, 
	1, 
	0, 
	&(conf->cerebrod_heartbeat_destination_ip_flag),
	conf->cerebrod_heartbeat_destination_ip, 
	CEREBRO_MAX_IPADDR_LEN
      },
      {
	"cerebrod_heartbeat_ttl", 
	CONFFILE_OPTION_INT, 
	-1,
	conffile_int, 
	1, 
	0, 
	&(conf->cerebrod_heartbeat_ttl_flag),
	&(conf->cerebrod_heartbeat_ttl), 
	0
      },
      {
	"cerebrod_speak", 
	CONFFILE_OPTION_BOOL, 
	-1,
	conffile_bool, 
	1, 
	0, 
	&(conf->cerebrod_speak_flag),
	&conf->cerebrod_speak, 
	0
      },
      {
	"cerebrod_listen", 
	CONFFILE_OPTION_BOOL, 
	-1,
	conffile_bool, 
	1, 
	0, 
	&(conf->cerebrod_listen_flag),
	&conf->cerebrod_listen, 
	0
      },
      {
	"cerebrod_listen_threads", 
	CONFFILE_OPTION_INT, 
	-1,
	conffile_int, 
	1, 
	0, 
	&(conf->cerebrod_listen_threads_flag),
	&(conf->cerebrod_listen_threads), 
	0
      },
      {
        "cerebrod_listen_ports",
        CONFFILE_OPTION_LIST_INT,
        -1,
        _cb_cerebrod_listen_ports,
        1,
        0,
        &(conf->cerebrod_listen_ports_flag),
        conf,
        0,
      },
      {
        "cerebrod_listen_ips",
        CONFFILE_OPTION_LIST_STRING,
        -1,
        _cb_cerebrod_listen_ips,
        1,
        0,
        &(conf->cerebrod_listen_ips_flag),
        conf,
        0,
      },
      {
        "cerebrod_listen_network_interfaces",
        CONFFILE_OPTION_LIST_STRING,
        -1,
        _cb_cerebrod_listen_network_interfaces,
        1,
        0,
        &(conf->cerebrod_listen_network_interfaces_flag),
        conf,
        0,
      },
      {
	"cerebrod_metric_controller", 
	CONFFILE_OPTION_BOOL, 
	-1,
	conffile_bool, 
	1, 
	0, 
	&(conf->cerebrod_metric_controller_flag),
	&conf->cerebrod_metric_controller, 
	0
      },
      {
	"cerebrod_metric_server", 
	CONFFILE_OPTION_BOOL, 
	-1,
	conffile_bool, 
	1, 
	0, 
	&(conf->cerebrod_metric_server_flag),
	&conf->cerebrod_metric_server, 
	0
      },
      {
	"cerebrod_metric_server_port", 
	CONFFILE_OPTION_INT, 
	-1,
	conffile_int, 
	1, 
	0, 
	&(conf->cerebrod_metric_server_port_flag),
	&(conf->cerebrod_metric_server_port), 
	0
      },
      {
	"cerebrod_event_server", 
	CONFFILE_OPTION_BOOL, 
	-1,
	conffile_bool, 
	1, 
	0, 
	&(conf->cerebrod_event_server_flag),
	&conf->cerebrod_event_server, 
	0
      },
      {
	"cerebrod_event_server_port", 
	CONFFILE_OPTION_INT, 
	-1,
	conffile_int, 
	1, 
	0, 
	&(conf->cerebrod_event_server_port_flag),
	&(conf->cerebrod_event_server_port), 
	0
      },
#if CEREBRO_DEBUG
      {
	"cerebrod_speak_debug", 
	CONFFILE_OPTION_BOOL, 
	-1,
	conffile_bool, 
	1, 
	0, 
	&(conf->cerebrod_speak_debug_flag),
	&conf->cerebrod_speak_debug, 
	0
      },
      {
	"cerebrod_listen_debug", 
	CONFFILE_OPTION_BOOL, 
	-1,
	conffile_bool, 
	1, 
	0, 
	&(conf->cerebrod_listen_debug_flag),
	&conf->cerebrod_listen_debug, 
	0
      },
      {
	"cerebrod_metric_controller_debug", 
	CONFFILE_OPTION_BOOL, 
	-1,
	conffile_bool, 
	1, 
	0, 
	&(conf->cerebrod_metric_controller_debug_flag),
	&conf->cerebrod_metric_controller_debug, 
	0
      },
      {
	"cerebrod_metric_server_debug", 
	CONFFILE_OPTION_BOOL, 
	-1,
	conffile_bool, 
	1, 
	0, 
	&(conf->cerebrod_metric_server_debug_flag),
	&conf->cerebrod_metric_server_debug, 
	0
      },
      {
	"cerebrod_event_server_debug", 
	CONFFILE_OPTION_BOOL, 
	-1,
	conffile_bool, 
	1, 
	0, 
	&(conf->cerebrod_event_server_debug_flag),
	&conf->cerebrod_event_server_debug, 
	0
      },
#endif /* CEREBRO_DEBUG */
    };
  conffile_t cf = NULL;
  int num;
  
  if (!(cf = conffile_handle_create()))
    {
      CEREBRO_DBG(("conffile_handle_create"));
      if (errnum)
        *errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }
  
#if CEREBRO_DEBUG
  if (!config_debug_config_file)
    config_file = CEREBRO_CONFIG_FILE_DEFAULT;
  else
    config_file = config_debug_config_file;
#else  /* !NDEBUG */
  config_file = CEREBRO_CONFIG_FILE_DEFAULT;
#endif /* !NDEBUG */

  memset(conf, '\0', sizeof(struct cerebro_config));
  num = sizeof(options)/sizeof(struct conffile_option);
  if (conffile_parse(cf, config_file, options, num, NULL, 0, 0) < 0)
    {
      char buf[CONFFILE_MAX_ERRMSGLEN];

      /* Its not an error if the default configuration file doesn't exist */
      if (!strcmp(config_file, CEREBRO_CONFIG_FILE_DEFAULT) 
	  && conffile_errnum(cf) == CONFFILE_ERR_EXIST)
	goto out;

      if (conffile_errmsg(cf, buf, CONFFILE_MAX_ERRMSGLEN) < 0)
	CEREBRO_DBG(("conffile_parse: %d", conffile_errnum(cf)));
      else
	CEREBRO_DBG(("conffile_parse: %s", buf));
	
      if (errnum)
        *errnum = CEREBRO_ERR_CONFIG_FILE;
      goto cleanup;
    }
  
 out:
  conffile_handle_destroy(cf);
  return 0;
  
 cleanup:
  memset(conf, '\0', sizeof(struct cerebro_config));
  conffile_handle_destroy(cf);
  return -1;
}

/* 
 * _set_cerebro_config
 *
 * Set the dest cerebro_config based on its current settings and the
 * settings in the src.
 *
 * Returns 0 on success, -1 on error
 */
static int
_set_cerebro_config(struct cerebro_config *dest, 
                    struct cerebro_config *src,
                    unsigned int *errnum)
{
  if (!dest || !src)
    {
      CEREBRO_DBG(("invalid parameters"));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (!dest->cerebro_hostnames_flag && src->cerebro_hostnames_flag)
    {
      int i;
      for (i = 0; i < src->cerebro_hostnames_len; i++)
	strcpy(dest->cerebro_hostnames[i], src->cerebro_hostnames[i]);
      dest->cerebro_hostnames_len = src->cerebro_hostnames_len;
      dest->cerebro_hostnames_flag++;
    }

  if (!dest->cerebro_port_flag && src->cerebro_port_flag)
    {
      dest->cerebro_port = src->cerebro_port;
      dest->cerebro_port_flag++;
    }

  if (!dest->cerebro_timeout_len && src->cerebro_timeout_len_flag)
    {
      dest->cerebro_timeout_len = src->cerebro_timeout_len;
      dest->cerebro_timeout_len_flag++;
    }

  if (!dest->cerebro_flags_flag && src->cerebro_flags_flag)
    {
      dest->cerebro_flags = src->cerebro_flags;
      dest->cerebro_flags_flag++;
    }

  if (!dest->cerebrod_heartbeat_frequency_flag 
      && src->cerebrod_heartbeat_frequency_flag)
    {
      dest->cerebrod_heartbeat_frequency_min = src->cerebrod_heartbeat_frequency_min;
      dest->cerebrod_heartbeat_frequency_max = src->cerebrod_heartbeat_frequency_max;
      dest->cerebrod_heartbeat_frequency_flag++;
    }

  if (!dest->cerebrod_heartbeat_source_port_flag
      && src->cerebrod_heartbeat_source_port_flag)
    {
      dest->cerebrod_heartbeat_source_port = src->cerebrod_heartbeat_source_port;
      dest->cerebrod_heartbeat_source_port_flag++;
    }

  if (!dest->cerebrod_heartbeat_source_network_interface_flag
      && src->cerebrod_heartbeat_source_network_interface_flag)
    {
      strcpy(dest->cerebrod_heartbeat_source_network_interface, 
             src->cerebrod_heartbeat_source_network_interface);
      dest->cerebrod_heartbeat_source_network_interface_flag++;
    }

  if (!dest->cerebrod_heartbeat_destination_port_flag
      && src->cerebrod_heartbeat_destination_port_flag)
    {
      dest->cerebrod_heartbeat_destination_port = src->cerebrod_heartbeat_destination_port;
      dest->cerebrod_heartbeat_destination_port_flag++;
    }

  if (!dest->cerebrod_heartbeat_destination_ip_flag
      && src->cerebrod_heartbeat_destination_ip_flag)
    {
      strcpy(dest->cerebrod_heartbeat_destination_ip, src->cerebrod_heartbeat_destination_ip);
      dest->cerebrod_heartbeat_destination_ip_flag++;
    }

  if (!dest->cerebrod_heartbeat_ttl_flag && src->cerebrod_heartbeat_ttl_flag)
    {
      dest->cerebrod_heartbeat_ttl = src->cerebrod_heartbeat_ttl;
      dest->cerebrod_heartbeat_ttl_flag++;
    }

  if (!dest->cerebrod_speak_flag && src->cerebrod_speak_flag)
    {
      dest->cerebrod_speak = src->cerebrod_speak;
      dest->cerebrod_speak_flag++;
    }

  if (!dest->cerebrod_listen_flag && src->cerebrod_listen_flag)
    {
      dest->cerebrod_listen = src->cerebrod_listen;
      dest->cerebrod_listen_flag++;
    }

  if (!dest->cerebrod_listen_threads_flag && src->cerebrod_listen_threads_flag)
    {
      dest->cerebrod_listen_threads = src->cerebrod_listen_threads;
      dest->cerebrod_listen_threads_flag++;
    }

  if (!dest->cerebrod_listen_ports_flag && src->cerebrod_listen_ports_flag)
    {
      int i;
      for (i = 0; i < src->cerebrod_listen_ports_len; i++)
	dest->cerebrod_listen_ports[i] =  src->cerebrod_listen_ports[i];
      dest->cerebrod_listen_ports_len = src->cerebrod_listen_ports_len;
      dest->cerebrod_listen_ports_flag++;
    }

  if (!dest->cerebrod_listen_ips_flag && src->cerebrod_listen_ips_flag)
    {
      int i;
      for (i = 0; i < src->cerebrod_listen_ips_len; i++)
	strcpy(dest->cerebrod_listen_ips[i], src->cerebrod_listen_ips[i]);
      dest->cerebrod_listen_ips_len = src->cerebrod_listen_ips_len;
      dest->cerebrod_listen_ips_flag++;
    }

  if (!dest->cerebrod_listen_network_interfaces_flag && src->cerebrod_listen_network_interfaces_flag)
    {
      int i;
      for (i = 0; i < src->cerebrod_listen_network_interfaces_len; i++)
	strcpy(dest->cerebrod_listen_network_interfaces[i], src->cerebrod_listen_network_interfaces[i]);
      dest->cerebrod_listen_network_interfaces_len = src->cerebrod_listen_network_interfaces_len;
      dest->cerebrod_listen_network_interfaces_flag++;
    }

  if (!dest->cerebrod_metric_controller_flag && src->cerebrod_metric_controller_flag)
    {
      dest->cerebrod_metric_controller = src->cerebrod_metric_controller;
      dest->cerebrod_metric_controller_flag++;
    }

  if (!dest->cerebrod_metric_server_flag && src->cerebrod_metric_server_flag)
    {
      dest->cerebrod_metric_server = src->cerebrod_metric_server;
      dest->cerebrod_metric_server_flag++;
    }

  if (!dest->cerebrod_metric_server_port_flag
      && src->cerebrod_metric_server_port_flag) 
    {
      dest->cerebrod_metric_server_port = src->cerebrod_metric_server_port;
      dest->cerebrod_metric_server_port_flag++;
    }

  if (!dest->cerebrod_event_server_flag && src->cerebrod_event_server_flag)
    {
      dest->cerebrod_event_server = src->cerebrod_event_server;
      dest->cerebrod_event_server_flag++;
    }

  if (!dest->cerebrod_event_server_port_flag
      && src->cerebrod_event_server_port_flag) 
    {
      dest->cerebrod_event_server_port = src->cerebrod_event_server_port;
      dest->cerebrod_event_server_port_flag++;
    }

#if CEREBRO_DEBUG
  if (!dest->cerebrod_speak_debug_flag && src->cerebrod_speak_debug_flag)
    {
      dest->cerebrod_speak_debug = src->cerebrod_speak_debug;
      dest->cerebrod_speak_debug_flag++;
    }

  if (!dest->cerebrod_listen_debug_flag && src->cerebrod_listen_debug_flag)
    {
      dest->cerebrod_listen_debug = src->cerebrod_listen_debug;
      dest->cerebrod_listen_debug_flag++;
    }

  if (!dest->cerebrod_metric_controller_debug_flag
      && src->cerebrod_metric_controller_debug_flag)
    {
      dest->cerebrod_metric_controller_debug = src->cerebrod_metric_controller_debug;
      dest->cerebrod_metric_controller_debug_flag++;
    }

  if (!dest->cerebrod_metric_server_debug_flag
      && src->cerebrod_metric_server_debug_flag)
    {
      dest->cerebrod_metric_server_debug = src->cerebrod_metric_server_debug;
      dest->cerebrod_metric_server_debug_flag++;
    }

  if (!dest->cerebrod_event_server_debug_flag
      && src->cerebrod_event_server_debug_flag)
    {
      dest->cerebrod_event_server_debug = src->cerebrod_event_server_debug;
      dest->cerebrod_event_server_debug_flag++;
    }
#endif /* CEREBRO_DEBUG */

  return 0;
}


/* 
 * _merge_cerebro_configs
 *
 * Merge contents of module_conf and config_file_conf into conf.  The
 * config file conf takes precedence.
 */
static int 
_merge_cerebro_configs(struct cerebro_config *conf,
                       struct cerebro_config *module_conf,
                       struct cerebro_config *config_file_conf,
                       unsigned int *errnum)
{
  if (!conf || !module_conf || !config_file_conf)
    {
      CEREBRO_DBG(("invalid parameters"));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  /*
   * Config file configuration takes precedence over config module
   * configuration
   */

  memset(conf, '\0', sizeof(struct cerebro_config));

  if (_set_cerebro_config(conf, config_file_conf, errnum) < 0)
    return -1;

  if (_set_cerebro_config(conf, module_conf, errnum) < 0)
    return -1;

  return 0;
}

int 
load_config(struct cerebro_config *conf, unsigned int *errnum)
{
  struct cerebro_config module_conf; 
  struct cerebro_config config_file_conf;

  if (!conf)
    {
      CEREBRO_DBG(("conf null"));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  memset(conf, '\0', sizeof(struct cerebro_config));
  memset(&module_conf, '\0', sizeof(struct cerebro_config));
  memset(&config_file_conf, '\0', sizeof(struct cerebro_config));

  if (_load_config_module(&module_conf, errnum) < 0)
    return -1;

  if (_load_config_file(&config_file_conf, errnum) < 0)
    return -1;

  if (_merge_cerebro_configs(conf, &module_conf, &config_file_conf, errnum) < 0)
    return -1;
 
  return 0;
}
