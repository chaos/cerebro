/*****************************************************************************\
 *  $Id: cerebro_module_default.c,v 1.1 2005-05-01 16:49:59 achu Exp $
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
#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <dirent.h>

#include "cerebro.h"
#include "cerebro_defs.h"
#include "cerebro_error.h"
#include "cerebro_module.h"
#include "ltdl.h"

#define DEFAULT_CLUSTERLIST_MODULE_NAME "default"
#define DEFAULT_CONFIG_MODULE_NAME "default"

int
default_clusterlist_setup(void)
{
  return 0;
}

int
default_clusterlist_cleanup(void)
{
  return 0;
}
int
default_clusterlist_get_all_nodes(char **nodes, unsigned int nodeslen)
{
  return 0;
}

int
default_clusterlist_numnodes(void)
{
  return 0;
}

int
default_clusterlist_node_in_cluster(const char *node)
{
  /* Must assume it is in the cluster */
  return 1;
}

int
default_clusterlist_get_nodename(const char *node, char *buf, unsigned int buflen)
{
  int len;

  if (!node)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "invalid node parameter",
                        DEFAULT_CLUSTERLIST_MODULE_NAME,
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!buf)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "invalid buf parameter",
                        DEFAULT_CLUSTERLIST_MODULE_NAME,
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!(buflen > 0))
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "invalid buflen parameter",
                        DEFAULT_CLUSTERLIST_MODULE_NAME,
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  len = strlen(node);
 
  if ((len + 1) > buflen)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: buflen too small: %d %d",
                        __FILE__, __FUNCTION__, __LINE__,
                        DEFAULT_CLUSTERLIST_MODULE_NAME, len, buflen);
      return -1;
    }
 
  strcpy(buf, node);

  return 0;
}
                                                                                     
struct cerebro_clusterlist_module_info default_clusterlist_module_info =
  {
    DEFAULT_CLUSTERLIST_MODULE_NAME,
    &default_clusterlist_setup,
    &default_clusterlist_cleanup,
    &default_clusterlist_get_all_nodes,
    &default_clusterlist_numnodes,
    &default_clusterlist_node_in_cluster,
    &default_clusterlist_get_nodename,
  };

int
default_config_setup(void)
{
  return 0;
}

int
default_config_cleanup(void)
{
  return 0;
}

int
default_config_load_cerebrod_default(struct cerebrod_module_config *conf)
{
  return 0;
}

struct cerebro_config_module_info default_config_module_info =
  {
    DEFAULT_CONFIG_MODULE_NAME,
    &default_config_setup,
    &default_config_cleanup,
    &default_config_load_cerebrod_default,
  };
