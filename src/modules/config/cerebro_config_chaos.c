/*****************************************************************************\
 *  $Id: cerebro_config_chaos.c,v 1.2 2005-08-23 21:10:15 achu Exp $
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

#define CHAOS_CONFIG_MODULE_NAME "chaos"

#define CHAOS_LARGE_CLUSTER_SIZE 512

/*
 * gh
 *
 * genders handle
 */
static genders_t gh = NULL;

/*
 * chaos_config_setup
 *
 * chaos config module setup function
 */
static int
chaos_config_setup(void)
{
  if (gh)
    {
      CEREBRO_DBG(("gh non-null"));
      return 0;
    }

  if (!(gh = genders_handle_create()))
    {
      CEREBRO_DBG(("genders_handle_create"));
      goto cleanup;
    }
 
  if (genders_load_data(gh, NULL) < 0)
    {
      if (genders_errnum(gh) == GENDERS_ERR_OPEN)
        {
          cerebro_err_output("genders database '%s' cannot be opened",
                             GENDERS_DEFAULT_FILE);
          goto cleanup;
        }
      else
        {
          CEREBRO_DBG(("genders_load_data: %s", genders_errormsg(gh)));
          goto cleanup;
        }
    }

  return 0;

 cleanup:
  if (gh)
    genders_handle_destroy(gh);
  gh = NULL;
  return -1;

}

/*
 * chaos_config_cleanup
 *
 * chaos config module cleanup function
 */
static int
chaos_config_cleanup(void)
{
  if (!gh)
    return 0;

  if (genders_handle_destroy(gh) < 0)
    CEREBRO_DBG(("genders_handle_destroy: %s", genders_errormsg(gh)));

  gh = NULL;
  return 0;
}

/* 
 * chaos_config_load_config
 *
 * config specifically for use on LLNL clusters. 'mgmt' nodes listen
 * and speak, while compute nodes only speak.  We always speak on the
 * private management network interface (the 'e' interface).  Compute
 * nodes connect to mgmt nodes via the private management network to
 * retrieve metric data.
 *
 * Returns 0 on success, -1 on error
 */
int
chaos_config_load_config(struct cerebro_config *conf)
{
  char altnamebuf[CEREBRO_MAX_NODENAME_LEN+1];
  int flag, numnodes;

  if (!gh)
    {
      CEREBRO_DBG(("gh null"));
      return -1;
    }

  if (!conf)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  if ((numnodes = genders_getnumnodes(gh)) < 0)
    {
      CEREBRO_DBG(("genders_numnodes: %s", genders_errormsg(gh)));
      return -1;
    }
                                   
  if ((flag = genders_testattr(gh, NULL, "mgmt", NULL, 0)) < 0)
    {
      CEREBRO_DBG(("genders_testattr: %s", genders_errormsg(gh)));
      return -1;
    }
  
  if (flag)
    {
      conf->cerebrod_speak = 1;
      conf->cerebrod_speak_flag++;
      conf->cerebrod_listen = 1;
      conf->cerebrod_listen_flag++;
      if (numnodes >= CHAOS_LARGE_CLUSTER_SIZE)
        {
          conf->cerebrod_listen_threads = 4;
          conf->cerebrod_listen_threads_flag++;
        }
      conf->cerebrod_metric_server = 1;
      conf->cerebrod_metric_server_flag++;
    }
  else
    {
      char **altnodelist = NULL;
      int i, altnodelist_len, mgmt_len;

      conf->cerebrod_speak = 1;
      conf->cerebrod_speak_flag++;
      conf->cerebrod_listen = 0;
      conf->cerebrod_listen_flag++;
      conf->cerebrod_metric_server = 0;
      conf->cerebrod_metric_server_flag++;

      if ((altnodelist_len = genders_altnodelist_create(gh, &altnodelist)) < 0)
	{
	  CEREBRO_DBG(("genders_altnodelist_create: %s", genders_errormsg(gh)));
	  return -1;
	}
      
      if ((mgmt_len = genders_getaltnodes_preserve(gh,
                                                   altnodelist,
                                                   altnodelist_len,
                                                   "mgmt",
                                                   NULL)) < 0)
	{
	  CEREBRO_DBG(("genders_getnodes: %s", genders_errormsg(gh)));
	  genders_altnodelist_destroy(gh, altnodelist);      
	  return -1;
	}
      
      if (mgmt_len > CEREBRO_CONFIG_HOSTNAMES_MAX)
	mgmt_len = CEREBRO_CONFIG_HOSTNAMES_MAX;
      
      for (i = 0 ; i < mgmt_len; i++)
	{
	  if (strlen(altnodelist[i]) > CEREBRO_MAX_NODENAME_LEN)
	    {
	      CEREBRO_DBG(("genders_getnodes: %s", genders_errormsg(gh)));
	      genders_altnodelist_destroy(gh, altnodelist);      
	      return -1;
	    }
	  strcpy(conf->cerebro_hostnames[i], altnodelist[i]);
	}
      conf->cerebro_hostnames_len = mgmt_len;
      conf->cerebro_hostnames_flag++;
      
      genders_altnodelist_destroy(gh, altnodelist);      
    }
  
  memset(altnamebuf, '\0', CEREBRO_MAX_NODENAME_LEN+1);
  if ((flag = genders_testattr(gh, 
			       NULL, 
			       GENDERS_ALTNAME_ATTRIBUTE,
			       altnamebuf,
			       CEREBRO_MAX_NODENAME_LEN)) < 0)
    {
      CEREBRO_DBG(("genders_testattr: %s", genders_errormsg(gh)));
      return -1;
    }

  if (flag)
    {
      char intf[INET_ADDRSTRLEN+1];
      struct hostent *h = NULL;
      struct in_addr in;
      
      if (!(h = gethostbyname(altnamebuf)))
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
      
      memset(conf->cerebrod_heartbeat_network_interface, 
             '\0', 
             CEREBRO_MAX_NETWORK_INTERFACE_LEN+1);
      strncpy(conf->cerebrod_heartbeat_network_interface, 
	      intf, 
	      CEREBRO_MAX_NETWORK_INTERFACE_LEN);
      conf->cerebrod_heartbeat_network_interface_flag++;
    }

  return 0;
}

#if WITH_STATIC_MODULES
struct cerebro_config_module_info chaos_config_module_info =
#else  /* !WITH_STATIC_MODULES */
struct cerebro_config_module_info config_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    CHAOS_CONFIG_MODULE_NAME,
    &chaos_config_setup,
    &chaos_config_cleanup,
    &chaos_config_load_config,
  };
