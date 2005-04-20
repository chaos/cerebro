/*****************************************************************************\
 *  $Id: cerebrod_clusterlist_genders.c,v 1.21 2005-04-20 19:43:22 achu Exp $
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

#include <genders.h>

#include "cerebro_defs.h"
#include "cerebrod_clusterlist_module.h"

#include "cerebrod_clusterlist.h"
#include "cerebrod_clusterlist_genders_util.h"
#include "cerebrod_clusterlist_util.h"
#include "cerebrod_error.h"
#include "wrappers.h"

#define GENDERS_CLUSTERLIST_MODULE_NAME "genders"

/* 
 * genders_handle
 *
 * genders handle
 */
genders_t genders_handle = NULL;

/*  
 * genders_file
 *
 * genders database
 */
char *genders_file = NULL;

/* 
 * genders_clusterlist_parse_options
 *
 * parse options for the genders clusterlist module
 */
int
genders_clusterlist_parse_options(char **options)
{
  assert(!genders_handle);

  if (options)
    cerebrod_clusterlist_parse_filename(options,
                                        &genders_file,
                                        GENDERS_CLUSTERLIST_MODULE_NAME);

  return 0;
}

/* 
 * genders_clusterlist_init
 *
 * genders clusterlist module init function
 */
int 
genders_clusterlist_init(void)
{
  assert(!genders_handle);

  return cerebrod_clusterlist_genders_init(&genders_handle, 
                                           genders_file,
                                           GENDERS_CLUSTERLIST_MODULE_NAME);
}

/* 
 * genders_clusterlist_finish
 *
 * genders clusterlist module finish function
 */
int
genders_clusterlist_finish(void)
{
  assert(genders_handle);

  return cerebrod_clusterlist_genders_finish(&genders_handle, 
                                             &genders_file,
                                             GENDERS_CLUSTERLIST_MODULE_NAME);
}

/*
 * genders_clusterlist_get_all_nodes
 *
 * genders clusterlist module get all nodes function
 */
int
genders_clusterlist_get_all_nodes(char **nodes, unsigned int nodeslen)
{
  assert(genders_handle);
  assert(nodes);
  
  return cerebrod_clusterlist_genders_get_all_nodes(genders_handle, 
                                                    nodes,
                                                    nodeslen,
                                                    GENDERS_CLUSTERLIST_MODULE_NAME);
}

/*
 * genders_clusterlist_numnodes
 *
 * genders clusterlist module numnodes function
 */
int 
genders_clusterlist_numnodes(void)
{
  assert(genders_handle);

  return cerebrod_clusterlist_genders_numnodes(genders_handle,
                                               GENDERS_CLUSTERLIST_MODULE_NAME);
}

/*
 * genders_clusterlist_node_in_cluster
 *
 * genders clusterlist module node in cluster function
 */
int
genders_clusterlist_node_in_cluster(char *node)
{
  int ret;
  char *nodePtr = NULL;
  char nodebuf[CEREBRO_MAXNODENAMELEN+1];

  assert(genders_handle);
  assert(node);

  /* Shorten hostname if necessary */
  if (strchr(node, '.'))
    {
      char *p;

      memset(nodebuf, '\0', CEREBRO_MAXNODENAMELEN+1);
      strncpy(nodebuf, node, CEREBRO_MAXNODENAMELEN);
      p = strchr(nodebuf, '.');
      *p = '\0';
      nodePtr = nodebuf;
    }
  else
    nodePtr = node;

  if ((ret = genders_isnode(genders_handle, nodePtr)) < 0)
    cerebrod_err_exit("%s(%s:%d): %s clusterlist module: genders_isnode: %s",
		      __FILE__, __FUNCTION__, __LINE__,
                      GENDERS_CLUSTERLIST_MODULE_NAME, 
		      genders_errormsg(genders_handle));

  return ret;
}

/*
 * genders_clusterlist_get_nodename
 *
 * genders clusterlist module get nodename function
 */
int
genders_clusterlist_get_nodename(char *node, char *buf, unsigned int buflen)
{
  assert(genders_handle);
  assert(node);
  assert(buf);

  return cerebrod_clusterlist_copy_nodename(node, buf, buflen, GENDERS_CLUSTERLIST_MODULE_NAME);
}

#if WITH_STATIC_MODULES
struct cerebrod_clusterlist_module_info genders_clusterlist_module_info =
#else /* !WITH_STATIC_MODULES */
struct cerebrod_clusterlist_module_info clusterlist_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    GENDERS_CLUSTERLIST_MODULE_NAME,
    &genders_clusterlist_parse_options,
    &genders_clusterlist_init,
    &genders_clusterlist_finish,
    &genders_clusterlist_get_all_nodes,
    &genders_clusterlist_numnodes,
    &genders_clusterlist_node_in_cluster,
    &genders_clusterlist_get_nodename,
  };
