/*****************************************************************************\
 *  $Id: clusterlist_module_default.c,v 1.3 2005-06-26 18:39:13 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */

#include "cerebro/cerebro_clusterlist_module.h"

#include "debug.h"

#define DEFAULT_CLUSTERLIST_MODULE_NAME "default"

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
default_clusterlist_numnodes(void)
{
  return 0;
}

int
default_clusterlist_get_all_nodes(char ***nodes)
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
      CEREBRO_ERR_DEBUG(("node null"));
      return -1;
    }

  if (!buf)
    {
      CEREBRO_ERR_DEBUG(("buf null"));
      return -1;
    }

  if (!buflen)
    {
      CEREBRO_ERR_DEBUG(("buflen invalid"));
      return -1;
    }
  
  len = strlen(node);
 
  if ((len + 1) > buflen)
    {
      CEREBRO_ERR_DEBUG(("buflen too small: %d %d", len, buflen));
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
    &default_clusterlist_numnodes,
    &default_clusterlist_get_all_nodes,
    &default_clusterlist_node_in_cluster,
    &default_clusterlist_get_nodename,
  };
