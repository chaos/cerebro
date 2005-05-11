/*****************************************************************************\
 *  $Id: cerebro_config_util.c,v 1.5 2005-05-11 17:06:28 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */

#include "cerebro_api.h"
#include "cerebro_config_util.h"
#include "cerebro_module.h"
#include "cerebro_util.h"
#include "cerebro/cerebro_config.h"
#include "cerebro/cerebro_error.h"
#include "conffile.h"

#if CEREBRO_DEBUG
char *cerebro_config_debug_config_file = NULL;
int cerebro_config_debug_output = 0;
#endif /* CEREBRO_DEBUG */

int
_cerebro_config_load_config_module(struct cerebro_config *conf)
{
  int load_config_module_called = 0;
  int module_setup_called = 0;
  int rv = -1;

  if (!_cerebro_module_config_module_is_loaded())
    {
      if (!_cerebro_module_is_setup())
	{
	  if (_cerebro_module_setup() < 0)
	    goto cleanup;
	  module_setup_called++;
	}
      
      if (_cerebro_module_load_config_module() < 0)
	goto cleanup;

      if (_cerebro_config_module_setup() < 0)
	goto cleanup;

      load_config_module_called++;
    }

#if CEREBRO_DEBUG  
  if (cerebro_config_debug_output)
    {
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Cerebro Config Configuration:\n");
      fprintf(stderr, "* -----------------------\n");
      fprintf(stderr, "* Loading config from module: %s\n",
              _cerebro_config_module_name());
      fprintf(stderr, "**************************************\n");
    }
#endif /* CEREBRO_DEBUG */

  if (_cerebro_config_module_load_default(conf) < 0)
    goto cleanup;

  rv = 0;
 cleanup:
  if (load_config_module_called)
    {
      _cerebro_config_module_cleanup();
      _cerebro_module_unload_config_module();
    }
  if (module_setup_called)
    _cerebro_module_cleanup();
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

  if (!option_ptr)
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

int
_cerebro_config_load_config_file(struct cerebro_config *conf)
{
  char *config_file = NULL;

  struct conffile_option options[] =
    {
      /*
       * Libcerebro configuration
       */
      {
	"cerebro_updown_hostnames", 
	CONFFILE_OPTION_LIST_STRING, 
	-1,
	_cb_cerebro_updown_hostnames, 
	1, 
	0, 
	&(conf->cerebro_updown_hostnames_flag), 
	conf, 
	0
      },
      {
	"cerebro_updown_port", 
	CONFFILE_OPTION_INT, 
	-1,
	conffile_int, 
	1, 
	0, 
	&(conf->cerebro_updown_port_flag), 
	&(conf->cerebro_updown_port), 
	0
      },
      {
	"cerebro_updown_timeout_len", 
	CONFFILE_OPTION_INT, 
	-1,
	conffile_int, 
	1, 
	0, 
	&(conf->cerebro_updown_timeout_len_flag),
	&(conf->cerebro_updown_timeout_len), 
	0
      },
      {
	"cerebro_updown_flags", 
	CONFFILE_OPTION_INT, 
	-1,
	conffile_int, 
	1, 
	0, 
	&(conf->cerebro_updown_flags_flag),
	&(conf->cerebro_updown_flags), 
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
	CEREBRO_MAXIPADDRLEN
      },
      {
	"cerebrod_heartbeat_network_interface", 
	CONFFILE_OPTION_STRING, 
	-1,
	conffile_string, 
	1, 
	0, 
	&(conf->cerebrod_heartbeat_network_interface_flag),
	conf->cerebrod_heartbeat_network_interface, 
	CEREBRO_MAXNETWORKINTERFACELEN
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
	"cerebrod_updown_server", 
	CONFFILE_OPTION_BOOL, 
	-1,
	conffile_bool, 
	1, 
	0, 
	&(conf->cerebrod_updown_server_flag),
	&conf->cerebrod_updown_server, 
	0
      },
      {
	"cerebrod_updown_server_port", 
	CONFFILE_OPTION_INT, 
	-1,
	conffile_int, 
	1, 
	0, 
	&(conf->cerebrod_updown_server_port_flag),
	&(conf->cerebrod_updown_server_port), 
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
	"cerebrod_updown_server_debug", 
	CONFFILE_OPTION_BOOL, 
	-1,
	conffile_bool, 
	1, 
	0, 
	&(conf->cerebrod_updown_server_debug_flag),
	&conf->cerebrod_updown_server_debug, 
	0
      },
#endif /* CEREBRO_DEBUG */
    };
  conffile_t cf = NULL;
  int num;
  
  if (!(cf = conffile_handle_create()))
    {
      cerebro_err_debug_lib("%s(%s:%d): conffile_handle_create",
			    __FILE__, __FUNCTION__, __LINE__);
      goto cleanup;
    }
  
#if CEREBRO_DEBUG
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
	cerebro_err_debug_lib("%s(%s:%d): conffile_parse: %d", 
			      __FILE__, __FUNCTION__, __LINE__,
			      conffile_errnum(cf));
      else
	cerebro_err_debug_lib("%s(%s:%d): conffile_parse: %s", 
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
_cerebro_config_merge_cerebro_config(struct cerebro_config *conf,
				     struct cerebro_config *module_conf,
				     struct cerebro_config *config_file_conf)
{
  int i;

  if (!conf)
    {
      cerebro_err_debug_lib("%s(%s:%d): conf null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!module_conf)
    {
      cerebro_err_debug_lib("%s(%s:%d): module_conf null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!config_file_conf)
    {
      cerebro_err_debug_lib("%s(%s:%d): config_file_conf null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  /*
   * Config file configuration takes precedence over config module
   * configuration
   */

  memset(conf, '\0', sizeof(struct cerebro_config));

  if (config_file_conf->cerebro_updown_hostnames_flag)
    {
      for (i = 0; i < config_file_conf->cerebro_updown_hostnames_len; i++)
	strcpy(conf->cerebro_updown_hostnames[i],
	       config_file_conf->cerebro_updown_hostnames[i]);
      conf->cerebro_updown_hostnames_len = config_file_conf->cerebro_updown_hostnames_len;
      conf->cerebro_updown_hostnames_flag++;
    }
  else if (module_conf->cerebro_updown_hostnames_flag)
    {
      for (i = 0; i < module_conf->cerebro_updown_hostnames_len; i++)
	strcpy(conf->cerebro_updown_hostnames[i],
	       module_conf->cerebro_updown_hostnames[i]);
      conf->cerebro_updown_hostnames_len = module_conf->cerebro_updown_hostnames_len;
      conf->cerebro_updown_hostnames_flag++;
    }

  if (config_file_conf->cerebro_updown_port_flag)
    {
      conf->cerebro_updown_port = config_file_conf->cerebro_updown_port;
      conf->cerebro_updown_port_flag++;
    }
  else if (module_conf->cerebro_updown_port_flag)
    {
      conf->cerebro_updown_port = module_conf->cerebro_updown_port;
      conf->cerebro_updown_port_flag++;
    }

  if (config_file_conf->cerebro_updown_timeout_len_flag)
    {
      conf->cerebro_updown_timeout_len = config_file_conf->cerebro_updown_timeout_len;
      conf->cerebro_updown_timeout_len_flag++;
    }
  else if (module_conf->cerebro_updown_timeout_len_flag)
    {
      conf->cerebro_updown_timeout_len = module_conf->cerebro_updown_timeout_len;
      conf->cerebro_updown_timeout_len_flag++;
    }

  if (config_file_conf->cerebro_updown_flags_flag)
    {
      conf->cerebro_updown_flags = config_file_conf->cerebro_updown_flags;
      conf->cerebro_updown_flags_flag++;
    }
  else if (module_conf->cerebro_updown_flags_flag)
    {
      conf->cerebro_updown_flags = module_conf->cerebro_updown_flags;
      conf->cerebro_updown_flags_flag++;
    }

  if (config_file_conf->cerebrod_heartbeat_frequency_flag)
    {
      conf->cerebrod_heartbeat_frequency_min = config_file_conf->cerebrod_heartbeat_frequency_min;
      conf->cerebrod_heartbeat_frequency_max = config_file_conf->cerebrod_heartbeat_frequency_max;
      conf->cerebrod_heartbeat_frequency_flag++;
    }
  else if (module_conf->cerebrod_heartbeat_frequency_flag)
    {
      conf->cerebrod_heartbeat_frequency_min = module_conf->cerebrod_heartbeat_frequency_min;
      conf->cerebrod_heartbeat_frequency_max = module_conf->cerebrod_heartbeat_frequency_max;
      conf->cerebrod_heartbeat_frequency_flag++;
    }

  if (config_file_conf->cerebrod_heartbeat_source_port_flag)
    {
      conf->cerebrod_heartbeat_source_port = config_file_conf->cerebrod_heartbeat_source_port;
      conf->cerebrod_heartbeat_source_port_flag++;
    }
  else if (module_conf->cerebrod_heartbeat_source_port_flag)
    {
      conf->cerebrod_heartbeat_source_port = module_conf->cerebrod_heartbeat_source_port;
      conf->cerebrod_heartbeat_source_port_flag++;
    }

  if (config_file_conf->cerebrod_heartbeat_destination_port_flag)
    {
      conf->cerebrod_heartbeat_destination_port = config_file_conf->cerebrod_heartbeat_destination_port;
      conf->cerebrod_heartbeat_destination_port_flag++;
    }
  else if (module_conf->cerebrod_heartbeat_destination_port_flag)
    {
      conf->cerebrod_heartbeat_destination_port = module_conf->cerebrod_heartbeat_destination_port;
      conf->cerebrod_heartbeat_destination_port_flag++;
    }

  if (config_file_conf->cerebrod_heartbeat_destination_ip_flag)
    {
      strcpy(conf->cerebrod_heartbeat_destination_ip, config_file_conf->cerebrod_heartbeat_destination_ip);
      conf->cerebrod_heartbeat_destination_ip_flag++;
    }
  else if (module_conf->cerebrod_heartbeat_destination_ip_flag)
    {
      strcpy(conf->cerebrod_heartbeat_destination_ip, module_conf->cerebrod_heartbeat_destination_ip);
      conf->cerebrod_heartbeat_destination_ip_flag++;
    }

  if (config_file_conf->cerebrod_heartbeat_network_interface_flag)
    {
      strcpy(conf->cerebrod_heartbeat_network_interface, config_file_conf->cerebrod_heartbeat_network_interface);
      conf->cerebrod_heartbeat_network_interface_flag++;
    }
  else if (module_conf->cerebrod_heartbeat_network_interface_flag)
    {
      strcpy(conf->cerebrod_heartbeat_network_interface, module_conf->cerebrod_heartbeat_network_interface);
      conf->cerebrod_heartbeat_network_interface_flag++;
    }

  if (config_file_conf->cerebrod_heartbeat_ttl_flag)
    {
      conf->cerebrod_heartbeat_ttl = config_file_conf->cerebrod_heartbeat_ttl;
      conf->cerebrod_heartbeat_ttl_flag++;
    }
  else if (module_conf->cerebrod_heartbeat_ttl_flag)
    {
      conf->cerebrod_heartbeat_ttl = module_conf->cerebrod_heartbeat_ttl;
      conf->cerebrod_heartbeat_ttl_flag++;
    }

  if (config_file_conf->cerebrod_speak_flag)
    {
      conf->cerebrod_speak = config_file_conf->cerebrod_speak;
      conf->cerebrod_speak_flag++;
    }
  else if (module_conf->cerebrod_speak_flag)
    {
      conf->cerebrod_speak = module_conf->cerebrod_speak;
      conf->cerebrod_speak_flag++;
    }

  if (config_file_conf->cerebrod_listen_flag)
    {
      conf->cerebrod_listen = config_file_conf->cerebrod_listen;
      conf->cerebrod_listen_flag++;
    }
  else if (module_conf->cerebrod_listen_flag)
    {
      conf->cerebrod_listen = module_conf->cerebrod_listen;
      conf->cerebrod_listen_flag++;
    }

  if (config_file_conf->cerebrod_listen_threads_flag)
    {
      conf->cerebrod_listen_threads = config_file_conf->cerebrod_listen_threads;
      conf->cerebrod_listen_threads_flag++;
    }
  else if (module_conf->cerebrod_listen_threads_flag)
    {
      conf->cerebrod_listen_threads = module_conf->cerebrod_listen_threads;
      conf->cerebrod_listen_threads_flag++;
    }

  if (config_file_conf->cerebrod_updown_server_flag)
    {
      conf->cerebrod_updown_server = config_file_conf->cerebrod_updown_server;
      conf->cerebrod_updown_server_flag++;
    }
  else if (module_conf->cerebrod_updown_server_flag)
    {
      conf->cerebrod_updown_server = module_conf->cerebrod_updown_server;
      conf->cerebrod_updown_server_flag++;
    }

  if (config_file_conf->cerebrod_updown_server_port_flag)
    {
      conf->cerebrod_updown_server_port = config_file_conf->cerebrod_updown_server_port;
      conf->cerebrod_updown_server_port_flag++;
    }
  else if (module_conf->cerebrod_updown_server_port_flag)
    {
      conf->cerebrod_updown_server_port = module_conf->cerebrod_updown_server_port;
      conf->cerebrod_updown_server_port_flag++;
    }

#if CEREBRO_DEBUG
  if (config_file_conf->cerebrod_speak_debug_flag)
    {
      conf->cerebrod_speak_debug = config_file_conf->cerebrod_speak_debug;
      conf->cerebrod_speak_debug_flag++;
    }
  else if (module_conf->cerebrod_speak_debug_flag)
    {
      conf->cerebrod_speak_debug = module_conf->cerebrod_speak_debug;
      conf->cerebrod_speak_debug_flag++;
    }

  if (config_file_conf->cerebrod_listen_debug_flag)
    {
      conf->cerebrod_listen_debug = config_file_conf->cerebrod_listen_debug;
      conf->cerebrod_listen_debug_flag++;
    }
  else if (module_conf->cerebrod_listen_debug_flag)
    {
      conf->cerebrod_listen_debug = module_conf->cerebrod_listen_debug;
      conf->cerebrod_listen_debug_flag++;
    }

  if (config_file_conf->cerebrod_updown_server_debug_flag)
    {
      conf->cerebrod_updown_server_debug = config_file_conf->cerebrod_updown_server_debug;
      conf->cerebrod_updown_server_debug_flag++;
    }
  else if (module_conf->cerebrod_updown_server_debug_flag)
    {
      conf->cerebrod_updown_server_debug = module_conf->cerebrod_updown_server_debug;
      conf->cerebrod_updown_server_debug_flag++;
    }
#endif /* CEREBRO_DEBUG */

  return 0;
}

int 
_cerebro_config_load(struct cerebro_config *conf)
{
  struct cerebro_config module_conf; 
  struct cerebro_config config_file_conf;

  if (!conf)
    {
      cerebro_err_debug_lib("%s(%s:%d): conf null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  memset(conf, '\0', sizeof(struct cerebro_config));
  memset(&module_conf, '\0', sizeof(struct cerebro_config));
  memset(&config_file_conf, '\0', sizeof(struct cerebro_config));

  if (_cerebro_config_load_config_module(&module_conf) < 0)
    return -1;

  if (_cerebro_config_load_config_file(&config_file_conf) < 0)
    return -1;

  if (_cerebro_config_merge_cerebro_config(conf, 
					   &module_conf,
					   &config_file_conf) < 0)
    return -1;
 
  return 0;
}

int 
_cerebro_load_config(cerebro_t handle)
{
  struct cerebro_config module_conf;
  struct cerebro_config config_file_conf;
  
  if (_cerebro_handle_check(handle) < 0)
    return -1;

  if (handle->loaded_state & CEREBRO_CONFIG_LOADED)
    {
      handle->errnum = CEREBRO_ERR_SUCCESS;
      return 0;
    }
  
  memset(&module_conf, '\0', sizeof(struct cerebro_config));
  if (_cerebro_config_load_config_module(&module_conf) < 0)
    {
      handle->errnum = CEREBRO_ERR_CONFIG_MODULE;
      return -1;
    }

  memset(&config_file_conf, '\0', sizeof(struct cerebro_config));
  if (_cerebro_config_load_config_file(&config_file_conf) < 0)
    {
      handle->errnum = CEREBRO_ERR_CONFIG_FILE;
      return -1;
    }

  memset(&(handle->config_data), '\0', sizeof(struct cerebro_config));
  if (_cerebro_config_merge_cerebro_config(&(handle->config_data), 
					   &module_conf, 
					   &config_file_conf) < 0)
    {
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  
  handle->loaded_state |= CEREBRO_CONFIG_LOADED;
  handle->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}

int 
_cerebro_unload_config(cerebro_t handle)
{
  if (_cerebro_handle_check(handle) < 0)
    return -1;

  memset(&(handle->config_data), '\0', sizeof(struct cerebro_config));
  
  handle->loaded_state &= ~CEREBRO_CONFIG_LOADED;
  handle->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}
