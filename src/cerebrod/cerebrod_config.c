/*****************************************************************************\
 *  $Id: cerebrod_config.c,v 1.46 2005-03-20 20:21:18 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#if HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */
#include <assert.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <sys/param.h>

#include "cerebrod_config.h"
#include "cerebrod_clusterlist.h"
#include "cerebrod_util.h"
#include "conffile.h"
#include "error.h"
#include "wrappers.h"

#define MULTICAST_CLASS_MIN 224
#define MULTICAST_CLASS_MAX 239
#define IPADDR_BITS          32   
#define IPADDR6_BITS        128

/* 
 * conf
 *
 * cerebrod configuration used by all of cerebrod
 */
struct cerebrod_config conf;

/* config_modules
 * 
 * configuration modules to search for by default
 */
char *config_modules[] = {
  "cerebrod_config_gendersllnl.la",
  NULL
};
int config_modules_len = 1;

/* 
 * _cerebrod_config_default 
 *
 * initialize conf structure with default values.
 */
static void
_cerebrod_config_default(void)
{
  memset(&conf, '\0', sizeof(struct cerebrod_config));

  conf.debug = CEREBROD_DEBUG_DEFAULT;
  conf.config_file = CEREBROD_CONFIG_FILE_DEFAULT;
  conf.heartbeat_frequency_min = CEREBROD_HEARTBEAT_FREQUENCY_MIN_DEFAULT;
  conf.heartbeat_frequency_max = CEREBROD_HEARTBEAT_FREQUENCY_MAX_DEFAULT;
  conf.heartbeat_source_port = CEREBROD_HEARTBEAT_SOURCE_PORT_DEFAULT;
  conf.heartbeat_destination_port = CEREBROD_HEARTBEAT_DESTINATION_PORT_DEFAULT;
  conf.heartbeat_destination_ip = CEREBROD_HEARTBEAT_DESTINATION_IP_DEFAULT;
  conf.heartbeat_network_interface = CEREBROD_HEARTBEAT_NETWORK_INTERFACE_DEFAULT;
  conf.heartbeat_ttl = CEREBROD_HEARTBEAT_TTL_DEFAULT;
  conf.speak = CEREBROD_SPEAK_DEFAULT;
  conf.listen = CEREBROD_LISTEN_DEFAULT;
  conf.listen_threads = CEREBROD_LISTEN_THREADS_DEFAULT;
  conf.updown_server = CEREBROD_UPDOWN_SERVER_DEFAULT;
  conf.updown_server_port = CEREBROD_UPDOWN_SERVER_PORT_DEFAULT;
  conf.clusterlist_module = CEREBROD_CLUSTERLIST_MODULE_DEFAULT;
  conf.clusterlist_module_options = CEREBROD_CLUSTERLIST_MODULE_OPTIONS_DEFAULT;
  conf.speak_debug = CEREBROD_SPEAK_DEBUG_DEFAULT;
  conf.listen_debug = CEREBROD_LISTEN_DEBUG_DEFAULT;
  conf.updown_server_debug = CEREBROD_UPDOWN_SERVER_DEBUG_DEFAULT;
}

/* 
 * _usage 
 * 
 * output usage and exit
 */
static void
_usage(void)
{
  fprintf(stderr, "Usage: cerebrod [OPTIONS]\n"
          "-h    --help          Output Help\n"
          "-v    --version       Output Version\n"
          "-c    --config_file   Specify alternate config file\n"
          "-m    --config_module Specify configuration module\n");
#ifndef NDEBUG
  fprintf(stderr, 
          "-d    --debug         Turn on debugging and run daemon in foreground\n");
#endif /* NDEBUG */
  exit(0);
}

/* 
 * _version 
 * 
 * output version and exit
 */
static void
_version(void)
{
  fprintf(stderr, "%s %s-%s\n", PROJECT, VERSION, RELEASE);
  exit(0);
}

/* 
 * _cerebrod_cmdline_parse
 *
 * parse command line options
 */
static void
_cerebrod_cmdline_parse(int argc, char **argv)
{ 
  char c;
#ifndef NDEBUG
  char *options = "hvc:m:d";
#else
  char *options = "hvc:m:";
#endif /* NDEBUG */

  assert(argv);

#if HAVE_GETOPT_LONG
  struct option long_options[] =
    {
      {"help",                0, NULL, 'h'},
      {"version",             0, NULL, 'v'},
      {"config_file",          1, NULL, 'c'},
      {"config_module",        1, NULL, 'm'},
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
        case 'c':       /* --config_file */
          conf.config_file = Strdup(optarg);
          break;
        case 'm':       /* --config_module */
          conf.config_module = Strdup(optarg);
          break;
#ifndef NDEBUG
        case 'd':       /* --debug */
          conf.debug++;
          break;
#endif /* NDEBUG */
        case '?':
        default:
          err_exit("unknown command line option '%c'", optopt);
        }          
    }
}

/* 
 * _cerebrod_cmdline_parse_check:
 *
 * check validity of command line parameters
 */
