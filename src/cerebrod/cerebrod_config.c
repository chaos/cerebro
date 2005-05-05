/*****************************************************************************\
 *  $Id: cerebrod_config.c,v 1.91 2005-05-05 22:24:59 achu Exp $
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

#include "cerebro/cerebro_error.h"
#include "cerebro/cerebro_config.h"
#include "cerebro/cerebro_config_module.h"
#include "cerebro/cerebro_module.h"

#include "cerebrod_config.h"
#include "cerebrod_util.h"
#include "wrappers.h"

#define MULTICAST_CLASS_MIN 224
#define MULTICAST_CLASS_MAX 239
#define IPADDR_BITS          32   
#define IPADDR6_BITS        128

#ifndef NDEBUG
extern char *cerebro_config_debug_config_file;
#endif /* NDEBUG */

/* 
 * conf
 *
 * cerebrod configuration used by all of cerebrod
 */
struct cerebrod_config conf;

/* 
 * _cerebrod_config_default 
 *
 * initialize conf structure with default values.
 */
static void
_cerebrod_config_default(void)
{
  memset(&conf, '\0', sizeof(struct cerebrod_config));

#ifndef NDEBUG
  conf.debug = CEREBROD_DEBUG_DEFAULT;
#endif /* NDEBUG */
  conf.config_file = CEREBRO_CONFIG_FILE_DEFAULT;
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
#ifndef NDEBUG
  conf.speak_debug = CEREBROD_SPEAK_DEBUG_DEFAULT;
  conf.listen_debug = CEREBROD_LISTEN_DEBUG_DEFAULT;
  conf.updown_server_debug = CEREBROD_UPDOWN_SERVER_DEBUG_DEFAULT;
#endif /* NDEBUG */
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
#ifndef NDEBUG
  fprintf(stderr, 
          "-c    --config_file   Specify alternate config file\n"
          "-d    --debug         Turn on debugging and run daemon\n"
	  "                      in foreground\n");
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
 * _cerebrod_cmdline
 *
 * parse command line options
 */
static void
_cerebrod_cmdline(int argc, char **argv)
{ 
  char c;
  char options[100];

#if HAVE_GETOPT_LONG
  struct option long_options[] =
    {
      {"help",                0, NULL, 'h'},
      {"version",             0, NULL, 'v'},
#ifndef NDEBUG
      {"config-file",         1, NULL, 'c'},
      {"debug",               0, NULL, 'd'},
#endif /* NDEBUG */
    };
#endif /* HAVE_GETOPT_LONG */

  assert(argv);

  memset(options, '\0', sizeof(options));
  strcat(options, "hvc:");
#ifndef NDEBUG
  strcat(options, "d");
#endif /* NDEBUG */

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
#ifndef NDEBUG
        case 'c':       /* --config-file */
          conf.config_file = Strdup(optarg);
          break;
        case 'd':       /* --debug */
          conf.debug++;
          break;
#endif /* NDEBUG */
        case '?':
        default:
          cerebro_err_exit("unknown command line option '%c'", optopt);
        }          
    }
}

/* 
 * _cerebrod_cmdline_check:
 *
 * check validity of command line parameters
 */
static void
_cerebrod_cmdline_check(void)
{
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

  return;
}

/*
 * _cerebrod_config_setup
 *
 * load configuration module.
 */
