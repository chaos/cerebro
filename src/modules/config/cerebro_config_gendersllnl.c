/*****************************************************************************\
 *  $Id: cerebro_config_gendersllnl.c,v 1.8 2005-05-04 01:15:30 achu Exp $
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <gendersllnl.h>

#include "cerebro_defs.h"
#include "cerebro_error.h"
#include "cerebro_config_module.h"

extern int h_errno;

#define GENDERSLLNL_CONFIG_MODULE_NAME "gendersllnl"

/*
 * gendersllnl_handle
 *
 * genders handle
 */
static genders_t gendersllnl_handle = NULL;

/*
 * gendersllnl_config_setup
 *
 * gendersllnl config module setup function
 */
static int
gendersllnl_config_setup(void)
{
  if (gendersllnl_handle)
    {
      cerebro_err_debug_module("%s(%s:%d): %s clusterlist module: "
			       "gendersllnl_handle non-null",
			       GENDERSLLNL_CONFIG_MODULE_NAME,
			       __FILE__, __FUNCTION__, __LINE__);
      return 0;
    }

  if (!(gendersllnl_handle = genders_handle_create()))
    {
      cerebro_err_debug_module("%s(%s:%d): %s clusterlist module: "
			       "genders_handle_create",
			       GENDERSLLNL_CONFIG_MODULE_NAME,
			       __FILE__, __FUNCTION__, __LINE__);
      goto cleanup;
    }
 
  if (genders_load_data(gendersllnl_handle, NULL) < 0)
    {
      if (genders_errnum(gendersllnl_handle) == GENDERS_ERR_OPEN)
        {
          cerebro_err_debug_module("%s config module: genders database '%s' "
				   "cannot be opened",
				   GENDERSLLNL_CONFIG_MODULE_NAME, 
				   GENDERS_DEFAULT_FILE);
          goto cleanup;
        }
      else
        {
          cerebro_err_debug_module("%s(%s:%d): %s config module: "
				   "genders_load_data: %s",
				   __FILE__, __FUNCTION__, __LINE__,
				   GENDERSLLNL_CONFIG_MODULE_NAME, 
				   genders_errormsg(gendersllnl_handle));
          goto cleanup;
        }
    }

  return 0;

 cleanup:
  if (gendersllnl_handle)
    genders_handle_destroy(gendersllnl_handle);
  gendersllnl_handle = NULL;
  return -1;

}

/*
 * gendersllnl_config_cleanup
 *
 * gendersllnl config module cleanup function
 */
static int
gendersllnl_config_cleanup(void)
{
  if (!gendersllnl_handle)
    {
      cerebro_err_debug_module("%s(%s:%d): %s clusterlist module: "
			       "gendersllnl_handle null",
			       GENDERSLLNL_CONFIG_MODULE_NAME,
			       __FILE__, __FUNCTION__, __LINE__);
      return 0;
    }

  if (genders_handle_destroy(gendersllnl_handle) < 0)
    {
      cerebro_err_debug_module("%s(%s:%d): %s config module: "
			       "genders_handle_destroy: %s",
			       __FILE__, __FUNCTION__, __LINE__,
			       GENDERSLLNL_CONFIG_MODULE_NAME, 
			       genders_errormsg(gendersllnl_handle));
    }

  gendersllnl_handle = NULL;
  return 0;
}

/* 
 * gendersllnl_config_load_default
 *
 * alter default module specifically for use on LLNL clusters. 'mgmt'
 * nodes listen and speak, while compute nodes only speak.  We always
 * speak out of the private management network interface.  Non-mgmt
 * nodes connect to mgmt nodes to receive updown data.
 *
 * Returns 0 on success, -1 on error
 */
