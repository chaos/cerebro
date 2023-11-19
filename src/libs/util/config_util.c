/*****************************************************************************\
 *  $Id: config_util.c,v 1.35 2010-02-02 01:01:21 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2018 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2005-2007 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <https://github.com/chaos/cerebro>.
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
 *  with Cerebro. If not, see <http://www.gnu.org/licenses/>.
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

char *config_debug_config_file = NULL;
int config_debug_output = 0;

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
  int rv = -1, found = 0;
  config_module_t config_handle = NULL;

  if (!(config_handle = config_module_load()))
    {
      if (errnum)
        *errnum = CEREBRO_ERR_CONFIG_MODULE;
      goto cleanup;
    }

  if ((found = config_module_found(config_handle)) < 0)
    {
      if (errnum)
        *errnum = CEREBRO_ERR_CONFIG_MODULE;
      goto cleanup;
    }

  if (!found)
    goto out;

  if (config_module_setup(config_handle) < 0)
    {
      if (errnum)
        *errnum = CEREBRO_ERR_CONFIG_MODULE;
      goto cleanup;
    }

  if (config_debug_output)
    {
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Cerebro Config Configuration:\n");
      fprintf(stderr, "* -----------------------\n");
      fprintf(stderr, "* Loading config from module: %s\n",
              config_module_name(config_handle));
      fprintf(stderr, "**************************************\n");
    }

  if (config_module_load_config(config_handle, conf) < 0)
    {
      if (errnum)
        *errnum = CEREBRO_ERR_CONFIG_MODULE;
      goto cleanup;
    }

 out:
  rv = 0;
 cleanup:
  if (found > 0)
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
      conf->cerebrod_heartbeat_frequency_max = data->intlist[0];
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
 * _cb_cerebro_metric_server
 *
 * callback function that parses and stores cerebrod metric servers to
 * connect to.
 *
 * Returns 0 on success, -1 on error
 */
static int
_cb_cerebro_metric_server(conffile_t cf, struct conffile_data *data,
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

  /* arg1 - required - ip/hostname - 0.0.0.0 for default
   * arg2 - optional - port - 0 for default
   */
  if (data->stringlist_len > 0)
    {
      int index = conf->cerebro_metric_server_len;

      if (data->stringlist_len > 2)
        {
          conffile_seterrnum(cf, CONFFILE_ERR_PARSE_ARG_TOOMANY);
          return -1;
        }

      if (strlen(data->stringlist[0]) > CEREBRO_MAX_HOSTNAME_LEN)
        {
          conffile_seterrnum(cf, CONFFILE_ERR_PARSE_OVERFLOW_ARGLEN);
          return -1;
        }
      strcpy(conf->cerebro_metric_server[index].hostname, data->stringlist[0]);

      if (data->stringlist_len > 1)
        {
          int port;
          char *endptr;

          port = strtol(data->stringlist[1], &endptr, 0);
          if (endptr != (data->stringlist[1] + strlen(data->stringlist[1])))
            {
              conffile_seterrnum(cf, CONFFILE_ERR_PARSE_ARG_INVALID);
              return -1;
            }

          conf->cerebro_metric_server[index].port = port;
        }
      else
        conf->cerebro_metric_server[index].port = CEREBRO_CONFIG_PORT_DEFAULT;

      conf->cerebro_metric_server_len++;
    }

  return 0;
}

/*
 * _cb_cerebro_event_server
 *
 * callback function that parses and stores cerebrod event servers to
 * connect to.
 *
 * Returns 0 on success, -1 on error
 */
