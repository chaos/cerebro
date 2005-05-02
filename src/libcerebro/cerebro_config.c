/*****************************************************************************\
 *  $Id: cerebro_config.c,v 1.1 2005-05-02 17:50:34 achu Exp $
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
#include "conffile.h"

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
 * _cerebro_load_config_file
 *
 * Read and load configuration file
 *
 * Returns data in structure and 0 on success, -1 on error and error
 * msg in the buffer.
 */
static int
_cerebro_load_config_file(const char *config_file,
			  struct cerebro_config *conf,
			  char *buf,
			  unsigned int buflen)
{
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
    goto cleanup;
  
  if (!config_file)
    config_file = CEREBRO_CONFIG_FILE_DEFAULT;

  memset(conf, '\0', sizeof(struct cerebro_config));
  num = sizeof(options)/sizeof(struct conffile_option);
  if (conffile_parse(cf, config_file, options, num, NULL, 0, 0) < 0)
    {
      /* Its not an error if the default configuration file doesn't exist */
      if (!strcmp(config_file, CEREBRO_CONFIG_FILE_DEFAULT) 
	  && conffile_errnum(cf) == CONFFILE_ERR_EXIST)
	goto out;

      conffile_errmsg(cf, buf, buflen);
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
cerebro_load_config_file(const char *config_file,
			 struct cerebro_config *conf,
			 char *buf,
			 unsigned int buflen)
{
  if (!config_file)
    {
      cerebro_err_debug("%s(%s:%d): config_file null",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!conf)
    {
      cerebro_err_debug("%s(%s:%d): conf null",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
 
  if (!buf)
    {
      cerebro_err_debug("%s(%s:%d): buf null",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
 
  if (!(buflen > 0))
    {
      cerebro_err_debug("%s(%s:%d): buflen not valid",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  memset(conf, '\0', sizeof(struct cerebro_config));
  return _cerebro_load_config_file(config_file, conf, buf, buflen);
}
