/*****************************************************************************\
 *  $Id: cerebrod_clusterlist_gendersllnl.c,v 1.19 2005-03-30 05:41:45 achu Exp $
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

#include <gendersllnl.h>

#include "cerebrod_clusterlist_module.h"

#include "cerebrod_clusterlist.h"
#include "cerebrod_clusterlist_genders_util.h"
#include "cerebrod_clusterlist_util.h"
#include "cerebrod_error.h"
#include "wrappers.h"

/* 
 * gendersllnl_handle
 *
 * genders handle
 */
genders_t gendersllnl_handle = NULL;

/*  
 * gendersllnl_file
 *
 * gendersllnl database
 */
char *gendersllnl_file = NULL;

/* 
 * gendersllnl_clusterlist_parse_options
 *
 * parse options for the gendersllnl clusterlist module
 */
int
gendersllnl_clusterlist_parse_options(char **options)
{
  assert(!gendersllnl_handle);

  if (options)
    cerebrod_clusterlist_parse_filename(options, &gendersllnl_file);

  return 0;
}

/* 
 * gendersllnl_clusterlist_init
 *
 * gendersllnl clusterlist module init function
 */
int
gendersllnl_clusterlist_init(void)
{
  assert(!gendersllnl_handle);

  return cerebrod_clusterlist_genders_init(&gendersllnl_handle, 
					   gendersllnl_file);
}

/* 
 * gendersllnl_clusterlist_finish
 *
 * gendersllnl clusterlist module finish function
 */
int
gendersllnl_clusterlist_finish(void)
{
  assert(gendersllnl_handle);

  return cerebrod_clusterlist_genders_finish(&gendersllnl_handle, 
					     &gendersllnl_file);
}

/*
 * gendersllnl_clusterlist_get_all_nodes
 *
 * gendersllnl clusterlist module get all nodes function
 */
int
gendersllnl_clusterlist_get_all_nodes(char **nodes, unsigned int nodeslen)
{
  assert(gendersllnl_handle);
  assert(nodes);

  return cerebrod_clusterlist_genders_get_all_nodes(gendersllnl_handle, 
						    nodes, 
						    nodeslen);
}

/*
 * gendersllnl_clusterlist_numnodes
 *
 * gendersllnl clusterlist module numnodes function
 */
int
gendersllnl_clusterlist_numnodes(void)
{
  assert(gendersllnl_handle);

  return cerebrod_clusterlist_genders_numnodes(gendersllnl_handle);
}

/*
 * gendersllnl_clusterlist_node_in_cluster
 *
 * gendersllnl clusterlist module node in cluster function
 */
int
gendersllnl_clusterlist_node_in_cluster(char *node)
{
  int ret, free_flag = 0;
  char *nodePtr = NULL;
  char *clusterlist_module_name = cerebrod_clusterlist_module_name();

  assert(gendersllnl_handle);
  assert(node);

  /* Shorten hostname if necessary */
  if (strchr(node, '.'))
    {
      char *p;

      nodePtr = Strdup(node);
      p = strchr(nodePtr, '.');
      *p = '\0';
      free_flag++;
    }
  else
    nodePtr = node;

  if ((ret = genders_isnode_or_altnode(gendersllnl_handle, nodePtr)) < 0)
    cerebrod_err_exit("%s(%s:%d): %s clusterlist module: genders_isnode: %s",
		      __FILE__, __FUNCTION__, __LINE__, 
		      clusterlist_module_name,
		      genders_errormsg(gendersllnl_handle));

  if (free_flag)
    Free(nodePtr);

  return ret;
}

/*
 * gendersllnl_clusterlist_get_nodename
 *
 * gendersllnl clusterlist module get nodename function
 */
int
gendersllnl_clusterlist_get_nodename(char *node, char *buf, unsigned int buflen)
{
  int free_flag = 0;
  char *nodePtr = NULL;
  char *clusterlist_module_name = cerebrod_clusterlist_module_name();

  assert(gendersllnl_handle);
  assert(node);
  assert(buf);

  /* Shorten hostname if necessary */
  if (strchr(node, '.'))
    {
      char *p;

      nodePtr = Strdup(node);
      p = strchr(nodePtr, '.');
      *p = '\0';
      free_flag++;
    }
  else
    nodePtr = node;

  if (genders_to_gendname(gendersllnl_handle, nodePtr, buf, buflen) < 0)
    cerebrod_err_exit("%s(%s:%d): %s clusterlist module: "
		      "genders_to_gendname: %s",
		      __FILE__, __FUNCTION__, __LINE__,
		      clusterlist_module_name, 
		      genders_errormsg(gendersllnl_handle));

  if (free_flag)
    Free(nodePtr);

  return 0;
}

#if WITH_STATIC_MODULES
struct cerebrod_clusterlist_module_info gendersllnl_clusterlist_module_info =
#else
struct cerebrod_clusterlist_module_info clusterlist_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    "gendersllnl",
    &gendersllnl_clusterlist_parse_options,
    &gendersllnl_clusterlist_init,
    &gendersllnl_clusterlist_finish,
    &gendersllnl_clusterlist_get_all_nodes,
    &gendersllnl_clusterlist_numnodes,
    &gendersllnl_clusterlist_node_in_cluster,
    &gendersllnl_clusterlist_get_nodename,
  };
