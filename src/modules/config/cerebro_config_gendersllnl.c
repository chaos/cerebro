/*****************************************************************************\
 *  $Id: cerebro_config_gendersllnl.c,v 1.16 2005-05-31 22:06:03 achu Exp $
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

extern int h_errno;

#define GENDERSLLNL_CONFIG_MODULE_NAME "gendersllnl"

#define GENDERSLLNL_LARGE_CLUSTER_SIZE 512
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
      cerebro_err_debug_module("%s(%s:%d): gendersllnl_handle non-null",
			       __FILE__, __FUNCTION__, __LINE__);
      return 0;
    }

  if (!(gendersllnl_handle = genders_handle_create()))
    {
      cerebro_err_debug_module("%s(%s:%d): genders_handle_create",
			       __FILE__, __FUNCTION__, __LINE__);
      goto cleanup;
    }
 
  if (genders_load_data(gendersllnl_handle, NULL) < 0)
    {
      if (genders_errnum(gendersllnl_handle) == GENDERS_ERR_OPEN)
        {
          cerebro_err_debug_module("genders database '%s' cannot be opened",
				   GENDERS_DEFAULT_FILE);
          goto cleanup;
        }
      else
        {
          cerebro_err_debug_module("%s(%s:%d): genders_load_data: %s",
				   __FILE__, __FUNCTION__, __LINE__,
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
      cerebro_err_debug_module("%s(%s:%d): gendersllnl_handle null",
			       __FILE__, __FUNCTION__, __LINE__);
      return 0;
    }

  if (genders_handle_destroy(gendersllnl_handle) < 0)
    {
      cerebro_err_debug_module("%s(%s:%d): genders_handle_destroy: %s",
			       __FILE__, __FUNCTION__, __LINE__,
			       genders_errormsg(gendersllnl_handle));
    }

  gendersllnl_handle = NULL;
  return 0;
}

/* 
 * gendersllnl_config_load_default
 *
 * config specifically for use on LLNL clusters. 'mgmt' nodes listen
 * and speak, while compute nodes only speak.  We always speak on the
 * private management network interface.  Non-mgmt nodes connect to
 * mgmt nodes to receive data.
 *
 * Returns 0 on success, -1 on error
 */
int
gendersllnl_config_load_default(struct cerebro_config *conf)
{
  char altnamebuf[CEREBRO_MAXNODENAMELEN+1];
  int flag, numnodes;

  if (!gendersllnl_handle)
    {
      cerebro_err_debug_module("%s(%s:%d): gendersllnl_handle null",
			       __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!conf)
    {
      cerebro_err_debug_module("%s(%s:%d): conf null",
			       __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if ((numnodes = genders_getnumnodes(gendersllnl_handle)) < 0)
    {
      cerebro_err_debug_module("%s(%s:%d): genders_numnodes: %s", 
			       __FILE__, __FUNCTION__, __LINE__,
			       genders_errormsg(gendersllnl_handle));
      return -1;
    }
                                   
  if ((flag = genders_testattr(gendersllnl_handle, 
			       NULL, 
			       "mgmt", 
			       NULL, 
			       0)) < 0)
    {
      cerebro_err_debug_module("%s(%s:%d): genders_testattr: %s", 
			       __FILE__, __FUNCTION__, __LINE__,
			       genders_errormsg(gendersllnl_handle));
      return -1;
    }
  
  if (flag)
    {
      conf->cerebrod_speak = 1;
      conf->cerebrod_speak_flag++;
      conf->cerebrod_listen = 1;
      conf->cerebrod_listen_flag++;
      if (numnodes >= GENDERSLLNL_LARGE_CLUSTER_SIZE)
        {
          conf->cerebrod_listen_threads = 4;
          conf->cerebrod_listen_threads_flag++;
        }
      conf->cerebrod_metric_server = 1;
      conf->cerebrod_metric_server_flag++;
    }
  else
    {
      char **nodelist = NULL;
      int i, nodelist_len, mgmt_len;

      conf->cerebrod_speak = 1;
      conf->cerebrod_speak_flag++;
      conf->cerebrod_listen = 0;
      conf->cerebrod_listen_flag++;
      conf->cerebrod_metric_server = 0;
      conf->cerebrod_metric_server_flag++;

      if ((nodelist_len = genders_nodelist_create(gendersllnl_handle, 
						  &nodelist)) < 0)
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

      if (mgmt_len > CEREBRO_CONFIG_HOSTNAMES_MAX)
	mgmt_len = CEREBRO_CONFIG_HOSTNAMES_MAX;

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
	  strcpy(conf->cerebro_hostnames[i], nodelist[i]);
	}
      conf->cerebro_hostnames_len = mgmt_len;
      conf->cerebro_hostnames_flag++;
      
      genders_nodelist_destroy(gendersllnl_handle, nodelist);      
    }

  memset(altnamebuf, '\0', CEREBRO_MAXNODENAMELEN+1);
  if ((flag = genders_testattr(gendersllnl_handle, 
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

  if (flag)
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
      
      memset(conf->cerebrod_heartbeat_network_interface, 
	     '\0', 
	     CEREBRO_MAXNETWORKINTERFACELEN+1);
      strncpy(conf->cerebrod_heartbeat_network_interface, 
	      heartbeat_network_interface, 
	      CEREBRO_MAXNETWORKINTERFACELEN);
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
