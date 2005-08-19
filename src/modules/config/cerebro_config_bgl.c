/*****************************************************************************\
 *  $Id: cerebro_config_bgl.c,v 1.1 2005-08-19 00:04:10 achu Exp $
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
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <gendersllnl.h>

#include "cerebro/cerebro_config_module.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_error.h"

#include "debug.h"

extern int h_errno;

#define BGL_CONFIG_MODULE_NAME "bgl"

#define BGL_MGMT_NODE          "bgli"

#define BGL_SUBNET             "172.0.0.1/8"

/*
 * bgl_config_setup
 *
 * bgl config module setup function
 */
static int
bgl_config_setup(void)
{
  /* nothing to setup */
  return 0;
}

/*
 * bgl_config_cleanup
 *
 * bgl config module cleanup function
 */
static int
bgl_config_cleanup(void)
{
  /* nothing to cleanup */
  return 0;
}

/* 
 * bgl_config_load_config
 *
 * config specifically for use on BlueGene/L.  
 *
 * bgli listens, everyone else only speaks.
 *
 * singlecast speak to bgli.
 *
 * All communication is on the 172.*.*.* subnet.
 *
 * Returns 0 on success, -1 on error
 */
int
bgl_config_load_config(struct cerebro_config *conf)
{
  char buf[CEREBRO_MAX_NODENAME_LEN+1];
  char intf[INET_ADDRSTRLEN+1];
  struct hostent *h = NULL;
  struct in_addr in;

  if (!conf)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  memset(buf, '\0', CEREBRO_MAX_NODENAME_LEN+1);
  if (gethostname(buf, CEREBRO_MAX_NODENAME_LEN) < 0)
    {
      CEREBRO_DBG(("gethostname: %s", strerror(errno)));
      return -1;
    }

  if (strstr(buf, BGL_MGMT_NODE))
    {
      conf->cerebrod_speak = 1;
      conf->cerebrod_speak_flag++;
      conf->cerebrod_listen = 1;
      conf->cerebrod_listen_flag++;
      conf->cerebrod_metric_server = 1;
      conf->cerebrod_metric_server_flag++;
    }
  else
    {
      conf->cerebrod_speak = 1;
      conf->cerebrod_speak_flag++;
      conf->cerebrod_listen = 0;
      conf->cerebrod_listen_flag++;
      conf->cerebrod_metric_server = 0;
      conf->cerebrod_metric_server_flag++;
      
      memset(conf->cerebro_hostnames[0], '\0', CEREBRO_MAX_HOSTNAME_LEN+1);
      strncpy(conf->cerebro_hostnames[0], BGL_MGMT_NODE, CEREBRO_MAX_HOSTNAME_LEN);
      conf->cerebro_hostnames_len = 1;
      conf->cerebro_hostnames_flag++;
    }

  if (!(h = gethostbyname(BGL_MGMT_NODE)))
    {
      CEREBRO_DBG(("gethostbyname: %s", hstrerror(h_errno)));
      return -1;
    }

  in = *((struct in_addr *)h->h_addr);

  memset(intf, '\0', INET_ADDRSTRLEN+1);
  if (!inet_ntop(AF_INET, &in, intf, INET_ADDRSTRLEN+1))
    {
      CEREBRO_DBG(("inet_ntop: %s", strerror(errno)));
      return -1;
    }
  
  memset(conf->cerebrod_heartbeat_destination_ip,
         '\0',
         CEREBRO_MAX_IPADDR_LEN+1);
  strncpy(conf->cerebrod_heartbeat_destination_ip,
          intf,
          CEREBRO_MAX_IPADDR_LEN);
  conf->cerebrod_heartbeat_destination_ip_flag++;

  memset(conf->cerebrod_heartbeat_network_interface, 
         '\0', 
         CEREBRO_MAX_NETWORK_INTERFACE_LEN+1);
  strncpy(conf->cerebrod_heartbeat_network_interface, 
          BGL_SUBNET, 
          CEREBRO_MAX_NETWORK_INTERFACE_LEN);
  conf->cerebrod_heartbeat_network_interface_flag++;

  return 0;
}

struct cerebro_config_module_info config_module_info =
  {
    BGL_CONFIG_MODULE_NAME,
    &bgl_config_setup,
    &bgl_config_cleanup,
    &bgl_config_load_config,
  };