static int
_cb_cerebro_event_server(conffile_t cf, struct conffile_data *data,
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

  /* arg1 - required - ip/hostname - 0.0.0.0 for default
   * arg2 - optional - port - 0 for default
   */
  if (data->stringlist_len > 0)
    {
      int index = conf->cerebro_event_server_len;

      if (data->stringlist_len > 2)
        {
          conffile_seterrnum(cf, CONFFILE_ERR_PARSE_ARG_TOOMANY);
          return -1;
        }

      if (strlen(data->stringlist[0]) > CEREBRO_MAX_HOSTNAME_LEN)
        {
          conffile_seterrnum(cf, CONFFILE_ERR_PARSE_OVERFLOW_ARGLEN);
          return -1;
        }
      strcpy(conf->cerebro_event_server[index].hostname, data->stringlist[0]);

      if (data->stringlist_len > 1)
        {
          int port;
          char *endptr;

          port = strtol(data->stringlist[1], &endptr, 0);
          if (endptr != (data->stringlist[1] + strlen(data->stringlist[1])))
            {
              conffile_seterrnum(cf, CONFFILE_ERR_PARSE_ARG_INVALID);
              return -1;
            }

          conf->cerebro_event_server[index].port = port;
        }
      else
        conf->cerebro_event_server[index].port = CEREBRO_CONFIG_PORT_DEFAULT;

      conf->cerebro_event_server_len++;
    }

  return 0;
}

/*
 * _cb_cerebrod_speak_message_config
 *
 * callback function that parses and stores cerebrod speak message
 * configuration.
 *
 * Returns 0 on success, -1 on error
 */
static int
_cb_cerebrod_speak_message_config(conffile_t cf, struct conffile_data *data,
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

  /* arg1 - required - destination ip - 0.0.0.0 for default
   * arg2 - optional - destination port - 0 for default
   * arg3 - optional - source port - 0 for default
   * arg4 - optional - source_network_interface - 0.0.0.0 for default
   */
  if (data->stringlist_len > 0)
    {
      int index = conf->cerebrod_speak_message_config_len;

      if (data->stringlist_len > 4)
        {
          conffile_seterrnum(cf, CONFFILE_ERR_PARSE_ARG_TOOMANY);
          return -1;
        }

      if (strlen(data->stringlist[0]) > CEREBRO_MAX_HOSTNAME_LEN)
        {
          conffile_seterrnum(cf, CONFFILE_ERR_PARSE_OVERFLOW_ARGLEN);
          return -1;
        }

      strcpy(conf->cerebrod_speak_message_config[index].ip, data->stringlist[0]);

      if (data->stringlist_len > 1)
        {
          int port;
          char *endptr;

          port = strtol(data->stringlist[1], &endptr, 0);
          if (endptr != (data->stringlist[1] + strlen(data->stringlist[1])))
            {
              conffile_seterrnum(cf, CONFFILE_ERR_PARSE_ARG_INVALID);
              return -1;
            }

          conf->cerebrod_speak_message_config[index].destination_port = port;
        }
      else
        conf->cerebrod_speak_message_config[index].destination_port = CEREBRO_CONFIG_PORT_DEFAULT;

      if (data->stringlist_len > 2)
        {
          int port;
          char *endptr;

          port = strtol(data->stringlist[2], &endptr, 0);
          if (endptr != (data->stringlist[2] + strlen(data->stringlist[2])))
            {
              conffile_seterrnum(cf, CONFFILE_ERR_PARSE_ARG_INVALID);
              return -1;
            }

          conf->cerebrod_speak_message_config[index].source_port = port;
        }
      else
        conf->cerebrod_speak_message_config[index].source_port = CEREBRO_CONFIG_PORT_DEFAULT;

      if (data->stringlist_len > 3)
        {
          if (strlen(data->stringlist[3]) > CEREBRO_MAX_HOSTNAME_LEN)
            {
              conffile_seterrnum(cf, CONFFILE_ERR_PARSE_OVERFLOW_ARGLEN);
              return -1;
            }
          strcpy(conf->cerebrod_speak_message_config[index].network_interface, data->stringlist[3]);
        }
      else
        strcpy(conf->cerebrod_speak_message_config[index].network_interface, CEREBRO_CONFIG_IP_DEFAULT);
      conf->cerebrod_speak_message_config_len++;
    }

  return 0;
}