static void
_cerebrod_config_setup(void)
{
  struct cerebro_config conf_l;

#ifndef NDEBUG
  cerebro_config_debug_config_file = conf.config_file;
#endif /* NDEBUG */

  if (cerebro_config_load(&conf_l) < 0)
    cerebro_err_exit("%s(%s:%d): cerebro_config_load",
		     __FILE__, __FUNCTION__, __LINE__);
  
  if (conf_l.cerebrod_heartbeat_frequency_flag)
    {
      conf.heartbeat_frequency_min = conf_l.cerebrod_heartbeat_frequency_min;
      conf.heartbeat_frequency_max = conf_l.cerebrod_heartbeat_frequency_max;
    }
  if (conf_l.cerebrod_heartbeat_source_port_flag)
    conf.heartbeat_source_port = conf_l.cerebrod_heartbeat_source_port;
  if (conf_l.cerebrod_heartbeat_destination_port_flag)
    conf.heartbeat_destination_port = conf_l.cerebrod_heartbeat_destination_port;
  if (conf_l.cerebrod_heartbeat_destination_ip_flag)
    conf.heartbeat_destination_ip = Strdup(conf_l.cerebrod_heartbeat_destination_ip);
  if (conf_l.cerebrod_heartbeat_network_interface_flag)
    conf.heartbeat_network_interface = Strdup(conf_l.cerebrod_heartbeat_network_interface);

  if (conf_l.cerebrod_heartbeat_ttl_flag)
    conf.heartbeat_ttl = conf_l.cerebrod_heartbeat_ttl;
  if (conf_l.cerebrod_speak_flag)
    conf.speak = conf_l.cerebrod_speak;
  if (conf_l.cerebrod_listen_flag)
    conf.listen = conf_l.cerebrod_listen;
  if (conf_l.cerebrod_listen_threads_flag)
    conf.listen_threads = conf_l.cerebrod_listen_threads;
  if (conf_l.cerebrod_updown_server_flag)
    conf.updown_server = conf_l.cerebrod_updown_server;
  if (conf_l.cerebrod_updown_server_port_flag)
    conf.updown_server_port = conf_l.cerebrod_updown_server_port;
#ifndef NDEBUG
  if (conf_l.cerebrod_speak_debug_flag)
    conf.speak_debug = conf_l.cerebrod_speak_debug;
  if (conf_l.cerebrod_listen_debug_flag)
    conf.listen_debug = conf_l.cerebrod_listen_debug;
  if (conf_l.cerebrod_updown_server_debug_flag)
    conf.updown_server_debug = conf_l.cerebrod_updown_server_debug;
#endif /* NDEBUG */
}

/*
 * _cerebrod_pre_calculate_config_check
 * 
 * Check configuration settings for errors before analyzing
 * configuration data.
 */
