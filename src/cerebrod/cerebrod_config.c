/*****************************************************************************\
 *  $Id: cerebrod_config.c,v 1.20 2005-02-01 01:09:32 achu Exp $
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

#include "cerebrod_config.h"
#include "conffile.h"
#include "error.h"
#include "wrappers.h"

#define MULTICAST_CLASS_MIN 224
#define MULTICAST_CLASS_MAX 239
#define IPADDR_BITS          32   
#define IPADDR6_BITS        128

struct cerebrod_config conf;

static void
_cerebrod_config_default(void)
{
  memset(&conf, '\0', sizeof(struct cerebrod_config));

  conf.debug = 0;
  conf.configfile = CEREBROD_CONFIGFILE_DEFAULT;
  conf.heartbeat_frequency_min = CEREBROD_HEARTBEAT_FREQUENCY_MIN_DEFAULT;
  conf.heartbeat_frequency_max = CEREBROD_HEARTBEAT_FREQUENCY_MAX_DEFAULT;
  conf.heartbeat_source_port = CEREBROD_HEARTBEAT_SOURCE_PORT_DEFAULT;
  conf.heartbeat_destination_port = CEREBROD_HEARTBEAT_DESTINATION_PORT_DEFAULT;
  conf.heartbeat_destination_ip = CEREBROD_HEARTBEAT_DESTINATION_IP_DEFAULT;
  conf.listen = CEREBROD_LISTEN_DEFAULT;
  conf.speak = CEREBROD_SPEAK_DEFAULT;
  conf.speak_from_network_interface = NULL;
  conf.speak_ttl = CEREBROD_SPEAK_TTL_DEFAULT;
  conf.listen_threads = CEREBROD_LISTEN_THREADS_DEFAULT;
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
          "-d    --debug       Turn on debugging (can be used multiple times)\n");
#endif /* NDEBUG */
  exit(0);
}

static void
_version(void)
{
  fprintf(stderr, "%s %s-%s\n", PROJECT, VERSION, RELEASE);
  exit(0);
}

static void
_cerebrod_cmdline_parse(int argc, char **argv)
{ 
  char c;
#ifndef NDEBUG
  char *options = "hvc:d";
#else
  char *options = "hvc:";
#endif /* NDEBUG */

  assert(argv);

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
          conf.debug++;
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

static void
_cerebrod_config_parse(void)
{
  int heartbeat_frequency_flag, heartbeat_source_port_flag, heartbeat_destination_port_flag, 
    heartbeat_destination_ip_flag, listen_flag, speak_flag, 
    speak_from_network_interface_flag, speak_ttl_flag, listen_threads_flag;

  struct conffile_option options[] =
    {
      {"heartbeat_frequency", CONFFILE_OPTION_LIST_INT, -1, _cb_heartbeat_freq,
       1, 0, &heartbeat_frequency_flag, NULL, 0},
      {"heartbeat_destination_port", CONFFILE_OPTION_INT, -1, conffile_int,
       1, 0, &heartbeat_destination_port_flag, &(conf.heartbeat_destination_port), 0},
      {"heartbeat_destination_ip", CONFFILE_OPTION_STRING, -1, _cb_stringptr,
       1, 0, &heartbeat_destination_ip_flag, &(conf.heartbeat_destination_ip), 0},
      {"heartbeat_source_port", CONFFILE_OPTION_INT, -1, conffile_int,
       1, 0, &heartbeat_source_port_flag, &(conf.heartbeat_source_port), 0},
      {"listen", CONFFILE_OPTION_BOOL, -1, conffile_bool,
       1, 0, &listen_flag, &conf.listen, 0},
      {"speak", CONFFILE_OPTION_BOOL, -1, conffile_bool,
       1, 0, &speak_flag, &conf.speak, 0},
      {"speak_from_network_interface", CONFFILE_OPTION_STRING, -1, _cb_stringptr,
       1, 0, &speak_from_network_interface_flag, 
       &(conf.speak_from_network_interface), 0},
      {"speak_ttl", CONFFILE_OPTION_INT, -1, conffile_int,
       1, 0, &speak_ttl_flag, &(conf.speak_ttl), 0},
      {"listen_threads", CONFFILE_OPTION_INT, -1, conffile_int,
       1, 0, &listen_threads_flag, &(conf.listen_threads), 0},
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
          err_exit("config file error: %s", buf);
        }
    }

  conffile_handle_destroy(cf);
}

