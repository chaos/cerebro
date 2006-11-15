/*****************************************************************************\
 *  $Id: cerebrod_config.c,v 1.129 2006-11-15 00:12:30 chu11 Exp $
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

#include "cerebro/cerebro_config.h"
#include "cerebro/cerebro_config_module.h"
#include "cerebro/cerebro_error.h"

#include "cerebrod_config.h"
#include "cerebrod_util.h"

#include "config_util.h"
#include "debug.h"
#include "wrappers.h"

#define MULTICAST_CLASS_MIN 224
#define MULTICAST_CLASS_MAX 239
#define IPADDR_BITS          32   
#define IPADDR6_BITS        128

#define INTF_NONE             0
#define INTF_IP               1 
#define INTF_NAME             2

#if CEREBRO_DEBUG
extern char *config_debug_config_file;
extern int config_debug_output;
#endif /* CEREBRO_DEBUG */

/* 
 * conf
 *
 * cerebrod configuration used by all of cerebrod
 */
struct cerebrod_config conf;

/*
 * _cerebrod_set_config_default
 *
 * initialize conf structure with default values.
 */
static void
_cerebrod_set_config_default(void)
{
  int i;

  memset(&conf, '\0', sizeof(struct cerebrod_config));

#if CEREBRO_DEBUG
  conf.debug = CEREBROD_DEBUG_DEFAULT;
  conf.config_file = CEREBRO_CONFIG_FILE_DEFAULT;
#endif /* CEREBRO_DEBUG */
  conf.heartbeat_frequency_min = CEREBROD_HEARTBEAT_FREQUENCY_MIN_DEFAULT;
  conf.heartbeat_frequency_max = CEREBROD_HEARTBEAT_FREQUENCY_MAX_DEFAULT;
  conf.message_source_port = CEREBROD_MESSAGE_SOURCE_PORT_DEFAULT;
  conf.message_source_network_interface = CEREBROD_MESSAGE_SOURCE_NETWORK_INTERFACE_DEFAULT;
  conf.message_destination_port = CEREBROD_MESSAGE_DESTINATION_PORT_DEFAULT;
  conf.message_destination_ip = CEREBROD_MESSAGE_DESTINATION_IP_DEFAULT;
  conf.message_ttl = CEREBROD_MESSAGE_TTL_DEFAULT;
  conf.speak = CEREBROD_SPEAK_DEFAULT;
  conf.listen = CEREBROD_LISTEN_DEFAULT;
  conf.listen_threads = CEREBROD_LISTEN_THREADS_DEFAULT;
  
  for (i = 0; i < CEREBRO_MAX_LISTENERS; i++)
    {
      conf.listen_ports[i] = CEREBROD_LISTEN_PORTS_DEFAULT;
      conf.listen_ips[i] = CEREBROD_LISTEN_IPS_DEFAULT;
      conf.listen_network_interfaces[i] = CEREBROD_LISTEN_NETWORK_INTERFACE_DEFAULT;
    }
  conf.listen_ports_len = 0;
  conf.listen_ips_len = 0;
  conf.listen_network_interfaces_len = 0;

  conf.metric_controller = CEREBROD_METRIC_CONTROLLER_DEFAULT;
  conf.metric_server = CEREBROD_METRIC_SERVER_DEFAULT;
  conf.metric_server_port = CEREBROD_METRIC_SERVER_PORT_DEFAULT;
  conf.event_server = CEREBROD_EVENT_SERVER_DEFAULT;
  conf.event_server_port = CEREBROD_EVENT_SERVER_PORT_DEFAULT;
#if CEREBRO_DEBUG
  conf.speak_debug = CEREBROD_SPEAK_DEBUG_DEFAULT;
  conf.listen_debug = CEREBROD_LISTEN_DEBUG_DEFAULT;
  conf.metric_controller_debug = CEREBROD_METRIC_CONTROLLER_DEBUG_DEFAULT;
  conf.metric_server_debug = CEREBROD_METRIC_SERVER_DEBUG_DEFAULT;
  conf.event_server_debug = CEREBROD_EVENT_SERVER_DEBUG_DEFAULT;
#endif /* CEREBRO_DEBUG */
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
          "-v    --version       Output Version\n");
#if CEREBRO_DEBUG
  fprintf(stderr, 
          "-c    --config_file   Specify alternate config file\n"
          "-d    --debug         Turn on debugging and run daemon\n"
	  "                      in foreground\n");