int
gendersllnl_config_load_default(struct cerebro_config *conf)
{
  char altnamebuf[CEREBRO_MAXNODENAMELEN+1];
  int is_mgmt, ret;

  if (!gendersllnl_handle)
    {
      cerebro_err_debug_module("%s(%s:%d): %s clusterlist module: "
			       "gendersllnl_handle null",
			       GENDERSLLNL_CONFIG_MODULE_NAME,
			       __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!conf)
    {
      cerebro_err_debug_module("%s(%s:%d): %s clusterlist module: "
			       "invalid conf parameter",
			       GENDERSLLNL_CONFIG_MODULE_NAME,
			       __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if ((is_mgmt = genders_testattr(gendersllnl_handle, NULL, "mgmt", NULL, 0)) < 0)
    {
      cerebro_err_debug_module("%s(%s:%d): genders_testattr: %s", 
			       __FILE__, __FUNCTION__, __LINE__,
			       genders_errormsg(gendersllnl_handle));
      return -1;
    }
  
  if (is_mgmt)
    {
      conf->cerebrod_speak = 1;
      conf->cerebrod_speak_flag++;
      conf->cerebrod_listen = 1;
      conf->cerebrod_listen_flag++;
      conf->cerebrod_updown_server = 1;
      conf->cerebrod_updown_server_flag++;
    }
  else
    {
      char **nodelist = NULL;
      int i, nodelist_len, mgmt_len;

      conf->cerebrod_speak = 1;
      conf->cerebrod_speak_flag++;
      conf->cerebrod_listen = 0;
      conf->cerebrod_listen_flag++;
      conf->cerebrod_updown_server = 0;
      conf->cerebrod_updown_server_flag++;

      if ((nodelist_len = genders_nodelist_create(gendersllnl_handle, &nodelist)) < 0)
	{
	  cerebro_err_debug_module("%s(%s:%d): genders_nodelist_create: %s",
				   __FILE__, __FUNCTION__, __LINE__,
				   genders_errormsg(gendersllnl_handle));
	  return -1;
	}
      
      if ((mgmt_len = genders_getnodes(gendersllnl_handle,
				       nodelist,
				       nodelist_len,
				       "mgmt",
				       NULL)) < 0)
	{
	  cerebro_err_debug_module("%s(%s:%d): genders_getnodes: %s",
				   __FILE__, __FUNCTION__, __LINE__,
				   genders_errormsg(gendersllnl_handle));
	  genders_nodelist_destroy(gendersllnl_handle, nodelist);      
	  return -1;
	}

      if (mgmt_len > CEREBRO_CONFIG_UPDOWN_HOSTNAMES_MAX)
	mgmt_len = CEREBRO_CONFIG_UPDOWN_HOSTNAMES_MAX;

      for (i = 0 ; i < mgmt_len; i++)
	{
	  if (strlen(nodelist[i]) > CEREBRO_MAXNODENAMELEN)
	    {
	      cerebro_err_debug_module("%s(%s:%d): genders_getnodes: %s",
				       __FILE__, __FUNCTION__, __LINE__,
				       genders_errormsg(gendersllnl_handle));
	      genders_nodelist_destroy(gendersllnl_handle, nodelist);      
	      return -1;
	    }
	  strcpy(conf->cerebro_updown_hostnames[i], nodelist[i]);
	}
      conf->cerebro_updown_hostnames_len = mgmt_len;
      conf->cerebro_updown_hostnames_flag++;
      
      genders_nodelist_destroy(gendersllnl_handle, nodelist);      
    }

  memset(altnamebuf, '\0', CEREBRO_MAXNODENAMELEN+1);
  if ((ret = genders_testattr(gendersllnl_handle, 
                              NULL, 
                              GENDERS_ALTNAME_ATTRIBUTE,
                              altnamebuf,
                              CEREBRO_MAXNODENAMELEN)) < 0)
    {
      cerebro_err_debug_module("%s(%s:%d): genders_testattr: %s",
			       __FILE__, __FUNCTION__, __LINE__,
			       genders_errormsg(gendersllnl_handle));
      return -1;
    }

  if (ret)
    {
      char heartbeat_network_interface[INET_ADDRSTRLEN+1];
      struct hostent *h = NULL;
      struct in_addr in;

      if (!(h = gethostbyname(altnamebuf)))
        {
          cerebro_err_debug_module("%s(%s:%d): gethostbyname: %s",
				   __FILE__, __FUNCTION__, __LINE__,
				   hstrerror(h_errno));
          return -1;
        }

      in = *((struct in_addr *)h->h_addr);

      memset(heartbeat_network_interface, '\0', INET_ADDRSTRLEN+1);
      if (!inet_ntop(AF_INET, 
                     &in, 
                     heartbeat_network_interface,
                     INET_ADDRSTRLEN+1))
        {
          cerebro_err_debug_module("%s(%s:%d): inet_ntop: %s",
				   __FILE__, __FUNCTION__, __LINE__,
				   strerror(errno));
          return -1;
        }
     
      memset(conf->cerebrod_heartbeat_network_interface, '\0', CEREBRO_MAXNETWORKINTERFACE+1);
      strncpy(conf->cerebrod_heartbeat_network_interface, heartbeat_network_interface, CEREBRO_MAXNETWORKINTERFACE);
      conf->cerebrod_heartbeat_network_interface_flag++;
    }

  return 0;
}

#if WITH_STATIC_MODULES
struct cerebro_config_module_info gendersllnl_config_module_info =
#else /* !WITH_STATIC_MODULES */
struct cerebro_config_module_info config_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    GENDERSLLNL_CONFIG_MODULE_NAME,
    &gendersllnl_config_setup,
    &gendersllnl_config_cleanup,
    &gendersllnl_config_load_default,
  };
