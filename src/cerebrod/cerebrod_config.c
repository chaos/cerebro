/*****************************************************************************\
 *  $Id: cerebrod_config.c,v 1.1.1.1 2004-07-02 22:31:29 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "cerebrod_config.h"
#include "conffile.h"
#include "error.h"
#include "wrappers.h"

extern struct cerebrod_config conf;

void
cerebrod_config_default(void)
{
  conf.debug = 0;
  conf.configfile = CEREBROD_CONFIGFILE_DEFAULT;
  conf.heartbeat_freq_min = CEREBROD_HEARTBEAT_FREQ_MIN_DEFAULT;
  conf.heartbeat_freq_max = CEREBROD_HEARTBEAT_FREQ_MAX_DEFAULT;
  conf.network_interface = NULL;
  conf.mcast_ip = CEREBROD_MCAST_IP_DEFAULT;
  conf.mcast_port = CEREBROD_MCAST_PORT_DEFAULT;
  conf.mcast_listen_threads = CEREBROD_MCAST_LISTEN_THREADS_DEFAULT;
}

static void
_usage(void)
{
  fprintf(stderr, "Usage: cerebrod [OPTIONS]\n"
          "-h    --help        Output Help\n"
          "-v    --version     Output Version\n"
          "-c    --configfile  Specify alternate config file\n");
#ifndef NDEBUG
  fprintf(stderr, 
          "-d    --debug       Turn on debugging\n");
#endif /* NDEBUG */
  exit(0);
}

static void
_version(void)
{
  fprintf(stderr, "%s %s-%s\n", PROJECT, VERSION, RELEASE);
  exit(0);
}

void
cerebrod_cmdline_parse(int argc, char **argv)
{ 
  char c;
#ifndef NDEBUG
  char *options = "hvc:d";
#else
  char *options = "hvc:";
#endif

#if HAVE_GETOPT_LONG
  struct option long_options[] =
    {
      {"help",                0, NULL, 'h'},
      {"version",             0, NULL, 'v'},
      {"configfile",          1, NULL, 'c'},
#ifndef NDEBUG
      {"debug",               0, NULL, 'd'},
#endif /* NDEBUG */
    };
#endif /* HAVE_GETOPT_LONG */

  /* turn off output messages */
  opterr = 0;

#if HAVE_GETOPT_LONG
  while ((c = getopt_long(argc, argv, options, long_options, NULL)) != -1)
#else
  while ((c = getopt(argc, argv, options)) != -1)
#endif
    {
      switch (c)
        {
        case 'h':       /* --help */
          _usage();
          break;
        case 'v':       /* --version */
          _version();
          break;
        case 'c':       /* --configfile */
          conf.configfile = Strdup(optarg);
          break;
#ifndef NDEBUG
        case 'd':       /* --debug */
          conf.debug = 1;
          break;
#endif /* NDEBUG */
        case '?':
        default:
          err_exit("unknown command line option '%c'", optopt);
        }          
    }
}

static int
_cb_heartbeat_freq(conffile_t cf, struct conffile_data *data,
                   char *optionname, int option_type, void *option_ptr,
                   int option_data, void *app_ptr, int app_data)
{
  /* No need to check length of list, conffile checks it for you */

  if (data->intlist[0] > data->intlist[1])
    {
      conffile_seterrnum(cf, CONFFILE_ERR_PARAMETERS);
      return -1;
    }
  conf.heartbeat_freq_min = data->intlist[0];
  conf.heartbeat_freq_max = data->intlist[1];
  return 0;
}

static int
_cb_stringptr(conffile_t cf, struct conffile_data *data,
              char *optionname, int option_type, void *option_ptr,
              int option_data, void *app_ptr, int app_data)
{
  if (option_ptr == NULL)
    {
      conffile_seterrnum(cf, CONFFILE_ERR_PARAMETERS);
      return -1;
    }

  *((char **)option_ptr) = Strdup(data->string);
  return 0;
}


void
cerebrod_config_parse(void)
{
  int heartbeat_freq_flag, network_interface_flag, mcast_ip_flag, 
    mcast_port_flag, mcast_listen_threads_flag;

  struct conffile_option options[] =
    {
      {"heartbeat_freq", CONFFILE_OPTION_LIST_INT, 2, _cb_heartbeat_freq,
       1, 0, &heartbeat_freq_flag, NULL, 0},
      {"network_interface", CONFFILE_OPTION_STRING, -1, _cb_stringptr,
       1, 0, &network_interface_flag, &(conf.network_interface), 0},
      {"mcast_ip", CONFFILE_OPTION_STRING, -1, _cb_stringptr,
       1, 0, &mcast_ip_flag, &(conf.mcast_ip), 0},
      {"mcast_port", CONFFILE_OPTION_INT, -1, conffile_int,
       1, 0, &mcast_port_flag, &(conf.mcast_port), 0},
      {"mcast_listen_threads", CONFFILE_OPTION_INT, -1, conffile_int,
       1, 0, &mcast_listen_threads_flag, &(conf.mcast_listen_threads), 0},
    };
  conffile_t cf = NULL;
  int num;

  if ((cf = conffile_handle_create()) == NULL) 
    err_exit("conffile_handle_create: failed to create handle");

  num = sizeof(options)/sizeof(struct conffile_option);
  if (conffile_parse(cf, conf.configfile, options, num, NULL, 0, 0) < 0)
    {
      /* Its not an error if the file doesn't exist */
      if (conffile_errnum(cf) != CONFFILE_ERR_EXIST)
        {
          char buf[CONFFILE_MAX_ERRMSGLEN];
          if (conffile_errmsg(cf, buf, CONFFILE_MAX_ERRMSGLEN) < 0)
            err_exit("conffile_parse: errnum = %d", conffile_errnum(cf));
          err_exit("conffile_parse: %s", buf);
        }
    }

  conffile_handle_destroy(cf);
}