#endif /* CEREBRO_DEBUG */
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
 * _cerebrod_cmdline_arguments_parse
 *
 * parse command line options
 */
static void
_cerebrod_cmdline_arguments_parse(int argc, char **argv)
{ 
  char options[100];
  int c;

#if HAVE_GETOPT_LONG
  struct option long_options[] =
    {
      {"help",                0, NULL, 'h'},
      {"version",             0, NULL, 'v'},
#if CEREBRO_DEBUG
      {"config-file",         1, NULL, 'c'},
      {"debug",               0, NULL, 'd'},
#endif /* CEREBRO_DEBUG */
    };
#endif /* HAVE_GETOPT_LONG */

  assert(argv);

  memset(options, '\0', sizeof(options));
  strcat(options, "hv");
#if CEREBRO_DEBUG
  strcat(options, "c:d");
#endif /* CEREBRO_DEBUG */

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
#if CEREBRO_DEBUG
        case 'c':       /* --config-file */
          conf.config_file = Strdup(optarg);
          break;
        case 'd':       /* --debug */
          conf.debug++;
          break;
#endif /* CEREBRO_DEBUG */
        case '?':
        default:
          cerebro_err_exit("unknown command line option '%c'", optopt);
        }          
    }
}

/*
 * _cerebrod_cmdline_arguments_error_check
 *
 * check validity of command line arguments
 */
static void
_cerebrod_cmdline_arguments_error_check(void)
{
#if CEREBRO_DEBUG
  /* Check if the configuration file exists */
  if (conf.config_file)
    {
      /* The default config_file is allowed to be missing, so don't
       * bother checking if it exists.
       */
      if (strcmp(conf.config_file, CEREBRO_CONFIG_FILE_DEFAULT) != 0)
        {
          struct stat buf;
          
          if (stat(conf.config_file, &buf) < 0)
            cerebro_err_exit("config file '%s' not found", conf.config_file);
        }
    }
#endif /* CEREBRO_DEBUG */
}

/*
 * _cerebrod_load_config
 *
 * load configuration data
 */
static void
_cerebrod_load_config(void)
{
  struct cerebro_config tconf;
  unsigned int errnum;
  int i;

#if CEREBRO_DEBUG
  config_debug_config_file = conf.config_file;
  config_debug_output = conf.debug;
#endif /* CEREBRO_DEBUG */

  if (load_config(&tconf, &errnum) < 0)
    CEREBRO_EXIT(("load_config: %d", errnum));
  
  if (tconf.cerebrod_heartbeat_frequency_flag)
    {
      conf.heartbeat_frequency_min = tconf.cerebrod_heartbeat_frequency_min;
      conf.heartbeat_frequency_max = tconf.cerebrod_heartbeat_frequency_max;
    }
  if (tconf.cerebrod_message_source_port_flag)
    conf.message_source_port = tconf.cerebrod_message_source_port;
  if (tconf.cerebrod_message_source_network_interface_flag)
    conf.message_source_network_interface = Strdup(tconf.cerebrod_message_source_network_interface);
  if (tconf.cerebrod_message_destination_port_flag)
    conf.message_destination_port = tconf.cerebrod_message_destination_port;
  if (tconf.cerebrod_message_destination_ip_flag)
    conf.message_destination_ip = Strdup(tconf.cerebrod_message_destination_ip);
  if (tconf.cerebrod_message_ttl_flag)
    conf.message_ttl = tconf.cerebrod_message_ttl;
  if (tconf.cerebrod_speak_flag)
    conf.speak = tconf.cerebrod_speak;
  if (tconf.cerebrod_listen_flag)
    conf.listen = tconf.cerebrod_listen;
  if (tconf.cerebrod_listen_threads_flag)
    conf.listen_threads = tconf.cerebrod_listen_threads;
  if (tconf.cerebrod_listen_ports)
    {
      for (i = 0; i < tconf.cerebrod_listen_ports_len; i++)
        conf.listen_ports[i] = tconf.cerebrod_listen_ports[i];
      conf.listen_ports_len = tconf.cerebrod_listen_ports_len;
    }
  if (tconf.cerebrod_listen_ips)
    {
      for (i = 0; i < tconf.cerebrod_listen_ips_len; i++)
        conf.listen_ips[i] = Strdup(tconf.cerebrod_listen_ips[i]);
      conf.listen_ips_len = tconf.cerebrod_listen_ips_len;
    }
  if (tconf.cerebrod_listen_network_interfaces)
    {
      for (i = 0; i < tconf.cerebrod_listen_network_interfaces_len; i++)
        conf.listen_network_interfaces[i] = Strdup(tconf.cerebrod_listen_network_interfaces[i]);
      conf.listen_network_interfaces_len = tconf.cerebrod_listen_network_interfaces_len;
    }
  if (tconf.cerebrod_metric_controller_flag)
    conf.metric_controller = tconf.cerebrod_metric_controller;
  if (tconf.cerebrod_metric_server_flag)
    conf.metric_server = tconf.cerebrod_metric_server;
  if (tconf.cerebrod_metric_server_port_flag)
    conf.metric_server_port = tconf.cerebrod_metric_server_port;
  if (tconf.cerebrod_event_server_flag)
    conf.event_server = tconf.cerebrod_event_server;
  if (tconf.cerebrod_event_server_port_flag)
    conf.event_server_port = tconf.cerebrod_event_server_port;
#if CEREBRO_DEBUG
  if (tconf.cerebrod_speak_debug_flag)
    conf.speak_debug = tconf.cerebrod_speak_debug;
  if (tconf.cerebrod_listen_debug_flag)
    conf.listen_debug = tconf.cerebrod_listen_debug;
  if (tconf.cerebrod_metric_controller_debug_flag)
    conf.metric_controller_debug = tconf.cerebrod_metric_controller_debug;
  if (tconf.cerebrod_metric_server_debug_flag)
    conf.metric_server_debug = tconf.cerebrod_metric_server_debug;
  if (tconf.cerebrod_event_server_debug_flag)
    conf.event_server_debug = tconf.cerebrod_event_server_debug;
#endif /* CEREBRO_DEBUG */
}

