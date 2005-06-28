/*****************************************************************************\
 *  $Id: config_util.c,v 1.8 2005-06-28 21:26:52 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */

#include "cerebro/cerebro_constants.h"

#include "config_util.h"

#include "conffile.h"
#include "config_module.h"
#include "debug.h"

#if CEREBRO_DEBUG
char *config_debug_config_file = NULL;
int config_debug_output = 0;
#endif /* CEREBRO_DEBUG */

int
load_config_module(struct cerebro_config *conf)
{
  int rv = -1;
  config_module_t config_handle = NULL;

  if (!(config_handle = config_module_load()))
    goto cleanup;

  if (config_module_setup(config_handle) < 0)
    goto cleanup;

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

  if (config_module_load_default(config_handle, conf) < 0)
    goto cleanup;

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
 * conffile callback function that parses and stores an array of
 * strings
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

int
load_config_file(struct cerebro_config *conf)
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
	"cerebrod_heartbeat_network_interface", 
	CONFFILE_OPTION_STRING, 
	-1,
	conffile_string, 
	1, 
	0, 
	&(conf->cerebrod_heartbeat_network_interface_flag),
	conf->cerebrod_heartbeat_network_interface, 
	CEREBRO_MAX_NETWORK_INTERFACE_LEN
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
	"cerebrod_metric_max", 
	CONFFILE_OPTION_INT, 
	-1,
	conffile_int, 
	1, 
	0, 
	&(conf->cerebrod_metric_max_flag),
	&(conf->cerebrod_metric_max), 
	0
      },
      {
	"cerebrod_monitor_max", 
	CONFFILE_OPTION_INT, 
	-1,
	conffile_int, 
	1, 
	0, 
	&(conf->cerebrod_monitor_max_flag),
	&(conf->cerebrod_monitor_max), 
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
#endif /* CEREBRO_DEBUG */
    };
  conffile_t cf = NULL;
  int num;
  
  if (!(cf = conffile_handle_create()))
    {
      CEREBRO_DBG(("conffile_handle_create"));
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
_set_cerebro_config(struct cerebro_config *dest, struct cerebro_config *src)
{
  if (!dest || !src)
    {
      CEREBRO_DBG(("invalid parameters"));
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

  if (!dest->cerebrod_heartbeat_network_interface_flag
      && src->cerebrod_heartbeat_network_interface_flag)
    {
      strcpy(dest->cerebrod_heartbeat_network_interface, 
             src->cerebrod_heartbeat_network_interface);
      dest->cerebrod_heartbeat_network_interface_flag++;
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

  if (!dest->cerebrod_metric_max_flag && src->cerebrod_metric_max_flag)
    {
      dest->cerebrod_metric_max = src->cerebrod_metric_max;
      dest->cerebrod_metric_max_flag++;
    }

  if (!dest->cerebrod_monitor_max_flag && src->cerebrod_monitor_max_flag)
    {
      dest->cerebrod_monitor_max = src->cerebrod_monitor_max;
      dest->cerebrod_monitor_max_flag++;
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

  if (!dest->cerebrod_metric_server_debug_flag
      && src->cerebrod_metric_server_debug_flag)
    {
      dest->cerebrod_metric_server_debug = src->cerebrod_metric_server_debug;
      dest->cerebrod_metric_server_debug_flag++;
    }
#endif /* CEREBRO_DEBUG */

  return 0;
}

int 
merge_cerebro_configs(struct cerebro_config *conf,
		      struct cerebro_config *module_conf,
		      struct cerebro_config *config_file_conf)
{
  if (!conf || !module_conf || !config_file_conf)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  /*
   * Config file configuration takes precedence over config module
   * configuration
   */

  memset(conf, '\0', sizeof(struct cerebro_config));

  if (_set_cerebro_config(conf, config_file_conf) < 0)
    return -1;

  if (_set_cerebro_config(conf, module_conf) < 0)
    return -1;

  return 0;
}

int 
load_config(struct cerebro_config *conf)
{
  struct cerebro_config module_conf; 
  struct cerebro_config config_file_conf;

  if (!conf)
    {
      CEREBRO_DBG(("conf null"));
      return -1;
    }

  memset(conf, '\0', sizeof(struct cerebro_config));
  memset(&module_conf, '\0', sizeof(struct cerebro_config));
  memset(&config_file_conf, '\0', sizeof(struct cerebro_config));

  if (load_config_module(&module_conf) < 0)
    return -1;

  if (load_config_file(&config_file_conf) < 0)
    return -1;

  if (merge_cerebro_configs(conf, 
			    &module_conf,
			    &config_file_conf) < 0)
    return -1;
 
  return 0;
}