/*
 * _cb_cerebrod_listen_message_config
 *
 * callback function that parses and stores cerebrod listen message
 * configuration.
 *
 * Returns 0 on success, -1 on error
 */
static int
_cb_cerebrod_listen_message_config(conffile_t cf, struct conffile_data *data,
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

  /* arg1 - required - ip - 0.0.0.0 for default
   * arg2 - optional - port - 0 for default
   * arg3 - optional - network_interface - 0.0.0.0 for default
   */
  if (data->stringlist_len > 0)
    {
      int index = conf->cerebrod_listen_message_config_len;

      if (data->stringlist_len > 3)
        {
          conffile_seterrnum(cf, CONFFILE_ERR_PARSE_ARG_TOOMANY);
          return -1;
        }

      if (strlen(data->stringlist[0]) > CEREBRO_MAX_HOSTNAME_LEN)
        {
          conffile_seterrnum(cf, CONFFILE_ERR_PARSE_OVERFLOW_ARGLEN);
          return -1;
        }

      strcpy(conf->cerebrod_listen_message_config[index].ip, data->stringlist[0]);

      if (data->stringlist_len > 1)
        {
          int port;
          char *endptr;

          port = strtol(data->stringlist[1], &endptr, 0);
          if (endptr != (data->stringlist[1] + strlen(data->stringlist[1])))
            {
              conffile_seterrnum(cf, CONFFILE_ERR_PARSE_ARG_INVALID);
              return -1;
            }

          conf->cerebrod_listen_message_config[index].port = port;
        }
      else
        conf->cerebrod_listen_message_config[index].port = CEREBRO_CONFIG_PORT_DEFAULT;

      if (data->stringlist_len > 2)
        {
          if (strlen(data->stringlist[2]) > CEREBRO_MAX_HOSTNAME_LEN)
            {
              conffile_seterrnum(cf, CONFFILE_ERR_PARSE_OVERFLOW_ARGLEN);
              return -1;
            }
          strcpy(conf->cerebrod_listen_message_config[index].network_interface, data->stringlist[2]);
        }
      else
        strcpy(conf->cerebrod_listen_message_config[index].network_interface, CEREBRO_CONFIG_IP_DEFAULT);

      conf->cerebrod_listen_message_config_len++;
    }

  return 0;
}

/*
 * _cb_cerebrod_forward_message_config
 *
 * callback function that parses and stores cerebrod forward message
 * configuration.
 *
 * Returns 0 on success, -1 on error
 */