static void
_cerebrod_pre_calculate_config_check(void)
{
  struct in_addr addr_temp;
  
  if (!Inet_pton(AF_INET, conf.heartbeat_destination_ip, &addr_temp))
    cerebro_err_exit("heartbeat destination IP address '%s' improperly format",
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
	    cerebro_err_exit("network interface IP address '%s' "
                             "improperly format", 
                             conf.heartbeat_network_interface);
	  
	  Free(heartbeat_network_interface_cpy);
	}
      else
	{
	  if (!Inet_pton(AF_INET, conf.heartbeat_network_interface, &addr_temp))
	    cerebro_err_exit("network interface IP address '%s' "
                             "improperly format", 
                             conf.heartbeat_network_interface);
	}
    }

  if (conf.heartbeat_destination_port == conf.heartbeat_source_port)
    cerebro_err_exit("heartbeat destination and source ports '%d' "
		      "cannot be identical",
		      conf.heartbeat_destination_port);

  if (conf.heartbeat_ttl <= 0)
    cerebro_err_exit("heartbeat ttl '%d' invalid", conf.heartbeat_ttl);

  if (conf.listen_threads <= 0)
    cerebro_err_exit("listen threads '%d' invalid", conf.listen_threads);

  /* If the listening server is turned off, none of the other
   * servers can be on.  So we turn them off.
   */
  if (!conf.listen)
    conf.updown_server = 0;

  if (conf.updown_server)
    {
      if (conf.updown_server_port == conf.heartbeat_destination_port)
	cerebro_err_exit("updown server port '%d' cannot be identical "
                         "to heartbeat destination port");
      if (conf.updown_server_port == conf.heartbeat_source_port)
	cerebro_err_exit("updown server port '%d' cannot be identical "
                         "to heartbeat source port");
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
	cerebro_err_exit("heartbeat destination IP address '%s' "
                         "improperly format",
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
  if (!Inet_pton(AF_INET, 
                 conf.heartbeat_destination_ip, 
                 &conf.heartbeat_destination_ip_in_addr))
    cerebro_err_exit("heartbeat destination IP address '%s' "
                     "improperly format",
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

  assert(buf);
  assert(ifc);

  for(;;)
    {
      *buf = Malloc(len);
      ifc->ifc_len = len;
      ifc->ifc_buf = *buf;

      if (ioctl(fd, SIOCGIFCONF, ifc) < 0)
        cerebro_err_exit("%s(%s:%d): ioctl: %s", 
                         __FILE__, __FUNCTION__, __LINE__,
                         strerror(errno));

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
  assert(interface_in_addr);
  assert(interface_index);
  
  if (!network_interface)
    {
      /* Case A: No interface specified
       * - If multicast heartbeat IP address specified, find a
       *   multicast interface
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
		cerebro_err_exit("%s(%s:%d): ioctl: %s",
                                 __FILE__, __FUNCTION__, __LINE__,
                                 strerror(errno));

	      if (!(ifr_tmp.ifr_flags & IFF_UP)
		  || !(ifr_tmp.ifr_flags & IFF_MULTICAST))
		continue;

	      /* Null termination not required, don't use strncpy() wrapper */
	      memset(&ifr_tmp, '\0', sizeof(struct ifreq));
	      strncpy(ifr_tmp.ifr_name, ifr->ifr_name, IFNAMSIZ);

	      if(ioctl(fd, SIOCGIFINDEX, &ifr_tmp) < 0)
		cerebro_err_exit("%s(%s:%d): ioctl: %s",
                                 __FILE__, __FUNCTION__, __LINE__,
                                 strerror(errno));
	      
	      sinptr = (struct sockaddr_in *)&ifr->ifr_addr;
	      interface_in_addr->s_addr = sinptr->sin_addr.s_addr;
	      *interface_index = ifr_tmp.ifr_ifindex;
	      found_interface++;
	      break;
	    }

	  if (!found_interface)
	    cerebro_err_exit("network interface with multicast not found", 
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
            cerebro_err_exit("network interface IP address '%s' "
                             "improperly format", network_interface);
          
	  if (!(snm = strtok(NULL, "")))
	    cerebro_err_exit("network interface '%s' subnet mask not specified",
                             network_interface);
	  else
	    {
	      mask = strtol(snm, &ptr, 10);
	      if (ptr != (snm + strlen(snm)))
		cerebro_err_exit("network interface '%s' subnet mask improper",
                                 network_interface);
	      else if (mask < 1 || mask > IPADDR_BITS)
		cerebro_err_exit("network interface '%s' subnet mask size "
                                 "invalid",
                                 network_interface);
	    }

	  Free(network_interface_cpy);
	}
      else
	{
	  /* If no '/', then just an IP address, mask is all bits */
	  if (!Inet_pton(AF_INET, network_interface, &addr_temp))
            cerebro_err_exit("network interface IP address '%s' "
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
	    cerebro_err_exit("%s(%s:%d): ioctl: %s",
                             __FILE__, __FUNCTION__, __LINE__,
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
		cerebro_err_exit("%s(%s:%d): ioctl: %s",
                                 __FILE__, __FUNCTION__, __LINE__,
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
	    cerebro_err_exit("network interface '%s' with multicast "
                             "not found", network_interface);
	  else
	    cerebro_err_exit("network interface '%s' not found", 
                             network_interface);
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
		cerebro_err_exit("%s(%s:%d): ioctl: %s",
                                 __FILE__, __FUNCTION__, __LINE__,
                                 strerror(errno));

	      if (!(ifr_tmp.ifr_flags & IFF_UP))
		cerebro_err_exit("network interface '%s' not up",
                                 network_interface);
	      
	      if (conf.multicast && !(ifr_tmp.ifr_flags & IFF_MULTICAST))
		cerebro_err_exit("network interface '%s' not a multicast "
                                 "interface",
                                 network_interface);

	      /* Null termination not required, don't use strncpy() wrapper */
	      memset(&ifr_tmp, '\0', sizeof(struct ifreq));
	      strncpy(ifr_tmp.ifr_name, ifr->ifr_name, IFNAMSIZ);

	      if(ioctl(fd, SIOCGIFINDEX, &ifr_tmp) < 0)
		cerebro_err_exit("%s(%s:%d): ioctl: %s",
                                 __FILE__, __FUNCTION__, __LINE__,
                                 strerror(errno));

	      sinptr = (struct sockaddr_in *)&ifr->ifr_addr;
	      interface_in_addr->s_addr = sinptr->sin_addr.s_addr;
	      *interface_index = ifr_tmp.ifr_ifindex;
	      found_interface++;
	      break;
	    }
	}

      if (!found_interface)
	cerebro_err_exit("network interface '%s' not found",
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
 * _cerebrod_calculate_config
 *
 * analyze and calculate configuration based on settings
 */
static void
_cerebrod_calculate_config(void)
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
}

/*
 * _cerebrod_post_calculate_config_check
 * 
 * Check configuration settings for errors after configuration data
 * has been analyzed.
 */
static void
_cerebrod_post_calculate_config_check(void)
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
        cerebro_err_exit("heartbeat destination IP address '%s' "
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
        cerebro_err_exit("heartbeat destination address not found");
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
      fprintf(stderr, "**************************************\n");
    }
#endif /* NDEBUG */
}

void
cerebrod_config_setup(int argc, char **argv)
{
  assert(argv);

  _cerebrod_config_default();
  _cerebrod_cmdline(argc, argv);
  _cerebrod_cmdline_check();
  _cerebrod_config_setup();
  _cerebrod_pre_calculate_config_check();
  _cerebrod_calculate_config();
  _cerebrod_post_calculate_config_check();
  _cerebrod_config_dump();
}