/* 
 * _cerebrod_config_error_check_network_interface
 *
 * Check for valid network interface IP address
 */
static void
_cerebrod_config_error_check_network_interface(char *network_interface)
{
  struct in_addr addr_temp;

  if (network_interface && strchr(network_interface, '.'))
    {
      if (strchr(network_interface, '/'))
	{
	  char *ipaddr_cpy = Strdup(network_interface);
	  char *tok;
          
	  tok = strtok(ipaddr_cpy, "/");
	  if (!Inet_pton(AF_INET, tok, &addr_temp))
	    cerebro_err_exit("network interface IP address '%s' invalid", 
                             network_interface);
	  
	  Free(ipaddr_cpy);
	}
      else
	{
	  if (!Inet_pton(AF_INET, network_interface, &addr_temp))
	    cerebro_err_exit("network interface IP address '%s' invalid",
                             network_interface);
	}
    }
}

/*
 * _cerebrod_config_error_check
 *
 * Check configuration for errors
 */
static void
_cerebrod_config_error_check(void)
{
  struct in_addr addr_temp;
  int i;
  
  if (conf.heartbeat_frequency_min <= 0)
    cerebro_err_exit("heartbeat frequency min '%d' invalid", 
                     conf.heartbeat_frequency_min);

  if (conf.heartbeat_frequency_max <= 0)
    cerebro_err_exit("heartbeat frequency max '%d' invalid", 
                     conf.heartbeat_frequency_max);
  
  if (conf.message_source_port <= 0)
    cerebro_err_exit("message source port '%d' invalid", 
                     conf.message_source_port);

  _cerebrod_config_error_check_network_interface(conf.message_source_network_interface);

  if (conf.message_destination_port <= 0)
    cerebro_err_exit("message destination port '%d' invalid", 
                     conf.message_destination_port); 

  if (conf.message_destination_port == conf.message_source_port)
    cerebro_err_exit("message destination and source ports '%d' identical",
		      conf.message_destination_port);

  if (!Inet_pton(AF_INET, conf.message_destination_ip, &addr_temp))
    cerebro_err_exit("message destination IP address '%s' invalid",
                     conf.message_destination_ip);

  if (conf.message_ttl <= 0)
    cerebro_err_exit("message ttl '%d' invalid", conf.message_ttl);

  if (conf.listen_threads <= 0)
    cerebro_err_exit("listen threads '%d' invalid", conf.listen_threads);

  for (i = 0; i < conf.listen_ports_len; i++)
    {
      if (conf.listen_ports[i] <= 0)
        cerebro_err_exit("listen port '%d' invalid", conf.listen_ports[i]);

      if (conf.message_source_port == conf.listen_ports[i])
        cerebro_err_exit("message source port and listen port '%d' identical",
                         conf.message_source_port);
    }

  if (conf.listen_ips_len)
    {
      for (i = 0; i < conf.listen_ips_len; i++)
        {
          if (!Inet_pton(AF_INET, conf.listen_ips[i], &addr_temp))
            cerebro_err_exit("listen IP address '%s' invalid",
                             conf.message_destination_ip);
        }
    }

  if (conf.listen_network_interfaces_len)
    {
      for (i = 0; i < conf.listen_network_interfaces_len; i++)
        _cerebrod_config_error_check_network_interface(conf.listen_network_interfaces[i]);
    }

  if (conf.metric_server_port <= 0)
    cerebro_err_exit("metric server port '%d' invalid", conf.metric_server_port);

  if (conf.event_server_port <= 0)
    cerebro_err_exit("event server port '%d' invalid", conf.event_server_port);
}