static int
_cb_cerebrod_forward_message_config(conffile_t cf, struct conffile_data *data,
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

  /* arg1 - required - destination ip - 0.0.0.0 for default
   * arg2 - optional - destination port - 0 for default
   * arg3 - optional - source port - 0 for default
   * arg4 - optional - source_network_interface - 0.0.0.0 for default
   * arg5 - optional - list host specific for forwarding
   * arg6 ...
   */
  if (data->stringlist_len > 0)
    {
      int index = conf->cerebrod_forward_message_config_len;

      if (data->stringlist_len > (4 + CEREBRO_CONFIG_FORWARD_HOST_MAX))
        {
          conffile_seterrnum(cf, CONFFILE_ERR_PARSE_ARG_TOOMANY);
          return -1;
        }

      if (strlen(data->stringlist[0]) > CEREBRO_MAX_HOSTNAME_LEN)
        {
          conffile_seterrnum(cf, CONFFILE_ERR_PARSE_OVERFLOW_ARGLEN);
          return -1;
        }

      strcpy(conf->cerebrod_forward_message_config[index].ip, data->stringlist[0]);

      if (data->stringlist_len > 1)
        {
          int port;
          char *endptr;

          port = strtol(data->stringlist[1], &endptr, 0);
          if (endptr != (data->stringlist[1] + strlen(data->stringlist[1])))
            {
              conffile_seterrnum(cf, CONFFILE_ERR_PARSE_ARG_INVALID);
              return -1;
            }

          conf->cerebrod_forward_message_config[index].destination_port = port;
        }
      else
        conf->cerebrod_forward_message_config[index].destination_port = CEREBRO_CONFIG_PORT_DEFAULT;

      if (data->stringlist_len > 2)
        {
          int port;
          char *endptr;

          port = strtol(data->stringlist[2], &endptr, 0);
          if (endptr != (data->stringlist[2] + strlen(data->stringlist[2])))
            {
              conffile_seterrnum(cf, CONFFILE_ERR_PARSE_ARG_INVALID);
              return -1;
            }

          conf->cerebrod_forward_message_config[index].source_port = port;
        }
      else
        conf->cerebrod_forward_message_config[index].source_port = CEREBRO_CONFIG_PORT_DEFAULT;

      if (data->stringlist_len > 3)
        {
          if (strlen(data->stringlist[3]) > CEREBRO_MAX_HOSTNAME_LEN)
            {
              conffile_seterrnum(cf, CONFFILE_ERR_PARSE_OVERFLOW_ARGLEN);
              return -1;
            }
          strcpy(conf->cerebrod_forward_message_config[index].network_interface, data->stringlist[3]);
        }
      else
        strcpy(conf->cerebrod_forward_message_config[index].network_interface, CEREBRO_CONFIG_IP_DEFAULT);

      if (data->stringlist_len > 4)
        {
          int i;

          for (i = 4; i < data->stringlist_len; i++)
            {
              int host_index = conf->cerebrod_forward_message_config[index].host_len;
              if (strlen(data->stringlist[i]) > CEREBRO_CONFIG_HOST_INPUT_MAX)
                {
                  conffile_seterrnum(cf, CONFFILE_ERR_PARSE_OVERFLOW_ARGLEN);
                  return -1;
                }
              strcpy(conf->cerebrod_forward_message_config[index].host[host_index],
                     data->stringlist[i]);
              conf->cerebrod_forward_message_config[index].host_len++;
            }
        }

      conf->cerebrod_forward_message_config_len++;
    }

  return 0;
}

/*
 * _cb_cerebrod_forward_host_accept
 *
 * callback function that parses and stores cerebrod forward host
 * accept configuration.
 *
 * Returns 0 on success, -1 on error
 */
static int
_cb_cerebrod_forward_host_accept(conffile_t cf, struct conffile_data *data,
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

      for (i = 0; i < data->stringlist_len; i++)
        {
          if (conf->cerebrod_forward_host_accept_len >= CEREBRO_CONFIG_FORWARD_HOST_ACCEPT_MAX)
            {
              conffile_seterrnum(cf, CONFFILE_ERR_PARSE_ARG_TOOMANY);
              return -1;
            }

          if (strlen(data->stringlist[i]) > CEREBRO_CONFIG_HOST_INPUT_MAX)
            {
              conffile_seterrnum(cf, CONFFILE_ERR_PARSE_OVERFLOW_ARGLEN);
              return -1;
            }

          strcpy(conf->cerebrod_forward_host_accept[conf->cerebrod_forward_host_accept_len],
                 data->stringlist[i]);
          conf->cerebrod_forward_host_accept_len++;
        }
    }
  return 0;
}

/*
 * _cb_cerebrod_module_exclude
 *
 * callback function that parses and stores cerebrod module exclude
 * configuration.
 *
 * Returns 0 on success, -1 on error
 */