static void
_cerebrod_cmdline_parse_check(void)
{
  /* Check if the configuration file exists */
  if (conf.config_file)
    {
      /* The default config_file is allowed to be missing, so don't
       * bother checking if it exists.
       */
      if (strcmp(conf.config_file, CEREBROD_CONFIG_FILE_DEFAULT) != 0)
        {
          struct stat buf;
  
          if (stat(conf.config_file, &buf) < 0)
            err_exit("config file '%s' not found", conf.config_file);
        }
    }

  /* Check if the configuration module exists */
  if (conf.config_module)
    {
      struct stat buf;

      /* Don't use stat() wrapper, we want to receive the error */

      if (conf.config_module[0] == '/')
	{
	  /* Case A: User specified absolute path to config module */
	  if (stat(conf.config_module, &buf) < 0)
	    err_exit("config module '%s' not found", 
		     conf.config_module);
	  
	  conf.config_module_file = Strdup(conf.config_module);
	}
      else
	{
	  /* Case B: Search for config module */
	  char filebuf[MAXPATHLEN+1];

	  /* Assume user passed in config filename.  Search in
	   * module directory.
	   */
	  memset(filebuf, '\0', MAXPATHLEN+1);
	  snprintf(filebuf, MAXPATHLEN, "%s/%s", 
		   CEREBROD_MODULE_DIR, conf.config_module);

	  if (!stat(filebuf, &buf))
	    {
	      conf.config_module_file = Strdup(filebuf);
	      return;
	    }

	  /* Assume user passed in config filename.  Search in
	   * current directory.
	   */
	  memset(filebuf, '\0', MAXPATHLEN+1);
	  snprintf(filebuf, MAXPATHLEN, "./%s", conf.config_module);

	  if (!stat(filebuf, &buf))
	    {
	      conf.config_module_file = Strdup(filebuf);
	      return;
	    }

	  /* Assume user passed in config module name.  Search in
	   * module directory.
	   */
	  memset(filebuf, '\0', MAXPATHLEN+1);
	  snprintf(filebuf, MAXPATHLEN, "%s/cerebrod_config_%s.la", 
		   CEREBROD_MODULE_DIR, conf.config_module);

	  if (!stat(filebuf, &buf))
	    {
	      conf.config_module_file = Strdup(filebuf);
	      return;
	    }

	  /* Assume user passed in config module name.  Search in
	   * current directory.
	   */
	  memset(filebuf, '\0', MAXPATHLEN+1);
	  snprintf(filebuf, MAXPATHLEN, "./cerebrod_config_%s.la", conf.config_module);

	  if (!stat(filebuf, &buf))
	    {
	      conf.config_module_file = Strdup(filebuf);
	      return;
	    }

	  if (!conf.config_module_file)
	    err_exit("config module '%s' not found", conf.config_module);
	}
    }
}

/* 
 * _config_load_module
 *
 * load a configuration module and all appropriate symbols
 *
 * - module_path - full path to config module to load
 *
 * Returns 1 on loading success, 0 on loading failure, -1 on fatal error
 */
static int
_config_load_module(char *module_path)
{
  lt_dlhandle config_module_dl_handle = NULL;
  struct cerebrod_config_module_info *config_module_info = NULL;
  struct cerebrod_config_module_ops *config_module_ops = NULL;

  assert(module_path);

  config_module_dl_handle = Lt_dlopen(module_path);
  config_module_info = (struct cerebrod_config_module_info *)Lt_dlsym(config_module_dl_handle, "config_module_info");
  config_module_ops = (struct cerebrod_config_module_ops *)Lt_dlsym(config_module_dl_handle, "config_module_ops");

  if (!config_module_info->config_module_name)
    err_exit("config module '%s' does not contain a valid name");

  if (!config_module_ops->load_default)
    err_exit("config module '%s' does not contain valid load_default function");
  
#ifndef NDEBUG
  if (conf.debug)
    {
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Cerebrod Config Configuration:\n");
      fprintf(stderr, "* -----------------------\n");
      fprintf(stderr, "* Loaded config module: %s\n", module_path); 
      fprintf(stderr, "**************************************\n");
    }
#endif /* NDEBUG */

  /* Load the alternate default configuration from the configuration module */
  if ((*config_module_ops->load_default)(&conf) < 0)
    err_exit("%s config module: load_default failed: %s",
             config_module_info->config_module_name, strerror(errno));

  Lt_dlclose(config_module_dl_handle);

  return 1;
}

/*
 * _config_module_setup
 *
 * load configuration module.  search for configuration module to load
 * if necessary
 */