static void
_cerebrod_config_check(void)
{
  struct in_addr addr_temp;
  
  if (!Inet_pton(AF_INET, conf.heartbeat_destination_ip, &addr_temp))
    err_exit("heartbeat destination IP address '%s' improperly format",
	     conf.heartbeat_destination_ip);

  if (conf.speak_from_network_interface
      && strchr(conf.speak_from_network_interface, '.'))
    {
      if (strchr(conf.speak_from_network_interface, '/'))
        {
          char *speak_from_network_interface_cpy = Strdup(conf.speak_from_network_interface);
	  char *tok;

          tok = strtok(speak_from_network_interface_cpy, "/");
          if (!Inet_pton(AF_INET, tok, &addr_temp))
            err_exit("speak from network interface IP address '%s' "
                     "improperly format", conf.speak_from_network_interface);

          Free(speak_from_network_interface_cpy);
        }
      else
        {
          if (!Inet_pton(AF_INET, conf.speak_from_network_interface, &addr_temp))
            err_exit("speak from network interface IP address '%s' "
                     "improperly format", conf.speak_from_network_interface);
        }
    }

  if (conf.speak_ttl <= 0)
    err_exit("speak ttl '%d' invalid", conf.speak_ttl);
}

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

static void
_cerebrod_calculate_heartbeat_frequency_ranged(void)
{
  if (conf.heartbeat_frequency_max > 0)
    conf.heartbeat_frequency_ranged = 1;
  else
    conf.heartbeat_frequency_ranged = 0;
}

static void
_cerebrod_calculate_heartbeat_destination_in_addr(void)
{
  if (!Inet_pton(AF_INET, conf.heartbeat_destination_ip, &conf.heartbeat_destination_in_addr))
    err_exit("heartbeat destination IP address '%s' improperly format",
	     conf.heartbeat_destination_ip);
}

/* From Unix Network Programming, by R. Stevens, Chapter 16 */
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

/* From Unix Network Programming, by R. Stevens, Chapter 16 */
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

static void
_cerebrod_calculate_speak_from_in_addr(void)
{
  if (!conf.speak_from_network_interface)
    {
      /* Case A: No interface specified
       * - If multicast speak to IP address specified, find a multicast interface
       * - If singlecast speak to IP address specified, use INADDR_ANY
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
	      conf.speak_from_in_addr.s_addr = sinptr->sin_addr.s_addr;
	      conf.speak_from_interface_index = ifr_tmp.ifr_ifindex;
	      found_interface++;
	      break;
	    }

	  if (!found_interface)
	    err_exit("network interface with multicast not found", 
		     conf.speak_from_network_interface);

	  Free(buf);
	  Close(fd);
	}
      else
	conf.speak_from_in_addr.s_addr = INADDR_ANY;
    }
  else if (strchr(conf.speak_from_network_interface, '.'))
    {
      /* Case B: IP address or subnet specified */
      struct ifconf ifc;
      struct ifreq *ifr;
      void *buf = NULL, *ptr = NULL;
      int fd, mask, found_interface = 0;
      struct in_addr addr_temp;
      u_int32_t conf_masked_ip;

      fd = Socket(AF_INET, SOCK_DGRAM, 0);

      _get_if_conf(&buf, &ifc, fd);
     
      if (strchr(conf.speak_from_network_interface, '/'))
	{
	  char *speak_from_network_interface_cpy = Strdup(conf.speak_from_network_interface);
	  char *snm, *ptr, *tok;

	  tok = strtok(speak_from_network_interface_cpy, "/");
	  if (!Inet_pton(AF_INET, tok, &addr_temp))
            err_exit("speak from network interface IP address '%s' "
                     "improperly format", conf.speak_from_network_interface);

	  if (!(snm = strtok(NULL, "")))
	    err_exit("network interface '%s' subnet mask not specified",
		     conf.speak_from_network_interface);
	  else
	    {
	      mask = strtol(snm, &ptr, 10);
	      if (ptr != (snm + strlen(snm)))
		err_exit("network interface '%s' subnet mask improper",
			 conf.speak_from_network_interface);
	      else if (mask < 1 || mask > IPADDR_BITS)
		err_exit("network interface '%s' subnet mask size invalid",
			 conf.speak_from_network_interface);
	    }

	  Free(speak_from_network_interface_cpy);
	}
      else
	{
	  /* If no '/', then just an IP address, mask is all bits */
	  if (!Inet_pton(AF_INET, conf.speak_from_network_interface, &addr_temp))
            err_exit("speak from network interface IP address '%s' "
                     "improperly format", conf.speak_from_network_interface);
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

	      conf.speak_from_in_addr.s_addr = sinptr->sin_addr.s_addr;
	      conf.speak_from_interface_index = ifr_tmp.ifr_ifindex;
	      found_interface++;
	      break;
	    }
	}
      
      if (!found_interface)
	err_exit("network interface '%s' with multicast "
		 "not found", conf.speak_from_network_interface);
      
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
          
	  if (!strcmp(conf.speak_from_network_interface, ifr->ifr_name))
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
			 conf.speak_from_network_interface);
	      
	      if (conf.multicast && !(ifr_tmp.ifr_flags & IFF_MULTICAST))
		err_exit("network interface '%s' not a multicast interface",
			 conf.speak_from_network_interface);

	      /* Null termination not required, don't use strncpy() wrapper */
	      memset(&ifr_tmp, '\0', sizeof(struct ifreq));
	      strncpy(ifr_tmp.ifr_name, ifr->ifr_name, IFNAMSIZ);

	      if(ioctl(fd, SIOCGIFINDEX, &ifr_tmp) < 0)
		err_exit("cerebrod_calculate_configuration: ioctl: %s",
			 strerror(errno));

	      sinptr = (struct sockaddr_in *)&ifr->ifr_addr;
	      conf.speak_from_in_addr.s_addr = sinptr->sin_addr.s_addr;
	      conf.speak_from_interface_index = ifr_tmp.ifr_ifindex;
	      found_interface++;
	      break;
	    }
	}

      if (!found_interface)
	err_exit("network interface '%s' not found",
		 conf.speak_from_network_interface);

      Free(buf);
      Close(fd);
    }
}