static int
_cb_cerebrod_module_exclude(conffile_t cf, struct conffile_data *data,
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

      for (i = 0; i < data->stringlist_len; i++)
        {
          if (strlen(data->stringlist[i]) > CEREBRO_MAX_MODULE_NAME_LEN)
            {
              conffile_seterrnum(cf, CONFFILE_ERR_PARSE_OVERFLOW_ARGLEN);
              return -1;
            }

          /* achu: passing char[][] as pointer is badness, play it safe */
          if (!strcasecmp(optionname,  "cerebrod_metric_module_exclude"))
            {
              if (conf->cerebrod_metric_module_exclude_len >= CEREBRO_CONFIG_METRIC_MODULE_EXCLUDE_MAX)
                {
                  conffile_seterrnum(cf, CONFFILE_ERR_PARSE_ARG_TOOMANY);
                  return -1;
                }

              strcpy(conf->cerebrod_metric_module_exclude[conf->cerebrod_metric_module_exclude_len],
                     data->stringlist[i]);
              conf->cerebrod_metric_module_exclude_len++;
            }
          else if (!strcasecmp(optionname,  "cerebrod_monitor_module_exclude"))
            {
              if (conf->cerebrod_monitor_module_exclude_len >= CEREBRO_CONFIG_MONITOR_MODULE_EXCLUDE_MAX)
                {
                  conffile_seterrnum(cf, CONFFILE_ERR_PARSE_ARG_TOOMANY);
                  return -1;
                }

              strcpy(conf->cerebrod_monitor_module_exclude[conf->cerebrod_monitor_module_exclude_len],
                     data->stringlist[i]);
              conf->cerebrod_monitor_module_exclude_len++;
            }
          else if (!strcasecmp(optionname,  "cerebrod_event_module_exclude"))
            {
              if (conf->cerebrod_event_module_exclude_len >= CEREBRO_CONFIG_EVENT_MODULE_EXCLUDE_MAX)
                {
                  conffile_seterrnum(cf, CONFFILE_ERR_PARSE_ARG_TOOMANY);
                  return -1;
                }

              strcpy(conf->cerebrod_event_module_exclude[conf->cerebrod_event_module_exclude_len],
                     data->stringlist[i]);
              conf->cerebrod_event_module_exclude_len++;
            }
        }
    }
  return 0;
}

#if CEREBRO_DEBUG
/*
 * _cb_cerebrod_alternate_hostname
 *
 * callback function that parses and stores cerebrod alternate hostname
 *
 * Returns 0 on success, -1 on error
 */