/* 
 * achu: A significant portion of the network code below is from from
 * Unix Network Programming, by R. Stevens, Chapter 16 
 */

/*
 * _get_if_conf
 *
 * Retrieve network interface configuration.
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
        CEREBRO_EXIT(("ioctl: %s", strerror(errno)));

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
 * Determine the length of an ifreq structure.
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
 * _calculate_in_addr_and_index
 *
 * Calculate a network interface ip address and interface index
 * based on the network interface passed in.
 *
 * Return calculated ip address and interface index.
 */
static void
_calculate_in_addr_and_index(int is_multicast,
                             char *intf, 
                             struct in_addr *in_addr, 
                             int *index)
{
  void *buf = NULL, *ptr = NULL;
  int fd, mask = 0, found_interface = 0;
  u_int32_t conf_masked_ip = 0;
  struct ifconf ifc;
  int intf_type;

  /* 
   * There are three possibile formats for the interface input:
   *
   * - No interface specified (i.e. null)
   * - IP address or subnet specified 
   * - Interface name specified 
   */

  if (!intf)
    intf_type = INTF_NONE;
  else if (strchr(intf, '.'))
    {
      struct in_addr addr_temp;

      if (strchr(intf, '/'))
        {
          char *intf_cpy = Strdup(intf);
          char *s_mask, *ptr, *tok;
          
          tok = strtok(intf_cpy, "/");
          if (!Inet_pton(AF_INET, tok, &addr_temp))
            cerebro_err_exit("network interface IP '%s' invalid", intf);
          
          if (!(s_mask = strtok(NULL, "")))
            cerebro_err_exit("subnet mask not specified in '%s'", intf);
          else
            {
              mask = strtol(s_mask, &ptr, 10);
              if ((ptr != (s_mask + strlen(s_mask))) 
                  || (mask < 1 || mask > IPADDR_BITS))
                cerebro_err_exit("subnet mask in '%s' invalid", intf);
            }
          
          Free(intf_cpy);
        }
      else
        {
          /* If no '/', then just an IP address, mask is all bits */
          if (!Inet_pton(AF_INET, intf, &addr_temp))
            cerebro_err_exit("network interface IP '%s' invalid", intf);
          mask = IPADDR_BITS;
        }
      
      conf_masked_ip = htonl(addr_temp.s_addr) >> (IPADDR_BITS - mask);
      
      intf_type = INTF_IP;
    }
  else
    intf_type = INTF_NAME;

  assert(in_addr && index);
  
  /* Special Case */
  if (intf_type == INTF_NONE && !is_multicast)
    {
      in_addr->s_addr = INADDR_ANY;
      return;
    }

  fd = Socket(AF_INET, SOCK_DGRAM, 0);

  _get_if_conf(&buf, &ifc, fd);

  for (ptr = buf; ptr < buf + ifc.ifc_len; ) 
    {
      struct sockaddr_in *sinptr;
      struct ifreq ifr_tmp;
      struct ifreq *ifr;
      int len;
          
      ifr = (struct ifreq *)ptr;
      
      len = _get_ifr_len(ifr);
      
      ptr += sizeof(ifr->ifr_name) + len;
      
      /* Don't bother continuing if this isn't the right interface */
      if (intf_type == INTF_NAME)
        {
          char ifr_name_buf[IFNAMSIZ+1];
          
          /* Guarantee NUL termination */
          memset(ifr_name_buf, '\0', IFNAMSIZ+1);
          memcpy(ifr_name_buf, ifr->ifr_name, IFNAMSIZ);

          if (strcmp(intf, ifr_name_buf))
            continue;
        }

      /* Don't bother continuing if the IP doesn't match */
      if (intf_type == INTF_IP)
        {
          u_int32_t intf_masked_ip;

          sinptr = (struct sockaddr_in *)&ifr->ifr_addr;
          intf_masked_ip = htonl((sinptr->sin_addr).s_addr) >> (32 - mask);

          if (intf_type == INTF_IP && conf_masked_ip != intf_masked_ip)
            continue;
        }

      /* NUL termination not required, don't use strncpy() wrapper */
      memset(&ifr_tmp, '\0', sizeof(struct ifreq));
      strncpy(ifr_tmp.ifr_name, ifr->ifr_name, IFNAMSIZ);

      if (ioctl(fd, SIOCGIFFLAGS, &ifr_tmp) < 0)
        CEREBRO_EXIT(("ioctl: %s", strerror(errno)));

      if (!(ifr_tmp.ifr_flags & IFF_UP))
        continue;

      if (is_multicast && !(ifr_tmp.ifr_flags & IFF_MULTICAST))
        continue;

      /* NUL termination not required, don't use strncpy() wrapper */
      memset(&ifr_tmp, '\0', sizeof(struct ifreq));
      strncpy(ifr_tmp.ifr_name, ifr->ifr_name, IFNAMSIZ);
      
      if(ioctl(fd, SIOCGIFINDEX, &ifr_tmp) < 0)
        CEREBRO_EXIT(("ioctl: %s", strerror(errno)));
          
      sinptr = (struct sockaddr_in *)&ifr->ifr_addr;
      in_addr->s_addr = sinptr->sin_addr.s_addr;
      *index = ifr_tmp.ifr_ifindex;
      found_interface++;
      break;
    }
  
  if (!found_interface)
    {
      if (is_multicast)
        {
          if (intf_type == INTF_NONE)
            cerebro_err_exit("multicast network interface not found");
          cerebro_err_exit("multicast network interface '%s' not found", intf);
        }
      else
        cerebro_err_exit("network interface '%s' not found", intf);
    }

  Free(buf);
  Close(fd);
}

