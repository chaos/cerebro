/*****************************************************************************\
 *  $Id: cerebro_module_default.c,v 1.6 2005-05-05 16:12:57 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */

#include "cerebro/cerebro_config_module.h"
#include "cerebro/cerebro_clusterlist_module.h"
#include "cerebro/cerebro_error.h"

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
      cerebro_err_debug_module("%s(%s:%d): %s clusterlist module: "
			       "node null",
			       DEFAULT_CLUSTERLIST_MODULE_NAME,
			       __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!buf)
    {
      cerebro_err_debug_module("%s(%s:%d): %s clusterlist module: "
			       "buf null",
			       DEFAULT_CLUSTERLIST_MODULE_NAME,
			       __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!buflen)
    {
      cerebro_err_debug_module("%s(%s:%d): %s clusterlist module: "
			       "buflen invalid",
			       DEFAULT_CLUSTERLIST_MODULE_NAME,
			       __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  len = strlen(node);
 
  if ((len + 1) > buflen)
    {
      cerebro_err_debug_module("%s(%s:%d): %s clusterlist module: "
			       "buflen too small: %d %d",
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
default_config_load_default(struct cerebro_config *conf)
{
  return 0;
}

struct cerebro_config_module_info default_config_module_info =
  {
    DEFAULT_CONFIG_MODULE_NAME,
    &default_config_setup,
    &default_config_cleanup,
    &default_config_load_default,
  };