static void
_cerebrod_calculate_configuration(void)
{
  /* Step 0: Determine if we are single casting or multicasting */
  _cerebrod_calculate_multicast();

  /* Step 1: Determine if the heartbeat frequencey is ranged or fixed */
  _cerebrod_calculate_heartbeat_frequency_ranged();

  /* Step 2: Calculate the destination ip */
  _cerebrod_calculate_heartbeat_destination_in_addr();

  /* Step 3: Determine the appropriate network interface to use based
   * on the user's speak_from_network_interface input.
   */
  _cerebrod_calculate_speak_from_in_addr();
}

static void 
_cerebrod_config_dump(void)
{
#ifndef NDEBUG
  if (conf.debug)
    {
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Cerebrod Configuration:\n");     
      fprintf(stderr, "* -----------------------\n");
      fprintf(stderr, "* debug: %d\n", conf.debug);
      fprintf(stderr, "* configfile: \"%s\"\n", conf.configfile);
      fprintf(stderr, "* heartbeat_frequency_min: %d\n", conf.heartbeat_frequency_min);
      fprintf(stderr, "* heartbeat_frequency_max: %d\n", conf.heartbeat_frequency_max);
      fprintf(stderr, "* heartbeat_source_port: %d\n", conf.heartbeat_source_port);
      fprintf(stderr, "* heartbeat_destination_port: %d\n", conf.heartbeat_destination_port);
      fprintf(stderr, "* heartbeat_destination_ip: \"%s\"\n", conf.heartbeat_destination_ip);
      fprintf(stderr, "* listen: %d\n", conf.listen);
      fprintf(stderr, "* speak: %d\n", conf.speak);
      fprintf(stderr, "* speak_from_network_interface: \"%s\"\n", conf.speak_from_network_interface);
      fprintf(stderr, "* multicast: %d\n", conf.multicast);
      fprintf(stderr, "* heartbeat_destination_in_addr: %s\n", inet_ntoa(conf.heartbeat_destination_in_addr));
      fprintf(stderr, "* speak_from_in_addr: %s\n", inet_ntoa(conf.speak_from_in_addr));
      fprintf(stderr, "* speak_from_interface_index: %d\n", conf.speak_from_interface_index);
      fprintf(stderr, "**************************************\n");
    }
#endif /* NDEBUG */
}

void
cerebrod_config(int argc, char **argv)
{
  assert(argv);

  _cerebrod_config_default();
  _cerebrod_cmdline_parse(argc, argv);
  _cerebrod_config_parse();
  _cerebrod_config_check();
  _cerebrod_calculate_configuration();
  _cerebrod_config_dump();
}