/* 
 * _cerebrod_ip_is_multicast
 *
 * Return 1 if the ip address is multicast, 0 if not
 */
static int
_cerebrod_ip_is_multicast(char *ip)
{
  if (strchr(ip, '.'))
    {
      char *ipaddr_cpy = Strdup(ip);
      char *tok, *ptr;
      int ip_class;
      int rv;

      tok = strtok(ipaddr_cpy, ".");
      ip_class = strtol(tok, &ptr, 10);
      if (ptr != (tok + strlen(tok)))
	cerebro_err_exit("IP address '%s' invalid", ip);
      
      if (ip_class >= MULTICAST_CLASS_MIN && ip_class <= MULTICAST_CLASS_MAX)
	rv = 1;
      else
	rv = 0;
      Free(ipaddr_cpy);
      return rv;
    }
  else
    cerebro_err_exit("IP address '%s' invalid", ip);

  /* NOT REACHED */
  return 0;
}

/*
 * _cerebrod_calculate_configuration_data
 *
 * analyze and calculate configuration data based on values input
 */
static void
_cerebrod_calculate_configuration_data(void)
{
  /* Determine if the message destiation is single or multi casted */
  conf.destination_ip_is_multicast = _cerebrod_ip_is_multicast(conf.message_destination_ip);

  /* Determine if the heartbeat frequencey is ranged or fixed */
  if (conf.heartbeat_frequency_max > 0)
    conf.heartbeat_frequency_ranged = 1;
  else
    conf.heartbeat_frequency_ranged = 0;

  /* Determine the appropriate network interface to use based on
   * the user's message_source_network_interface input.
   */
  _calculate_in_addr_and_index(conf.destination_ip_is_multicast,
                               conf.message_source_network_interface,
                               &conf.message_source_network_interface_in_addr,
                               &conf.message_source_network_interface_index);

  /* Calculate the destination ip */
  if (!Inet_pton(AF_INET, 
                 conf.message_destination_ip, 
                 &conf.message_destination_ip_in_addr))
    cerebro_err_exit("message destination IP address '%s' invalid",
                     conf.message_destination_ip);

  if (conf.listen)
    {
      int max_len = 0;
      int i;

      /* Whatever the max index is, we will use */
      if (conf.listen_ports_len > max_len)
        max_len = conf.listen_ports_len;
      if (conf.listen_ips_len > max_len)
        max_len = conf.listen_ips_len;
      if (conf.listen_network_interfaces_len > max_len)
        max_len = conf.listen_network_interfaces_len;

      /* We need atleast one IP/NIC to listen off of. */
      if (!max_len)
        max_len = 1;
      
      conf.listen_len = max_len;

      for (i = 0; i < conf.listen_len; i++)
        {
          /* Determine if the list is single or multi casted */
          conf.listen_ips_is_multicast[i] = _cerebrod_ip_is_multicast(conf.listen_ips[i]);
          
          /* Calculate the ip in_addr */
          if (!Inet_pton(AF_INET, 
                         conf.listen_ips[i], 
                         &conf.listen_ips_in_addr[i]))
            cerebro_err_exit("listen IP address '%s' invalid",
                             conf.listen_ips[i]);

          /* Get the network interface and index for this listener*/
          _calculate_in_addr_and_index(conf.listen_ips_is_multicast[i],
                                       conf.listen_network_interfaces[i],
                                       &conf.listen_network_interfaces_in_addr[i],
                                       &conf.listen_network_interfaces_index[i]);
        }
    }

  /* If the listening server is turned off, the metric or event server
   * must be turned off
   */
  if (!conf.listen)
    {
      conf.metric_server = 0;
      conf.event_server = 0;
    }

  /* Why would the user want the daemon to do nothing?  Oh well*/
  if (!conf.speak && !conf.listen)
    conf.metric_controller = 0;
}