static void
_cerebrod_config_module_setup(void)
{
  Lt_dlinit();

  if (conf.config_module_file)
    {
      if (_config_load_module(conf.config_module_file) != 1)
	err_exit("config module '%s' could not be loaded",
		 conf.config_module);
    }
  else
    {
      if (cerebrod_search_dir_for_module(CEREBROD_MODULE_DIR,
                                         config_modules,
					 config_modules_len,
                                         _config_load_module))
        goto done;
      
      if (cerebrod_search_dir_for_module(".",
                                         config_modules,
					 config_modules_len,
                                         _config_load_module))
        goto done;
    }

 done:
  Lt_dlexit();
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
_cb_heartbeat_freq(conffile_t cf, struct conffile_data *data,
                   char *optionname, int option_type, void *option_ptr,
                   int option_data, void *app_ptr, int app_data)
{
  if (data->intlist_len == 1)
    {
      conf.heartbeat_frequency_min = data->intlist[0];
      conf.heartbeat_frequency_max = 0;
    }
  else if (data->intlist_len == 2)
    {
      if (data->intlist[0] > data->intlist[1])
	{
	  conffile_seterrnum(cf, CONFFILE_ERR_PARAMETERS);
	  return -1;
	}
      conf.heartbeat_frequency_min = data->intlist[0];
      conf.heartbeat_frequency_max = data->intlist[1];
    }
  else
    {
      conffile_seterrnum(cf, CONFFILE_ERR_PARSE_OVERFLOW_ARGLEN);
      return -1;
    }

  return 0;
}

/*
 * _cb_stringptr
 *
 * conffile callback function that parses and stores a string
 *
 * Returns 0 on success, -1 on error
 */
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

/*
 * _cb_module_options
 *
 * conffile callback function that parses and stores an array of
 * clusterlist module options.
 *
 * Returns 0 on success, -1 on error
 */
static int
_cb_module_options(conffile_t cf, struct conffile_data *data,
		   char *optionname, int option_type, void *option_ptr,
		   int option_data, void *app_ptr, int app_data)
{
  if (option_ptr == NULL)
    {
      conffile_seterrnum(cf, CONFFILE_ERR_PARAMETERS);
      return -1;
    }

  if (data->stringlist_len > 0)
    {
      char ***p = (char ***)option_ptr; 
      int i;

      *p = (char **)Malloc(sizeof(char *) * (data->stringlist_len + 1));

      for (i = 0; i < data->stringlist_len; i++)
	(*p)[i] = Strdup(data->stringlist[i]);
      (*p)[i] = NULL;
    }

  return 0;
}

/*
 * _cb_config_parse
 * 
 * Using the conffile configuration file parsing library, parse the cerebrod
 * configuration file.
 */
static void
_cerebrod_config_parse(void)
{
  int heartbeat_frequency_flag, heartbeat_source_port_flag, 
    heartbeat_destination_port_flag, heartbeat_destination_ip_flag, 
    heartbeat_network_interface_flag, heartbeat_ttl_flag, speak_flag, 
    listen_flag, listen_threads_flag, updown_server_flag, 
    updown_server_port_flag, clusterlist_module_flag, 
    clusterlist_module_options_flag, speak_debug_flag, listen_debug_flag, 
    updown_server_debug_flag;
  
  struct conffile_option options[] =
    {
      {"heartbeat_frequency", CONFFILE_OPTION_LIST_INT, -1, _cb_heartbeat_freq,
       1, 0, &heartbeat_frequency_flag, NULL, 0},
      {"heartbeat_source_port", CONFFILE_OPTION_INT, -1, conffile_int,
       1, 0, &heartbeat_source_port_flag, &(conf.heartbeat_source_port), 0},
      {"heartbeat_destination_port", CONFFILE_OPTION_INT, -1, conffile_int,
       1, 0, &heartbeat_destination_port_flag, &(conf.heartbeat_destination_port), 0},
      {"heartbeat_destination_ip", CONFFILE_OPTION_STRING, -1, _cb_stringptr,
       1, 0, &heartbeat_destination_ip_flag, &(conf.heartbeat_destination_ip), 0},
      {"heartbeat_network_interface", CONFFILE_OPTION_STRING, -1, _cb_stringptr,
       1, 0, &heartbeat_network_interface_flag, 
       &(conf.heartbeat_network_interface), 0},
      {"heartbeat_ttl", CONFFILE_OPTION_INT, -1, conffile_int,
       1, 0, &heartbeat_ttl_flag, &(conf.heartbeat_ttl), 0},
      {"speak", CONFFILE_OPTION_BOOL, -1, conffile_bool,
       1, 0, &speak_flag, &conf.speak, 0},
      {"listen", CONFFILE_OPTION_BOOL, -1, conffile_bool,
       1, 0, &listen_flag, &conf.listen, 0},
      {"listen_threads", CONFFILE_OPTION_INT, -1, conffile_int,
       1, 0, &listen_threads_flag, &(conf.listen_threads), 0},
      {"updown_server", CONFFILE_OPTION_BOOL, -1, conffile_bool,
       1, 0, &updown_server_flag, &conf.updown_server, 0},
      {"updown_server_port", CONFFILE_OPTION_INT, -1, conffile_int,
       1, 0, &updown_server_port_flag, &(conf.updown_server_port), 0},
      {"clusterlist_module", CONFFILE_OPTION_STRING, -1, _cb_stringptr,
       1, 0, &clusterlist_module_flag, &(conf.clusterlist_module), 0},
      {"clusterlist_module_options", CONFFILE_OPTION_LIST_STRING, -1, 
       _cb_module_options, 1, 0, &clusterlist_module_options_flag, 
       &(conf.clusterlist_module_options), 0},
      {"speak_debug", CONFFILE_OPTION_BOOL, -1, conffile_bool,
       1, 0, &speak_debug_flag, &conf.speak_debug, 0},
      {"listen_debug", CONFFILE_OPTION_BOOL, -1, conffile_bool,
       1, 0, &listen_debug_flag, &conf.listen_debug, 0},
      {"updown_server_debug", CONFFILE_OPTION_BOOL, -1, conffile_bool,
       1, 0, &updown_server_debug_flag, &conf.updown_server_debug, 0},
    };
  conffile_t cf = NULL;
  int num;

  if ((cf = conffile_handle_create()) == NULL) 
    err_exit("conffile_handle_create: failed to create handle");

  num = sizeof(options)/sizeof(struct conffile_option);
  if (conffile_parse(cf, conf.config_file, options, num, NULL, 0, 0) < 0)
    {
      /* Its not an error if the file doesn't exist */
      if (conffile_errnum(cf) != CONFFILE_ERR_EXIST)
        {
          char buf[CONFFILE_MAX_ERRMSGLEN];
          if (conffile_errmsg(cf, buf, CONFFILE_MAX_ERRMSGLEN) < 0)
            err_exit("conffile_parse: errnum = %d", conffile_errnum(cf));
          err_exit("config file error: %s", buf);
        }
    }

  conffile_handle_destroy(cf);
}

/*
 * _cerebrod_pre_calculate_configuration_config_check
 * 
 * Check configuration settings for errors before analyzing
 * configuration data.
 */
static void
_cerebrod_pre_calculate_configuration_config_check(void)
{
  struct in_addr addr_temp;
  
  if (!Inet_pton(AF_INET, conf.heartbeat_destination_ip, &addr_temp))
    err_exit("heartbeat destination IP address '%s' improperly format",
	     conf.heartbeat_destination_ip);

  if (conf.heartbeat_network_interface
      && strchr(conf.heartbeat_network_interface, '.'))
    {
      if (strchr(conf.heartbeat_network_interface, '/'))
	{
	  char *heartbeat_network_interface_cpy = Strdup(conf.heartbeat_network_interface);
	  char *tok;
	  
	  tok = strtok(heartbeat_network_interface_cpy, "/");
	  if (!Inet_pton(AF_INET, tok, &addr_temp))
	    err_exit("network interface IP address '%s' "
		     "improperly format", conf.heartbeat_network_interface);
	  
	  Free(heartbeat_network_interface_cpy);
	}
      else
	{
	  if (!Inet_pton(AF_INET, conf.heartbeat_network_interface, &addr_temp))
	    err_exit("network interface IP address '%s' "
		     "improperly format", conf.heartbeat_network_interface);
	}
    }

  if (conf.heartbeat_destination_port == conf.heartbeat_source_port)
    err_exit("heartbeat destination and source ports '%d' cannot be identical",
	     conf.heartbeat_destination_port);

  if (conf.heartbeat_ttl <= 0)
    err_exit("heartbeat ttl '%d' invalid", conf.heartbeat_ttl);

  if (conf.listen_threads <= 0)
    err_exit("listen threads '%d' invalid", conf.listen_threads);

  /* If the listening server is turned off, none of the other
   * servers can be on.  So we turn them off.
   */
  if (!conf.listen)
    conf.updown_server = 0;

  if (conf.updown_server)
    {
      if (conf.updown_server_port == conf.heartbeat_destination_port)
	err_exit("updown server port '%d' cannot be identical to heartbeat destination port");
      if (conf.updown_server_port == conf.heartbeat_source_port)
	err_exit("updown server port '%d' cannot be identical to heartbeat source port");
    }
}

/*
 * _cerebrod_calculate_multicast
 * 
 * Determine if the heartbeat_destination_ip is a multicast address
 */
static void
_cerebrod_calculate_multicast(void)
{
  if (strchr(conf.heartbeat_destination_ip, '.'))
    {
      char *heartbeat_destination_ip_cpy = Strdup(conf.heartbeat_destination_ip);
      char *tok, *ptr;
      int ip_class;
      
      tok = strtok(heartbeat_destination_ip_cpy, ".");
      ip_class = strtol(tok, &ptr, 10);
      if (ptr != (tok + strlen(tok)))
	err_exit("heartbeat destination IP address '%s' improperly format",
		 conf.heartbeat_destination_ip);
      
      if (ip_class >= MULTICAST_CLASS_MIN
	  && ip_class <= MULTICAST_CLASS_MAX)
	conf.multicast = 1;
      else
	conf.multicast = 0;
      Free(heartbeat_destination_ip_cpy);
    }
}

/*
 * _cerebrod_calculate_heartbeat_frequency_ranged
 * 
 * Determine if the heartbeat frequencey is static or ranged
 */
static void
_cerebrod_calculate_heartbeat_frequency_ranged(void)
{
  if (conf.heartbeat_frequency_max > 0)
    conf.heartbeat_frequency_ranged = 1;
  else
    conf.heartbeat_frequency_ranged = 0;
}

/*
 * _cerebrod_calculate_heartbeat_destination_ip_in_addr
 * 
 * Convert the destination ip address from presentable to network order
 */
static void
_cerebrod_calculate_heartbeat_destination_ip_in_addr(void)
{
  if (!Inet_pton(AF_INET, conf.heartbeat_destination_ip, &conf.heartbeat_destination_ip_in_addr))
    err_exit("heartbeat destination IP address '%s' improperly format",
	     conf.heartbeat_destination_ip);
}

/*
 * _get_if_conf
 *
 * Retrieve network interface configuration.  Code is mostly from Unix
 * Network Programming by R. Stevens, Chapter 16
 * 
 * Return buffer of local network interface configurations.
 */
static void 
_get_if_conf(void **buf, struct ifconf *ifc, int fd)
{
  int lastlen = -1, len = sizeof(struct ifreq) * 100;

  assert(buf && ifc);

  for(;;)
    {
      *buf = Malloc(len);
      ifc->ifc_len = len;
      ifc->ifc_buf = *buf;

      if (ioctl(fd, SIOCGIFCONF, ifc) < 0)
        err_exit("_get_if_conf: ioctl: %s", strerror(errno));

      if (ifc->ifc_len == lastlen)
        break;

      lastlen = ifc->ifc_len;
      len += 10 * sizeof(struct ifreq);
      Free(*buf);
    }
}

/*
 * _get_ifr_len
 *
 * Determine the length of an ifreq structure.  Code is mostly from
 * Unix Network Programming by R. Stevens, Chapter 16
 *
 * Return calculated length of the ifreq structure
 */
static int
_get_ifr_len(struct ifreq *ifr)
{
  int len;

  assert(ifr);

#if HAVE_SA_LEN
  if (sizeof(struct sockaddr) > ifr->ifr_addr.sa_len)
    len = sizeof(struct sockaddr);
  else
    len = ifr->ifr_addr.sa_len;
#else /* !HAVE_SA_LEN */
  /* For now we only assume AF_INET and AF_INET6 */
  switch(ifr->ifr_addr.sa_family) {
#ifdef HAVE_IPV6
  case AF_INET6:
    len = sizeof(struct sockaddr_in6);
    break;
#endif /* HAVE_IPV6 */
  case AF_INET:
  default:
    len = sizeof(struct sockaddr_in);
    break;
  }

  if (len < (sizeof(struct ifreq) - IFNAMSIZ))
    len = sizeof(struct ifreq) - IFNAMSIZ;
#endif /* HAVE_SA_LEN */

  return len;
}

/*
 * _cerebrod_calculate_in_addr_and_index
 *
 * Calculate a network interface ip address and interface index
 * based on the network interface passed in.
 *
 * Return calculated ip address and interface index.
 */
static void
_cerebrod_calculate_in_addr_and_index(char *network_interface,
				      struct in_addr *interface_in_addr,
				      int *interface_index)
{
  assert(interface_in_addr && interface_index);
  
  if (!network_interface)
    {
      /* Case A: No interface specified
       * - If multicast heartbeat IP address specified, find a multicast interface
       * - If singlecast heartbeat IP address specified, use INADDR_ANY
       */
      if (conf.multicast)
	{
	  struct ifconf ifc;
	  struct ifreq *ifr;
	  void *buf = NULL, *ptr = NULL;
	  int fd, found_interface = 0;;
                                                                    
	  /* From Unix Network Programming, by R. Stevens, Chapter 16 */

	  fd = Socket(AF_INET, SOCK_DGRAM, 0);

	  _get_if_conf(&buf, &ifc, fd);

	  /* Check all interfaces */
	  for (ptr = buf; ptr < buf + ifc.ifc_len; ) 
	    {
	      struct ifreq ifr_tmp;
	      struct sockaddr_in *sinptr;
	      int len;

	      ifr = (struct ifreq *)ptr;

	      len = _get_ifr_len(ifr);

	      ptr += sizeof(ifr->ifr_name) + len;
          
	      /* Null termination not required, don't use strncpy() wrapper */
	      memset(&ifr_tmp, '\0', sizeof(struct ifreq));
	      strncpy(ifr_tmp.ifr_name, ifr->ifr_name, IFNAMSIZ);

	      if(ioctl(fd, SIOCGIFFLAGS, &ifr_tmp) < 0)
		err_exit("cerebrod_calculate_configuration: ioctl: %s",
			 strerror(errno));

	      if (!(ifr_tmp.ifr_flags & IFF_UP)
		  || !(ifr_tmp.ifr_flags & IFF_MULTICAST))
		continue;

	      /* Null termination not required, don't use strncpy() wrapper */
	      memset(&ifr_tmp, '\0', sizeof(struct ifreq));
	      strncpy(ifr_tmp.ifr_name, ifr->ifr_name, IFNAMSIZ);

	      if(ioctl(fd, SIOCGIFINDEX, &ifr_tmp) < 0)
		err_exit("cerebrod_calculate_configuration: ioctl: %s",
			 strerror(errno));
	      
	      sinptr = (struct sockaddr_in *)&ifr->ifr_addr;
	      interface_in_addr->s_addr = sinptr->sin_addr.s_addr;
	      *interface_index = ifr_tmp.ifr_ifindex;
	      found_interface++;
	      break;
	    }

	  if (!found_interface)
	    err_exit("network interface with multicast not found", 
		     network_interface);

	  Free(buf);
	  Close(fd);
	}
      else
	interface_in_addr->s_addr = INADDR_ANY;
    }
  else if (strchr(network_interface, '.'))
    {
      /* Case B: IP address or subnet specified */
      struct ifconf ifc;
      struct ifreq *ifr;
      void *buf = NULL, *ptr = NULL;
      int fd, mask = 0, found_interface = 0;
      struct in_addr addr_temp;
      u_int32_t conf_masked_ip;

      fd = Socket(AF_INET, SOCK_DGRAM, 0);

      _get_if_conf(&buf, &ifc, fd);
     
      if (strchr(network_interface, '/'))
	{
	  char *network_interface_cpy = Strdup(network_interface);
	  char *snm, *ptr, *tok;

	  tok = strtok(network_interface_cpy, "/");
	  if (!Inet_pton(AF_INET, tok, &addr_temp))
            err_exit("network interface IP address '%s' "
                     "improperly format", network_interface);

	  if (!(snm = strtok(NULL, "")))
	    err_exit("network interface '%s' subnet mask not specified",
		     network_interface);
	  else
	    {
	      mask = strtol(snm, &ptr, 10);
	      if (ptr != (snm + strlen(snm)))
		err_exit("network interface '%s' subnet mask improper",
			 network_interface);
	      else if (mask < 1 || mask > IPADDR_BITS)
		err_exit("network interface '%s' subnet mask size invalid",
			 network_interface);
	    }

	  Free(network_interface_cpy);
	}
      else
	{
	  /* If no '/', then just an IP address, mask is all bits */
	  if (!Inet_pton(AF_INET, network_interface, &addr_temp))
            err_exit("network interface IP address '%s' "
                     "improperly format", network_interface);
	  mask = IPADDR_BITS;
	}

      conf_masked_ip = htonl(addr_temp.s_addr) >> (IPADDR_BITS - mask);

      /* Check all interfaces */
      for(ptr = buf; ptr < buf + ifc.ifc_len;)
	{ 
	  struct ifreq ifr_tmp;
	  struct sockaddr_in *sinptr;
	  u_int32_t intf_masked_ip;
	  int len;

	  ifr = (struct ifreq *)ptr;

	  len = _get_ifr_len(ifr);

	  ptr += sizeof(ifr->ifr_name) + len;
          
	  sinptr = (struct sockaddr_in *)&ifr->ifr_addr;
          
	  intf_masked_ip = htonl((sinptr->sin_addr).s_addr) >> (32 - mask);
          
	  /* Null termination not required, don't use wrapper */
	  memset(&ifr_tmp, '\0', sizeof(struct ifreq));
	  strncpy(ifr_tmp.ifr_name, ifr->ifr_name, IFNAMSIZ);

	  if (ioctl(fd, SIOCGIFFLAGS, &ifr_tmp) < 0)
	    err_exit("cerebrod_calculate_configuration: ioctl: %s",
		     strerror(errno));
          
	  if (!(ifr_tmp.ifr_flags & IFF_UP))
	    continue;

	  if (conf.multicast && !(ifr_tmp.ifr_flags & IFF_MULTICAST))
	    continue;
          
	  if (conf_masked_ip == intf_masked_ip)
	    {
	      /* Null termination not required, don't use strncpy() wrapper */
	      memset(&ifr_tmp, '\0', sizeof(struct ifreq));
	      strncpy(ifr_tmp.ifr_name, ifr->ifr_name, IFNAMSIZ);

	      if(ioctl(fd, SIOCGIFINDEX, &ifr_tmp) < 0)
		err_exit("cerebrod_calculate_configuration: ioctl: %s",
			 strerror(errno));

	      interface_in_addr->s_addr = sinptr->sin_addr.s_addr;
	      *interface_index = ifr_tmp.ifr_ifindex;
	      found_interface++;
	      break;
	    }
	}
      
      if (!found_interface)
	{
	  if (conf.multicast)
	    err_exit("network interface '%s' with multicast "
		     "not found", network_interface);
	  else
	    err_exit("network interface '%s' not found", network_interface);
	}
      
      Free(buf);
      Close(fd);
    }
  else
    {
      /* Case C: Interface name specified */
      struct ifconf ifc;
      struct ifreq *ifr;
      void *buf = NULL, *ptr = NULL;
      int fd, found_interface = 0;
                                                                    
      /* From Unix Network Programming, by R. Stevens, Chapter 16 */

      fd = Socket(AF_INET, SOCK_DGRAM, 0);

      _get_if_conf(&buf, &ifc, fd);

      /* Check all interfaces */
      for (ptr = buf; ptr < buf + ifc.ifc_len; ) 
	{
	  int len;

	  ifr = (struct ifreq *)ptr;

	  len = _get_ifr_len(ifr);

	  ptr += sizeof(ifr->ifr_name) + len;
          
	  if (!strcmp(network_interface, ifr->ifr_name))
	    {
	      struct sockaddr_in *sinptr;
	      struct ifreq ifr_tmp;

	      /* Null termination not required, don't use wrapper */
	      memset(&ifr_tmp, '\0', sizeof(struct ifreq));
	      strncpy(ifr_tmp.ifr_name, ifr->ifr_name, IFNAMSIZ);

	      if(ioctl(fd, SIOCGIFFLAGS, &ifr_tmp) < 0)
		err_exit("cerebrod_calculate_configuration: ioctl: %s",
			 strerror(errno));

	      if (!(ifr_tmp.ifr_flags & IFF_UP))
		err_exit("network interface '%s' not up",
			 network_interface);
	      
	      if (conf.multicast && !(ifr_tmp.ifr_flags & IFF_MULTICAST))
		err_exit("network interface '%s' not a multicast interface",
			 network_interface);

	      /* Null termination not required, don't use strncpy() wrapper */
	      memset(&ifr_tmp, '\0', sizeof(struct ifreq));
	      strncpy(ifr_tmp.ifr_name, ifr->ifr_name, IFNAMSIZ);

	      if(ioctl(fd, SIOCGIFINDEX, &ifr_tmp) < 0)
		err_exit("cerebrod_calculate_configuration: ioctl: %s",
			 strerror(errno));

	      sinptr = (struct sockaddr_in *)&ifr->ifr_addr;
	      interface_in_addr->s_addr = sinptr->sin_addr.s_addr;
	      *interface_index = ifr_tmp.ifr_ifindex;
	      found_interface++;
	      break;
	    }
	}

      if (!found_interface)
	err_exit("network interface '%s' not found",
		 network_interface);

      Free(buf);
      Close(fd);
    }
}

/*
 * _cerebrod_calculate_heartbeat_network_interface_in_addr_and_index
 *
 * Calculate the heartbeat ip address and interface index based on the
 * network interface passed in the configuration file.
 */
static void
_cerebrod_calculate_heartbeat_network_interface_in_addr_and_index(void)
{
  _cerebrod_calculate_in_addr_and_index(conf.heartbeat_network_interface,
					&conf.heartbeat_network_interface_in_addr,
					&conf.heartbeat_interface_index);
}

/*
 * _cerebrod_calculate_clusterlist_module
 *
 * Search for and determine the full path to the clusterlist module we
 * wish to use
 */
static void
_cerebrod_calculate_clusterlist_module(void)
{
  if (conf.clusterlist_module)
    {
      struct stat buf;

      /* Don't use stat() wrapper, we want to receive the error */

      if (conf.clusterlist_module[0] == '/')
	{
	  /* Case A: User specified absolute path to clusterlist module */
	  if (stat(conf.clusterlist_module, &buf) < 0)
	    err_exit("clusterlist module '%s' not found", 
		     conf.clusterlist_module);
	  
	  conf.clusterlist_module_file = Strdup(conf.clusterlist_module);
	}
      else
	{
	  /* Case B: Search for clusterlist module */
	  char filebuf[MAXPATHLEN+1];

	  /* Assume user passed in clusterlist filename.  Search in
	   * module directory.
	   */
	  memset(filebuf, '\0', MAXPATHLEN+1);
	  snprintf(filebuf, MAXPATHLEN, "%s/%s", 
		   CEREBROD_MODULE_DIR, conf.clusterlist_module);

	  if (!stat(filebuf, &buf))
	    {
	      conf.clusterlist_module_file = Strdup(filebuf);
	      return;
	    }

	  /* Assume user passed in clusterlist filename.  Search in
	   * current directory.
	   */
	  memset(filebuf, '\0', MAXPATHLEN+1);
	  snprintf(filebuf, MAXPATHLEN, "./%s", conf.clusterlist_module);

	  if (!stat(filebuf, &buf))
	    {
	      conf.clusterlist_module_file = Strdup(filebuf);
	      return;
	    }

	  /* Assume user passed in clusterlist module name.  Search in
	   * module directory.
	   */
	  memset(filebuf, '\0', MAXPATHLEN+1);
	  snprintf(filebuf, MAXPATHLEN, "%s/cerebrod_clusterlist_%s.la", 
		   CEREBROD_MODULE_DIR, conf.clusterlist_module);

	  if (!stat(filebuf, &buf))
	    {
	      conf.clusterlist_module_file = Strdup(filebuf);
	      return;
	    }

	  /* Assume user passed in clusterlist module name.  Search in
	   * current directory.
	   */
	  memset(filebuf, '\0', MAXPATHLEN+1);
	  snprintf(filebuf, MAXPATHLEN, "./cerebrod_clusterlist_%s.la", conf.clusterlist_module);

	  if (!stat(filebuf, &buf))
	    {
	      conf.clusterlist_module_file = Strdup(filebuf);
	      return;
	    }

	  if (!conf.clusterlist_module_file)
	    err_exit("clusterlist module '%s' not found", conf.clusterlist_module);
	}
    }
}

/*
 * _cerebrod_calculate_configuration
 *
 * analyze and calculate configuration based on settings
 */
static void
_cerebrod_calculate_configuration(void)
{
  /* Determine if the heartbeat is single or multi casted */
  _cerebrod_calculate_multicast();

  /* Determine if the heartbeat frequencey is ranged or fixed */
  _cerebrod_calculate_heartbeat_frequency_ranged();

  /* Calculate the destination ip */
  _cerebrod_calculate_heartbeat_destination_ip_in_addr();

  /* Determine the appropriate network interface to use based on
   * the user's heartbeat_network_interface input.
   */
  _cerebrod_calculate_heartbeat_network_interface_in_addr_and_index();

  /* Determine the clusterlist module to use */
  _cerebrod_calculate_clusterlist_module();
}

/*
 * _cerebrod_post_calculate_configuration_config_check
 * 
 * Check configuration settings for errors after configuration data
 * has been analyzed.
 */
static void
_cerebrod_post_calculate_configuration_config_check(void)
{
  if (!conf.multicast && conf.listen)
    {
      struct ifconf ifc;
      struct ifreq *ifr;
      void *buf = NULL, *ptr = NULL;
      int fd, found_interface = 0;
      struct in_addr addr_temp;

      fd = Socket(AF_INET, SOCK_DGRAM, 0);
      _get_if_conf(&buf, &ifc, fd);
     
      /* If no '/', then just an IP address, mask is all bits */
      if (!Inet_pton(AF_INET, conf.heartbeat_destination_ip, &addr_temp))
        err_exit("heartbeat destination IP address '%s' "
                 "improperly format", conf.heartbeat_destination_ip);

      /* Check all interfaces */
      for(ptr = buf; ptr < buf + ifc.ifc_len;)
	{ 
	  struct sockaddr_in *sinptr;
	  int len;

	  ifr = (struct ifreq *)ptr;

	  len = _get_ifr_len(ifr);

	  ptr += sizeof(ifr->ifr_name) + len;
          
	  sinptr = (struct sockaddr_in *)&ifr->ifr_addr;
          
          if (!memcmp((void *)&addr_temp, 
                      (void *)&sinptr->sin_addr, 
                      sizeof(struct in_addr)))
            {
	      found_interface++;
	      break;
            }
	}
      
      Free(buf);
      Close(fd);

      if (!found_interface)
        err_exit("heartbeat destination address not found");
    }
}

/* 
 * _cerebrod_config_dump
 * 
 * Dump configuration data
 */
static void 
_cerebrod_config_dump(void)
{
#ifndef NDEBUG
  if (conf.debug)
    {
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Cerebrod Configuration:\n");     
      fprintf(stderr, "* -------------------------------\n");
      fprintf(stderr, "* Command Line Options\n");
      fprintf(stderr, "* -------------------------------\n");
      fprintf(stderr, "* debug: %d\n", conf.debug);
      fprintf(stderr, "* config_file: \"%s\"\n", conf.config_file);
      fprintf(stderr, "* config_module: \"%s\"\n", conf.config_module);
      fprintf(stderr, "* -------------------------------\n");
      fprintf(stderr, "* Configuration File Options\n");
      fprintf(stderr, "* -------------------------------\n");
      fprintf(stderr, "* heartbeat_frequency_min: %d\n", conf.heartbeat_frequency_min);
      fprintf(stderr, "* heartbeat_frequency_max: %d\n", conf.heartbeat_frequency_max);
      fprintf(stderr, "* heartbeat_source_port: %d\n", conf.heartbeat_source_port);
      fprintf(stderr, "* heartbeat_destination_port: %d\n", conf.heartbeat_destination_port);
      fprintf(stderr, "* heartbeat_destination_ip: \"%s\"\n", conf.heartbeat_destination_ip);
      fprintf(stderr, "* heartbeat_network_interface: \"%s\"\n", conf.heartbeat_network_interface);
      fprintf(stderr, "* heartbeat_ttl: %d\n", conf.heartbeat_ttl);
      fprintf(stderr, "* speak: %d\n", conf.speak);
      fprintf(stderr, "* listen: %d\n", conf.listen);
      fprintf(stderr, "* listen_threads: %d\n", conf.listen_threads);
      fprintf(stderr, "* updown_server: %d\n", conf.updown_server);
      fprintf(stderr, "* updown_server_port: %d\n", conf.updown_server_port);
      fprintf(stderr, "* clusterlist_module: %s\n", conf.clusterlist_module);
      fprintf(stderr, "* clusterlist_module_options: ");
      if (!conf.clusterlist_module_options)
        fprintf(stderr, "%s\n", (char *)conf.clusterlist_module_options);
      else
        {
          int i = 0;
          while (conf.clusterlist_module_options[i] != NULL) 
            {
              fprintf(stderr, "%s ", conf.clusterlist_module_options[i]);
              i++;
            }
          fprintf(stderr, "\n");
        }
      fprintf(stderr, "* speak_debug: %d\n", conf.speak_debug);
      fprintf(stderr, "* listen_debug: %d\n", conf.listen_debug);
      fprintf(stderr, "* updown_server_debug: %d\n", conf.updown_server_debug);
      fprintf(stderr, "* -------------------------------\n");
      fprintf(stderr, "* Calculated Configuration\n");
      fprintf(stderr, "* -------------------------------\n");
      fprintf(stderr, "* multicast: %d\n", conf.multicast);
      fprintf(stderr, "* heartbeat_destination_ip_in_addr: %s\n", inet_ntoa(conf.heartbeat_destination_ip_in_addr));
      fprintf(stderr, "* heartbeat_network_interface_in_addr: %s\n", inet_ntoa(conf.heartbeat_network_interface_in_addr));
      fprintf(stderr, "* heartbeat_interface_index: %d\n", conf.heartbeat_interface_index);
      fprintf(stderr, "* clusterlist_module_file: %s\n", conf.clusterlist_module_file);
      fprintf(stderr, "**************************************\n");
    }
#endif /* NDEBUG */
}

/*
 * cerebrod_config
 * 
 * perform all cerebrod configuration.  Includes command line parsing,
 * config module loading, and configuration file parsing
 */
void
cerebrod_config(int argc, char **argv)
{
  assert(argv);

  _cerebrod_config_default();
  _cerebrod_cmdline_parse(argc, argv);
  _cerebrod_cmdline_parse_check();
  _cerebrod_config_module_setup();
  _cerebrod_config_parse();
  _cerebrod_pre_calculate_configuration_config_check();
  _cerebrod_calculate_configuration();
  _cerebrod_post_calculate_configuration_config_check();
  _cerebrod_config_dump();
}
