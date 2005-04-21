/*****************************************************************************\
 *  $Id: cerebrod_config_gendersllnl.c,v 1.2 2005-04-21 17:59:15 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <gendersllnl.h>

#include "cerebro_defs.h"

#include "cerebrod_config_module.h"

#include "error.h"
#include "wrappers.h"

extern int h_errno;

/* 
 * gendersllnl_heartbeat_network_interface
 *
 * Store private management network ip address
 */
char gendersllnl_heartbeat_network_interface[INET_ADDRSTRLEN+1];

/* 
 * gendersllnl_config_load_default
 *
 * alter default module specifically for use on LLNL clusters 'mgmt'
 * nodes listen and speak, while compute nodes only speak.
 *
 * Returns 0 on success, -1 on error
 */
int
gendersllnl_config_load_default(struct cerebrod_module_config *conf)
{
  genders_t handle = NULL;
  char altnamebuf[CEREBRO_MAXNODENAMELEN+1];
  int ret;

  assert(conf);

  if (!(handle = genders_handle_create()))
    err_exit("%s(%s:%d): genders_handle_create", 
             __FILE__, __FUNCTION__, __LINE__);
 
  if (genders_load_data(handle, NULL) < 0)
    err_exit("%s(%s:%d): genders_load_data: %s", 
             __FILE__, __FUNCTION__, __LINE__,
             genders_errormsg(handle));

  if ((ret = genders_testattr(handle, NULL, "mgmt", NULL, 0)) < 0)
    err_exit("%s(%s:%d): genders_testattr: %s", 
             __FILE__, __FUNCTION__, __LINE__,
             genders_errormsg(handle));
    
  if (ret)
    {
      conf->speak = 1;
      conf->listen = 1;
      conf->updown_server = 1;
    }
  else
    {
      conf->speak = 1;
      conf->listen = 0;
      conf->updown_server = 0;
    }

  memset(altnamebuf, '\0', CEREBRO_MAXNODENAMELEN+1);
  if ((ret = genders_testattr(handle, 
                              NULL, 
                              GENDERS_ALTNAME_ATTRIBUTE,
                              altnamebuf,
                              CEREBRO_MAXNODENAMELEN)) < 0)
    err_exit("%s(%s:%d): genders_testattr: %s",
             __FILE__, __FUNCTION__, __LINE__,
             genders_errormsg(handle));

  if (ret)
    {
      struct hostent *h = NULL;
      struct in_addr in;

      h = Gethostbyname(altnamebuf);

      in = *((struct in_addr *)h->h_addr);

      memset(gendersllnl_heartbeat_network_interface, '\0', INET_ADDRSTRLEN+1);
      Inet_ntop(AF_INET, 
                &in, 
                gendersllnl_heartbeat_network_interface,
                INET_ADDRSTRLEN+1);

      conf->heartbeat_network_interface = gendersllnl_heartbeat_network_interface;
    }

  if (genders_handle_destroy(handle) < 0)
    err_exit("%s(%s:%d): genders_handle_destroy: %s",
             __FILE__, __FUNCTION__, __LINE__,
             genders_errormsg(handle));

  return 0;
}

#if WITH_STATIC_MODULES
struct cerebrod_config_module_info gendersllnl_config_module_info =
#else /* !WITH_STATIC_MODULES */
struct cerebrod_config_module_info config_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    "gendersllnl",
    &gendersllnl_config_load_default,
  };