/*
 * _cerebrod_configuration_data_error_check
 *
 * Check configuration data for errors
 */
static void
_cerebrod_configuration_data_error_check(void)
{
  int i;
 
  /* If the desitnation ip address isn't multicast, and we are
   * listening, the destination ip address better be one of our NICs
   */
  if (!conf.destination_ip_is_multicast && conf.listen)
    {
      struct ifconf ifc;
      struct ifreq *ifr;
      void *buf = NULL, *ptr = NULL;
      int fd, found_interface = 0;
      struct in_addr addr_temp;

      fd = Socket(AF_INET, SOCK_DGRAM, 0);
      _get_if_conf(&buf, &ifc, fd);
     
      /* If no '/', then just an IP address, mask is all bits */
      if (!Inet_pton(AF_INET, conf.message_destination_ip, &addr_temp))
        cerebro_err_exit("message destination IP address '%s' "
                         "improperly format", conf.message_destination_ip);

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
        cerebro_err_exit("message destination address not found");
    }

  if (conf.metric_server)
    {
      if (conf.metric_server_port == conf.message_destination_port)
	cerebro_err_exit("metric server port '%d' cannot be identical "
                         "to message destination port", conf.metric_server_port);

      if (conf.metric_server_port == conf.message_source_port)
	cerebro_err_exit("metric server port '%d' cannot be identical "
                         "to message source port", conf.metric_server_port);

      if (conf.metric_server_port == conf.event_server_port)
	cerebro_err_exit("metric server port '%d' cannot be identical "
                         "to event server port", conf.metric_server_port);

      for (i = 0; i < conf.listen_len; i++)
        {
          if (conf.metric_server_port == conf.listen_ports[i])
            cerebro_err_exit("metric server port '%d' cannot be identical "
                             "to a listen port", conf.metric_server_port);
        }
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
#if CEREBRO_DEBUG
  int i;

  if (!conf.debug)
    return;

  fprintf(stderr, "**************************************\n");
  fprintf(stderr, "* Cerebrod Configuration:\n");     
  fprintf(stderr, "* -------------------------------\n");
  fprintf(stderr, "* Command Line Options\n");
  fprintf(stderr, "* -------------------------------\n");
  fprintf(stderr, "* debug: %d\n", conf.debug);
  fprintf(stderr, "* config_file: \"%s\"\n", conf.config_file);
  fprintf(stderr, "* -------------------------------\n");
  fprintf(stderr, "* Configuration File Options\n");
  fprintf(stderr, "* -------------------------------\n");
  fprintf(stderr, "* heartbeat_frequency_min: %d\n", conf.heartbeat_frequency_min);
  fprintf(stderr, "* heartbeat_frequency_max: %d\n", conf.heartbeat_frequency_max);
  fprintf(stderr, "* message_source_port: %d\n", conf.message_source_port);
  fprintf(stderr, "* message_source_network_interface: \"%s\"\n", conf.message_source_network_interface);
  fprintf(stderr, "* message_destination_port: %d\n", conf.message_destination_port);
  fprintf(stderr, "* message_destination_ip: \"%s\"\n", conf.message_destination_ip);
  fprintf(stderr, "* message_ttl: %d\n", conf.message_ttl);
  fprintf(stderr, "* speak: %d\n", conf.speak);
  fprintf(stderr, "* listen: %d\n", conf.listen);
  fprintf(stderr, "* listen_threads: %d\n", conf.listen_threads);
  for (i = 0; i < conf.listen_len; i++)
    {
      fprintf(stderr, "* listen_ports[%d]: %d\n", i, conf.listen_ports[i]);
      fprintf(stderr, "* listen_ips[%d]: \"%s\"\n", i, conf.listen_ips[i]);
      fprintf(stderr, "* listen_network_interface[%d]: \"%s\"\n", i, conf.listen_network_interfaces[i]);
    }
  fprintf(stderr, "* metric_controller: %d\n", conf.metric_controller);
  fprintf(stderr, "* metric_server: %d\n", conf.metric_server);
  fprintf(stderr, "* metric_server_port: %d\n", conf.metric_server_port);
  fprintf(stderr, "* event_server: %d\n", conf.event_server);
  fprintf(stderr, "* event_server_port: %d\n", conf.event_server_port);
  fprintf(stderr, "* speak_debug: %d\n", conf.speak_debug);
  fprintf(stderr, "* listen_debug: %d\n", conf.listen_debug);
  fprintf(stderr, "* metric_controller_debug: %d\n", conf.metric_controller_debug);
  fprintf(stderr, "* metric_server_debug: %d\n", conf.metric_server_debug);
  fprintf(stderr, "* event_server_debug: %d\n", conf.event_server_debug);
  fprintf(stderr, "* -------------------------------\n");
  fprintf(stderr, "* Calculated Configuration\n");
  fprintf(stderr, "* -------------------------------\n");
  fprintf(stderr, "* destination_ip_is_multicast: %d\n", conf.destination_ip_is_multicast);
  fprintf(stderr, "* message_source_network_interface_in_addr: %s\n", inet_ntoa(conf.message_source_network_interface_in_addr));
  fprintf(stderr, "* message_destination_ip_in_addr: %s\n", inet_ntoa(conf.message_destination_ip_in_addr));
  fprintf(stderr, "* message_source_network_interface_index: %d\n", conf.message_source_network_interface_index);
  for (i = 0; i < conf.listen_len; i++)
    {
      fprintf(stderr, "* listen_ips_is_multicast[%d]: %d\n", i, conf.listen_ips_is_multicast[i]);
      fprintf(stderr, "* listen_ips_in_addr[%d]: %s\n", i, inet_ntoa(conf.listen_ips_in_addr[i]));
      fprintf(stderr, "* listen_network_interfaces_in_addr[%d]: %s\n", i, inet_ntoa(conf.listen_network_interfaces_in_addr[i]));
      fprintf(stderr, "* listen_network_interfaces_index[%d]: %d\n", i, conf.listen_network_interfaces_index[i]);
    }
  fprintf(stderr, "**************************************\n");
#endif /* CEREBRO_DEBUG */
}

void
cerebrod_config_setup(int argc, char **argv)
{
  assert(argv);

  _cerebrod_set_config_default();
  _cerebrod_cmdline_arguments_parse(argc, argv);
  _cerebrod_cmdline_arguments_error_check();
  _cerebrod_load_config();
  _cerebrod_config_error_check();
  _cerebrod_calculate_configuration_data();
  _cerebrod_configuration_data_error_check();
  _cerebrod_config_dump();
}
