/*****************************************************************************\
 *  $Id: cerebro_config.c,v 1.5 2005-05-03 22:46:34 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <errno.h>

#include "cerebro_config.h"
#include "cerebro_error.h"
#include "cerebro_module.h"
#include "conffile.h"

#ifndef NDEBUG
char *cerebro_config_debug_config_file = NULL;

int cerebro_config_debug_output = 0;
#endif /* NDEBUG */

/* 
 * _cerebro_config_load_config_module
 *
 * Find and load config module
 *
 * Returns data in structure and 0 on success, -1 on error
 */
static int
_cerebro_config_load_config_module(struct cerebro_config *conf)
{
  int load_config_module_called = 0;
  int module_setup_called = 0;
  int rv = -1;

  if (!cerebro_module_config_module_is_loaded())
    {
      if (!cerebro_module_is_setup())
	{
	  if (cerebro_module_setup() < 0)
	    {
	      cerebro_err_debug("%s(%s:%d): cerebro_module_setup",
				__FILE__, __FUNCTION__, __LINE__);
	      goto cleanup;
	    }
	  module_setup_called++;
	}
      
      if (cerebro_module_load_config_module() < 0)
	{
	  cerebro_err_debug("%s(%s:%d): cerebro_load_config_module",
			    __FILE__, __FUNCTION__, __LINE__);
	  goto cleanup;
	}

      if (cerebro_config_module_setup() < 0)
	{
	  cerebro_err_debug("%s(%s:%d): cerebro_config_setup",
			    __FILE__, __FUNCTION__, __LINE__);
	  goto cleanup;
	}

      load_config_module_called++;
    }

#ifndef NDEBUG
  if (cerebro_config_debug_output)
    {
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Cerebro Config Configuration:\n");
      fprintf(stderr, "* -----------------------\n");
      fprintf(stderr, "* Loading config from module: %s\n",
              cerebro_config_module_name());
      fprintf(stderr, "**************************************\n");
    }
#endif  /* NDEBUG */

  if (cerebro_config_module_load_default(conf) < 0)
    {
      cerebro_err_debug("%s(%s:%d): cerebro_config_load_default",
			__FILE__, __FUNCTION__, __LINE__);
      goto cleanup;
    }

  rv = 0;

 cleanup:
  if (load_config_module_called)
    {
      cerebro_config_module_cleanup();
      cerebro_module_unload_config_module();
    }
  if (module_setup_called)
    cerebro_module_cleanup();
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

  if (option_ptr == NULL)
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
 * _cb_cerebro_updown_hostnames
 *
 * conffile callback function that parses and stores an array of
 * strings
 *
 * Returns 0 on success, -1 on error
 */
static int
_cb_cerebro_updown_hostnames(conffile_t cf, struct conffile_data *data,
                 char *optionname, int option_type, void *option_ptr,
                 int option_data, void *app_ptr, int app_data)
{
  struct cerebro_config *conf;

  if (option_ptr == NULL)
    {
      conffile_seterrnum(cf, CONFFILE_ERR_PARAMETERS);
      return -1;
    }
  
  conf = (struct cerebro_config *)option_ptr;
  
  if (data->stringlist_len > 0)
    {
      int i;

      if (data->stringlist_len > CEREBRO_CONFIG_UPDOWN_HOSTNAMES_MAX)
	{
          conffile_seterrnum(cf, CONFFILE_ERR_PARSE_ARG_TOOMANY);
          return -1;
	}
      
      for (i = 0; i < data->stringlist_len; i++)
        {
	  if (strlen(data->stringlist[i]) > CEREBRO_MAXNODENAMELEN)
	    {
	      conffile_seterrnum(cf, CONFFILE_ERR_PARSE_OVERFLOW_ARGLEN);
	      return -1;
	    }

	  strcpy(conf->cerebro_updown_hostnames[i], data->stringlist[i]);
        }
      conf->cerebro_updown_hostnames_len = data->stringlist_len;
    }

  return 0;
}

/* 
 * _cerebro_config_load_config_file
 *
 * Read and load configuration file
 *
 * Returns data in structure and 0 on success, -1 on error
 */
static int
_cerebro_config_load_config_file(struct cerebro_config *conf)
{
  char *config_file = NULL;

  struct conffile_option options[] =
    {
      /*
       * Libcerebro configuration
       */
      {"cerebro_updown_hostnames", CONFFILE_OPTION_LIST_STRING, -1,
       _cb_cerebro_updown_hostnames, 1, 0, &(conf->cerebro_updown_hostnames_flag),
       conf, 0},
      {"cerebro_updown_port", CONFFILE_OPTION_INT, -1,
       conffile_int, 1, 0, &(conf->cerebro_updown_port_flag),
       &(conf->cerebro_updown_port), 0},
      {"cerebro_updown_timeout_len", CONFFILE_OPTION_INT, -1,
       conffile_int, 1, 0, &(conf->cerebro_updown_timeout_len_flag),
       &(conf->cerebro_updown_timeout_len), 0},
      {"cerebro_updown_flags", CONFFILE_OPTION_INT, -1,
       conffile_int, 1, 0, &(conf->cerebro_updown_flags_flag),
       &(conf->cerebro_updown_flags), 0},
      /*
       * Cerebrod configuration
       */
      {"cerebrod_heartbeat_frequency", CONFFILE_OPTION_LIST_INT, -1,
       _cb_cerebrod_heartbeat_freq, 1, 0, &(conf->cerebrod_heartbeat_frequency_flag),
       conf, 0},
      {"cerebrod_heartbeat_source_port", CONFFILE_OPTION_INT, -1,
       conffile_int, 1, 0, &(conf->cerebrod_heartbeat_source_port_flag),
       &(conf->cerebrod_heartbeat_source_port), 0},
      {"cerebrod_heartbeat_destination_port", CONFFILE_OPTION_INT, -1,
       conffile_int, 1, 0, &(conf->cerebrod_heartbeat_destination_port_flag),
       &(conf->cerebrod_heartbeat_destination_port), 0},
      {"cerebrod_heartbeat_destination_ip", CONFFILE_OPTION_STRING, -1,
       conffile_string, 1, 0, &(conf->cerebrod_heartbeat_destination_ip_flag),
       conf->cerebrod_heartbeat_destination_ip, CEREBRO_IPADDRSTRLEN},
      {"cerebrod_heartbeat_network_interface", CONFFILE_OPTION_STRING, -1,
       conffile_string, 1, 0, &(conf->cerebrod_heartbeat_network_interface_flag),
       conf->cerebrod_heartbeat_network_interface, CEREBRO_MAXNETWORKINTERFACE},
      {"cerebrod_heartbeat_ttl", CONFFILE_OPTION_INT, -1,
       conffile_int, 1, 0, &(conf->cerebrod_heartbeat_ttl_flag),
       &(conf->cerebrod_heartbeat_ttl), 0},
      {"cerebrod_speak", CONFFILE_OPTION_BOOL, -1,
       conffile_bool, 1, 0, &(conf->cerebrod_speak_flag),
       &conf->cerebrod_speak, 0},
      {"cerebrod_listen", CONFFILE_OPTION_BOOL, -1,
       conffile_bool, 1, 0, &(conf->cerebrod_listen_flag),
       &conf->cerebrod_listen, 0},
      {"cerebrod_listen_threads", CONFFILE_OPTION_INT, -1,
       conffile_int, 1, 0, &(conf->cerebrod_listen_threads_flag),
       &(conf->cerebrod_listen_threads), 0},
      {"cerebrod_updown_server", CONFFILE_OPTION_BOOL, -1,
       conffile_bool, 1, 0, &(conf->cerebrod_updown_server_flag),
       &conf->cerebrod_updown_server, 0},
      {"cerebrod_updown_server_port", CONFFILE_OPTION_INT, -1,
       conffile_int, 1, 0, &(conf->cerebrod_updown_server_port_flag),
       &(conf->cerebrod_updown_server_port), 0},
#ifndef NDEBUG
      {"cerebrod_speak_debug", CONFFILE_OPTION_BOOL, -1,
       conffile_bool, 1, 0, &(conf->cerebrod_speak_debug_flag),
       &conf->cerebrod_speak_debug, 0},
      {"cerebrod_listen_debug", CONFFILE_OPTION_BOOL, -1,
       conffile_bool, 1, 0, &(conf->cerebrod_listen_debug_flag),
       &conf->cerebrod_listen_debug, 0},
      {"cerebrod_updown_server_debug", CONFFILE_OPTION_BOOL, -1,
       conffile_bool, 1, 0, &(conf->cerebrod_updown_server_debug_flag),
       &conf->cerebrod_updown_server_debug, 0},
#endif /* NDEBUG */
    };
  conffile_t cf = NULL;
  int num;
  
  if (!(cf = conffile_handle_create()))
    {
      cerebro_err_debug("%s(%s:%d): conffile_handle_create: %s",
                        __FILE__, __FUNCTION__, __LINE__, strerror(errno));
      goto cleanup;
    }
  
#ifndef NDEBUG
  if (!cerebro_config_debug_config_file)
    config_file = CEREBRO_CONFIG_FILE_DEFAULT;
  else
    config_file = cerebro_config_debug_config_file;
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
	cerebro_err_debug("%s(%s:%d): conffile_parse: %d", 
			  __FILE__, __FUNCTION__, __LINE__,
			  conffile_errnum(cf));
      else
	cerebro_err_debug("%s(%s:%d): conffile_parse: %s", 
			  __FILE__, __FUNCTION__, __LINE__,
			  buf);
	
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

int 
cerebro_config_load(struct cerebro_config *conf)
{
  struct cerebro_config mod_conf; 
  struct cerebro_config file_conf;
  int i;

  if (!conf)
    {
      cerebro_err_debug("%s(%s:%d): conf null",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  memset(conf, '\0', sizeof(struct cerebro_config));
  memset(&mod_conf, '\0', sizeof(struct cerebro_config));
  memset(&file_conf, '\0', sizeof(struct cerebro_config));

  if (_cerebro_config_load_config_module(&mod_conf) < 0)
    return -1;

  if (_cerebro_config_load_config_file(&file_conf) < 0)
    return -1;

  /* 
   * Config file configuration takes precedence over config module
   * configuration
   */

  if (file_conf.cerebro_updown_hostnames_flag)
    {
      for (i = 0; i < file_conf.cerebro_updown_hostnames_len; i++)
	strcpy(conf->cerebro_updown_hostnames[i],
	       file_conf.cerebro_updown_hostnames[i]);
      conf->cerebro_updown_hostnames_len = file_conf.cerebro_updown_hostnames_len;
      conf->cerebro_updown_hostnames_flag++;
    }
  else if (mod_conf.cerebro_updown_hostnames_flag)
    {
      for (i = 0; i < mod_conf.cerebro_updown_hostnames_len; i++)
	strcpy(conf->cerebro_updown_hostnames[i],
	       mod_conf.cerebro_updown_hostnames[i]);
      conf->cerebro_updown_hostnames_len = mod_conf.cerebro_updown_hostnames_len;
      conf->cerebro_updown_hostnames_flag++;
    }

  if (file_conf.cerebro_updown_port_flag)
    {
      conf->cerebro_updown_port = file_conf.cerebro_updown_port;
      conf->cerebro_updown_port_flag++;
    }
  else if (mod_conf.cerebro_updown_port_flag)
    {
      conf->cerebro_updown_port = mod_conf.cerebro_updown_port;
      conf->cerebro_updown_port_flag++;
    }

  if (file_conf.cerebro_updown_timeout_len_flag)
    {
      conf->cerebro_updown_timeout_len = file_conf.cerebro_updown_timeout_len;
      conf->cerebro_updown_timeout_len_flag++;
    }
  else if (mod_conf.cerebro_updown_timeout_len_flag)
    {
      conf->cerebro_updown_timeout_len = mod_conf.cerebro_updown_timeout_len;
      conf->cerebro_updown_timeout_len_flag++;
    }

  if (file_conf.cerebro_updown_flags_flag)
    {
      conf->cerebro_updown_flags = file_conf.cerebro_updown_flags;
      conf->cerebro_updown_flags_flag++;
    }
  else if (mod_conf.cerebro_updown_flags_flag)
    {
      conf->cerebro_updown_flags = mod_conf.cerebro_updown_flags;
      conf->cerebro_updown_flags_flag++;
    }

  if (file_conf.cerebrod_heartbeat_frequency_flag)
    {
      conf->cerebrod_heartbeat_frequency_min = file_conf.cerebrod_heartbeat_frequency_min;
      conf->cerebrod_heartbeat_frequency_max = file_conf.cerebrod_heartbeat_frequency_max;
      conf->cerebrod_heartbeat_frequency_flag++;
    }
  else if (mod_conf.cerebrod_heartbeat_frequency_flag)
    {
      conf->cerebrod_heartbeat_frequency_min = mod_conf.cerebrod_heartbeat_frequency_min;
      conf->cerebrod_heartbeat_frequency_max = mod_conf.cerebrod_heartbeat_frequency_max;
      conf->cerebrod_heartbeat_frequency_flag++;
    }

  if (file_conf.cerebrod_heartbeat_source_port_flag)
    {
      conf->cerebrod_heartbeat_source_port = file_conf.cerebrod_heartbeat_source_port;
      conf->cerebrod_heartbeat_source_port_flag++;
    }
  else if (mod_conf.cerebrod_heartbeat_source_port_flag)
    {
      conf->cerebrod_heartbeat_source_port = mod_conf.cerebrod_heartbeat_source_port;
      conf->cerebrod_heartbeat_source_port_flag++;
    }

  if (file_conf.cerebrod_heartbeat_destination_port_flag)
    {
      conf->cerebrod_heartbeat_destination_port = file_conf.cerebrod_heartbeat_destination_port;
      conf->cerebrod_heartbeat_destination_port_flag++;
    }
  else if (mod_conf.cerebrod_heartbeat_destination_port_flag)
    {
      conf->cerebrod_heartbeat_destination_port = mod_conf.cerebrod_heartbeat_destination_port;
      conf->cerebrod_heartbeat_destination_port_flag++;
    }

  if (file_conf.cerebrod_heartbeat_destination_ip_flag)
    {
      strcpy(conf->cerebrod_heartbeat_destination_ip, file_conf.cerebrod_heartbeat_destination_ip);
      conf->cerebrod_heartbeat_destination_ip_flag++;
    }
  else if (mod_conf.cerebrod_heartbeat_destination_ip_flag)
    {
      strcpy(conf->cerebrod_heartbeat_destination_ip, file_conf.cerebrod_heartbeat_destination_ip);
      conf->cerebrod_heartbeat_destination_ip_flag++;
    }

  if (file_conf.cerebrod_heartbeat_network_interface_flag)
    {
      strcpy(conf->cerebrod_heartbeat_network_interface, file_conf.cerebrod_heartbeat_network_interface);
      conf->cerebrod_heartbeat_network_interface_flag++;
    }
  else if (mod_conf.cerebrod_heartbeat_network_interface_flag)
    {
      strcpy(conf->cerebrod_heartbeat_network_interface, file_conf.cerebrod_heartbeat_network_interface);
      conf->cerebrod_heartbeat_network_interface_flag++;
    }

  if (file_conf.cerebrod_heartbeat_ttl_flag)
    {
      conf->cerebrod_heartbeat_ttl = file_conf.cerebrod_heartbeat_ttl;
      conf->cerebrod_heartbeat_ttl_flag++;
    }
  else if (mod_conf.cerebrod_heartbeat_ttl_flag)
    {
      conf->cerebrod_heartbeat_ttl = mod_conf.cerebrod_heartbeat_ttl;
      conf->cerebrod_heartbeat_ttl_flag++;
    }

  if (file_conf.cerebrod_speak_flag)
    {
      conf->cerebrod_speak = file_conf.cerebrod_speak;
      conf->cerebrod_speak_flag++;
    }
  else if (mod_conf.cerebrod_speak_flag)
    {
      conf->cerebrod_speak = mod_conf.cerebrod_speak;
      conf->cerebrod_speak_flag++;
    }

  if (file_conf.cerebrod_listen_flag)
    {
      conf->cerebrod_listen = file_conf.cerebrod_listen;
      conf->cerebrod_listen_flag++;
    }
  else if (mod_conf.cerebrod_listen_flag)
    {
      conf->cerebrod_listen = mod_conf.cerebrod_listen;
      conf->cerebrod_listen_flag++;
    }

  if (file_conf.cerebrod_listen_threads_flag)
    {
      conf->cerebrod_listen_threads = file_conf.cerebrod_listen_threads;
      conf->cerebrod_listen_threads_flag++;
    }
  else if (mod_conf.cerebrod_listen_threads_flag)
    {
      conf->cerebrod_listen_threads = mod_conf.cerebrod_listen_threads;
      conf->cerebrod_listen_threads_flag++;
    }

  if (file_conf.cerebrod_updown_server_flag)
    {
      conf->cerebrod_updown_server = file_conf.cerebrod_updown_server;
      conf->cerebrod_updown_server_flag++;
    }
  else if (mod_conf.cerebrod_updown_server_flag)
    {
      conf->cerebrod_updown_server = mod_conf.cerebrod_updown_server;
      conf->cerebrod_updown_server_flag++;
    }

  if (file_conf.cerebrod_updown_server_port_flag)
    {
      conf->cerebrod_updown_server_port = file_conf.cerebrod_updown_server_port;
      conf->cerebrod_updown_server_port_flag++;
    }
  else if (mod_conf.cerebrod_updown_server_port_flag)
    {
      conf->cerebrod_updown_server_port = mod_conf.cerebrod_updown_server_port;
      conf->cerebrod_updown_server_port_flag++;
    }

#ifndef NDEBUG
  if (file_conf.cerebrod_speak_debug_flag)
    {
      conf->cerebrod_speak_debug = file_conf.cerebrod_speak_debug;
      conf->cerebrod_speak_debug_flag++;
    }
  else if (mod_conf.cerebrod_speak_debug_flag)
    {
      conf->cerebrod_speak_debug = mod_conf.cerebrod_speak_debug;
      conf->cerebrod_speak_debug_flag++;
    }

  if (file_conf.cerebrod_listen_debug_flag)
    {
      conf->cerebrod_listen_debug = file_conf.cerebrod_listen_debug;
      conf->cerebrod_listen_debug_flag++;
    }
  else if (mod_conf.cerebrod_listen_debug_flag)
    {
      conf->cerebrod_listen_debug = mod_conf.cerebrod_listen_debug;
      conf->cerebrod_listen_debug_flag++;
    }

  if (file_conf.cerebrod_updown_server_debug_flag)
    {
      conf->cerebrod_updown_server_debug = file_conf.cerebrod_updown_server_debug;
      conf->cerebrod_updown_server_debug_flag++;
    }
  else if (mod_conf.cerebrod_updown_server_debug_flag)
    {
      conf->cerebrod_updown_server_debug = mod_conf.cerebrod_updown_server_debug;
      conf->cerebrod_updown_server_debug_flag++;
    }
#endif /* NDEBUG */
 
  return 0;
}
