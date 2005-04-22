/*****************************************************************************\
 *  $Id: cerebro_clusterlist_none.c,v 1.3 2005-04-22 21:31:04 achu Exp $
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

#include "cerebro_error.h"
#include "cerebro_clusterlist_module.h"
#include "cerebro_clusterlist_util.h"
#include "wrappers.h"

#define NONE_CLUSTERLIST_MODULE_NAME "none"

/* 
 * none_clusterlist_parse_options
 *
 * parse options for the none clusterlist module
 */
static int
none_clusterlist_parse_options(char **options)
{
  int i = 0;

  if (!options)
    return 0;

  /* None module takes no options */
  while (options[i] != NULL)
    {
      err_exit("none clusterlist module: option '%s' unrecognized", 
               options[i]);
      i++;
    }

  return 0;
}

/* 
 * none_clusterlist_setup
 *
 * none clusterlist module setup function
 */
static int 
none_clusterlist_setup(void)
{
  return 0;
}

/* 
 * none_clusterlist_cleanup
 *
 * none clusterlist module cleanup function
 */
static int
none_clusterlist_cleanup(void)
{
  return 0;
}

/* 
 * none_clusterlist_get_all_nodes
 *
 * none clusterlist module get all nodes function
 */
static int
none_clusterlist_get_all_nodes(char **nodes, unsigned int nodeslen)
{
  return 0;
}

/* 
 * none_clusterlist_numnodes
 *
 * none clusterlist module numnodes function
 */
static int 
none_clusterlist_numnodes(void)
{
  return 0;
}

/* 
 * none_clusterlist_node_in_cluster
 *
 * none clusterlist module node in cluster function
 */
static int
none_clusterlist_node_in_cluster(char *node)
{
  /* Must assume it is */
  return 1;
}

/* 
 * none_clusterlist_get_nodename
 *
 * none clusterlist module get nodename function
 */
static int
none_clusterlist_get_nodename(char *node, char *buf, unsigned int buflen)
{
  assert(node);
  assert(buf);

  return cerebro_clusterlist_copy_nodename(node, 
                                           buf, 
                                           buflen, 
                                           NONE_CLUSTERLIST_MODULE_NAME);
}

#if WITH_STATIC_MODULES
struct cerebro_clusterlist_module_info none_clusterlist_module_info =
#else /* !WITH_STATIC_MODULES */
struct cerebro_clusterlist_module_info clusterlist_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    NONE_CLUSTERLIST_MODULE_NAME,
    &none_clusterlist_parse_options,
    &none_clusterlist_setup,
    &none_clusterlist_cleanup,
    &none_clusterlist_get_all_nodes,
    &none_clusterlist_numnodes,
    &none_clusterlist_node_in_cluster,
    &none_clusterlist_get_nodename,
  };