static int
_cb_cerebrod_alternate_hostname(conffile_t cf, struct conffile_data *data,
                                char *optionname, int option_type, void *option_ptr,
                                int option_data, void *app_ptr, int app_data)
{
  struct cerebro_config *conf;
  char *p;

  if (!option_ptr)
    {
      conffile_seterrnum(cf, CONFFILE_ERR_PARAMETERS);
      return -1;
    }

  conf = (struct cerebro_config *)option_ptr;

  if (strlen(data->string) > CEREBRO_MAX_HOSTNAME_LEN)
    {
      conffile_seterrnum(cf, CONFFILE_ERR_PARSE_OVERFLOW_ARGLEN);
      return -1;
    }

  strcpy(conf->cerebrod_alternate_hostname, data->string);
  /* shorten hostname if necessary */
  if ((p = strchr(conf->cerebrod_alternate_hostname, '.')))
    *p = '\0';
  conf->cerebrod_alternate_hostname_flag++;
  return 0;
}
#endif /* CEREBRO_DEBUG */

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
        "cerebro_metric_server",
        CONFFILE_OPTION_LIST_STRING,
        -1,
        _cb_cerebro_metric_server,
        CEREBRO_CONFIG_CEREBRO_METRIC_SERVERS_MAX,
        0,
        &(conf->cerebro_metric_server_flag),
        conf,
        0
      },
      {
        "cerebro_event_server",
        CONFFILE_OPTION_LIST_STRING,
        -1,
        _cb_cerebro_event_server,
        CEREBRO_CONFIG_CEREBRO_EVENT_SERVERS_MAX,
        0,
        &(conf->cerebro_event_server_flag),
        conf,
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
        "cerebrod_speak_message_config",
        CONFFILE_OPTION_LIST_STRING,
        -1,
        _cb_cerebrod_speak_message_config,
        CEREBRO_CONFIG_SPEAK_MESSAGE_CONFIG_MAX,
        0,
        &(conf->cerebrod_speak_message_config_flag),
        conf,
        0
      },
      {
        "cerebrod_speak_message_ttl",
        CONFFILE_OPTION_INT,
        -1,
        conffile_int,
        1,
        0,
        &(conf->cerebrod_speak_message_ttl_flag),
        &(conf->cerebrod_speak_message_ttl),
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
        "cerebrod_listen_message_config",
        CONFFILE_OPTION_LIST_STRING,
        -1,
        _cb_cerebrod_listen_message_config,
        CEREBRO_CONFIG_LISTEN_MESSAGE_CONFIG_MAX,
        0,
        &(conf->cerebrod_listen_message_config_flag),
        conf,
        0
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
      {
        "cerebrod_forward_message_config",
        CONFFILE_OPTION_LIST_STRING,
        -1,
        _cb_cerebrod_forward_message_config,
        CEREBRO_CONFIG_FORWARD_MESSAGE_CONFIG_MAX,
        0,
        &(conf->cerebrod_forward_message_config_flag),
        conf,
        0
      },
      {
        "cerebrod_forward_message_ttl",
        CONFFILE_OPTION_INT,
        -1,
        conffile_int,
        1,
        0,
        &(conf->cerebrod_forward_message_ttl_flag),
        &(conf->cerebrod_forward_message_ttl),
        0
      },
      {
        "cerebrod_forward_host_accept",
        CONFFILE_OPTION_LIST_STRING,
        -1,
        _cb_cerebrod_forward_host_accept,
        CEREBRO_CONFIG_FORWARD_HOST_ACCEPT_MAX,
        0,
        &(conf->cerebrod_forward_host_accept_flag),
        conf,
        0
      },
      {
        "cerebrod_metric_module_exclude",
        CONFFILE_OPTION_LIST_STRING,
        -1,
        _cb_cerebrod_module_exclude,
        CEREBRO_CONFIG_METRIC_MODULE_EXCLUDE_MAX,
        0,
        &(conf->cerebrod_metric_module_exclude_flag),
        conf,
        0
      },
      {
        "cerebrod_monitor_module_exclude",
        CONFFILE_OPTION_LIST_STRING,
        -1,
        _cb_cerebrod_module_exclude,
        CEREBRO_CONFIG_MONITOR_MODULE_EXCLUDE_MAX,
        0,
        &(conf->cerebrod_monitor_module_exclude_flag),
        conf,
        0
      },
      {
        "cerebrod_event_module_exclude",
        CONFFILE_OPTION_LIST_STRING,
        -1,
        _cb_cerebrod_module_exclude,
        CEREBRO_CONFIG_EVENT_MODULE_EXCLUDE_MAX,
        0,
        &(conf->cerebrod_event_module_exclude_flag),
        conf,
        0
      },
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
      {
        "cerebrod_gettimeofday_workaround",
        CONFFILE_OPTION_BOOL,
        -1,
        conffile_bool,
        1,
        0,
        &(conf->cerebrod_gettimeofday_workaround_flag),
        &conf->cerebrod_gettimeofday_workaround,
        0
      },
#if CEREBRO_DEBUG
      {
        "cerebrod_alternate_hostname",
        CONFFILE_OPTION_STRING,
        -1,
        _cb_cerebrod_alternate_hostname,
        1,
        0,
        &(conf->cerebrod_alternate_hostname_flag),
        conf,
        0
      },
#endif /* CEREBRO_DEBUG */
    };
  conffile_t cf = NULL;
  int num;

  if (!(cf = conffile_handle_create()))
    {
      CEREBRO_ERR(("conffile_handle_create"));
      if (errnum)
        *errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  if (!config_debug_config_file)
    config_file = CEREBRO_CONFIG_FILE_DEFAULT;
  else
    config_file = config_debug_config_file;

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
  int i, j;

  if (!dest || !src)
    {
      CEREBRO_DBG(("invalid parameters"));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (!dest->cerebro_metric_server_flag && src->cerebro_metric_server_flag)
    {
      for (i = 0; i < src->cerebro_metric_server_len; i++)
        {
          strcpy(dest->cerebro_metric_server[i].hostname, src->cerebro_metric_server[i].hostname);
          dest->cerebro_metric_server[i].port = src->cerebro_metric_server[i].port;
        }
      dest->cerebro_metric_server_len = src->cerebro_metric_server_len;
      dest->cerebro_metric_server_flag++;
    }

  if (!dest->cerebro_event_server_flag && src->cerebro_event_server_flag)
    {
      for (i = 0; i < src->cerebro_event_server_len; i++)
        {
          strcpy(dest->cerebro_event_server[i].hostname, src->cerebro_event_server[i].hostname);
          dest->cerebro_event_server[i].port = src->cerebro_event_server[i].port;
        }
      dest->cerebro_event_server_len = src->cerebro_event_server_len;
      dest->cerebro_event_server_flag++;
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

  if (!dest->cerebrod_speak_flag && src->cerebrod_speak_flag)
    {
      dest->cerebrod_speak = src->cerebrod_speak;
      dest->cerebrod_speak_flag++;
    }

  if (!dest->cerebrod_speak_message_config_flag
      && src->cerebrod_speak_message_config_flag
      && src->cerebrod_speak_message_config_len)
    {
      for (i = 0; i < src->cerebrod_speak_message_config_len; i++)
        {
          strcpy(dest->cerebrod_speak_message_config[i].ip, src->cerebrod_speak_message_config[i].ip);
          dest->cerebrod_speak_message_config[i].destination_port = src->cerebrod_speak_message_config[i].destination_port;
          dest->cerebrod_speak_message_config[i].source_port = src->cerebrod_speak_message_config[i].source_port;
          strcpy(dest->cerebrod_speak_message_config[i].network_interface, src->cerebrod_speak_message_config[i].network_interface);
        }
      dest->cerebrod_speak_message_config_len = src->cerebrod_speak_message_config_len;
      dest->cerebrod_speak_message_config_flag++;
    }

  if (!dest->cerebrod_speak_message_ttl_flag && src->cerebrod_speak_message_ttl_flag)
    {
      dest->cerebrod_speak_message_ttl = src->cerebrod_speak_message_ttl;
      dest->cerebrod_speak_message_ttl_flag++;
    }

  if (!dest->cerebrod_listen_flag && src->cerebrod_listen_flag)
    {
      dest->cerebrod_listen = src->cerebrod_listen;
      dest->cerebrod_listen_flag++;
    }

  if (!dest->cerebrod_listen_message_config_flag
      && src->cerebrod_listen_message_config_flag
      && src->cerebrod_listen_message_config_len)
    {
      for (i = 0; i < src->cerebrod_listen_message_config_len; i++)
        {
          strcpy(dest->cerebrod_listen_message_config[i].ip, src->cerebrod_listen_message_config[i].ip);
          dest->cerebrod_listen_message_config[i].port = src->cerebrod_listen_message_config[i].port;
          strcpy(dest->cerebrod_listen_message_config[i].network_interface, src->cerebrod_listen_message_config[i].network_interface);
        }
      dest->cerebrod_listen_message_config_len = src->cerebrod_listen_message_config_len;
      dest->cerebrod_listen_message_config_flag++;
    }

  if (!dest->cerebrod_listen_threads_flag && src->cerebrod_listen_threads_flag)
    {
      dest->cerebrod_listen_threads = src->cerebrod_listen_threads;
      dest->cerebrod_listen_threads_flag++;
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

  if (!dest->cerebrod_forward_message_config_flag
      && src->cerebrod_forward_message_config_flag
      && src->cerebrod_forward_message_config_len)
    {
      for (i = 0; i < src->cerebrod_forward_message_config_len; i++)
        {
          strcpy(dest->cerebrod_forward_message_config[i].ip, src->cerebrod_forward_message_config[i].ip);
          dest->cerebrod_forward_message_config[i].destination_port = src->cerebrod_forward_message_config[i].destination_port;
          dest->cerebrod_forward_message_config[i].source_port = src->cerebrod_forward_message_config[i].source_port;
          strcpy(dest->cerebrod_forward_message_config[i].network_interface, src->cerebrod_forward_message_config[i].network_interface);
          for (j = 0; j < src->cerebrod_forward_message_config[i].host_len; j++)
            strcpy(dest->cerebrod_forward_message_config[i].host[j],
                   src->cerebrod_forward_message_config[i].host[j]);
          dest->cerebrod_forward_message_config[i].host_len = src->cerebrod_forward_message_config[i].host_len;
        }
      dest->cerebrod_forward_message_config_len = src->cerebrod_forward_message_config_len;
      dest->cerebrod_forward_message_config_flag++;
    }

  if (!dest->cerebrod_forward_message_ttl_flag && src->cerebrod_forward_message_ttl_flag)
    {
      dest->cerebrod_forward_message_ttl = src->cerebrod_forward_message_ttl;
      dest->cerebrod_forward_message_ttl_flag++;
    }

  if (!dest->cerebrod_forward_host_accept_flag
      && src->cerebrod_forward_host_accept_flag
      && src->cerebrod_forward_host_accept_len)
    {
      for (i = 0; i < src->cerebrod_forward_host_accept_len; i++)
        strcpy(dest->cerebrod_forward_host_accept[i],
               src->cerebrod_forward_host_accept[i]);
      dest->cerebrod_forward_host_accept_len = src->cerebrod_forward_host_accept_len;
      dest->cerebrod_forward_host_accept_flag++;
    }

  if (!dest->cerebrod_metric_module_exclude_flag
      && src->cerebrod_metric_module_exclude_flag
      && src->cerebrod_metric_module_exclude_len)
    {
      for (i = 0; i < src->cerebrod_metric_module_exclude_len; i++)
        strcpy(dest->cerebrod_metric_module_exclude[i],
               src->cerebrod_metric_module_exclude[i]);
      dest->cerebrod_metric_module_exclude_len = src->cerebrod_metric_module_exclude_len;
      dest->cerebrod_metric_module_exclude_flag++;
    }

  if (!dest->cerebrod_monitor_module_exclude_flag
      && src->cerebrod_monitor_module_exclude_flag
      && src->cerebrod_monitor_module_exclude_len)
    {
      for (i = 0; i < src->cerebrod_monitor_module_exclude_len; i++)
        strcpy(dest->cerebrod_monitor_module_exclude[i],
               src->cerebrod_monitor_module_exclude[i]);
      dest->cerebrod_monitor_module_exclude_len = src->cerebrod_monitor_module_exclude_len;
      dest->cerebrod_monitor_module_exclude_flag++;
    }

  if (!dest->cerebrod_event_module_exclude_flag
      && src->cerebrod_event_module_exclude_flag
      && src->cerebrod_event_module_exclude_len)
    {
      for (i = 0; i < src->cerebrod_event_module_exclude_len; i++)
        strcpy(dest->cerebrod_event_module_exclude[i],
               src->cerebrod_event_module_exclude[i]);
      dest->cerebrod_event_module_exclude_len = src->cerebrod_event_module_exclude_len;
      dest->cerebrod_event_module_exclude_flag++;
    }

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

  if (!dest->cerebrod_gettimeofday_workaround_flag
      && src->cerebrod_gettimeofday_workaround_flag)
    {
      dest->cerebrod_gettimeofday_workaround = src->cerebrod_gettimeofday_workaround;
      dest->cerebrod_gettimeofday_workaround_flag++;
    }

#if CEREBRO_DEBUG
  if (!dest->cerebrod_alternate_hostname_flag
      && src->cerebrod_alternate_hostname_flag)
    {
      strcpy(dest->cerebrod_alternate_hostname, src->cerebrod_alternate_hostname);
      dest->cerebrod_alternate_hostname_flag++;
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
