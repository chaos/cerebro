/*****************************************************************************\
 *  $Id: cerebrod_clusterlist_gendersllnl.c,v 1.14 2005-03-21 14:36:47 achu Exp $
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

#include "cerebrod.h"
#include "cerebrod_clusterlist.h"
#include "cerebrod_clusterlist_genders_util.h"
#include "cerebrod_clusterlist_util.h"
#include "error.h"
#include "wrappers.h"

/* 
 * handle
 *
 * genders handle
 */
genders_t handle = NULL;

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
  assert(!handle);

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
  assert(!handle);
  return cerebrod_clusterlist_genders_init(&handle, gendersllnl_file);
}

/* 
 * gendersllnl_clusterlist_finish
 *
 * gendersllnl clusterlist module finish function
 */
int
gendersllnl_clusterlist_finish(void)
{
  assert(handle);
                                                                                     
  return cerebrod_clusterlist_genders_finish(&handle, &gendersllnl_file);
}

/*
 * gendersllnl_clusterlist_get_all_nodes
 *
 * gendersllnl clusterlist module get all nodes function
 */
int
gendersllnl_clusterlist_get_all_nodes(char **nodes, unsigned int nodeslen)
{
  assert(handle);
  assert(nodes);
                                                                                     
  return cerebrod_clusterlist_genders_get_all_nodes(handle, nodes, nodeslen);
}

/*
 * gendersllnl_clusterlist_numnodes
 *
 * gendersllnl clusterlist module numnodes function
 */
int
gendersllnl_clusterlist_numnodes(void)
{
  assert(handle);
                                                                                     
  return cerebrod_clusterlist_genders_numnodes(handle);
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

  assert(handle);
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

  if ((ret = genders_isnode_or_altnode(handle, nodePtr)) < 0)
    err_exit("gendersllnl_clusterlist_node_in_cluster: genders_isnode: %s",
	     genders_errormsg(handle));

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
  int len, free_flag = 0;
  char *nodePtr = NULL;

  assert(handle);
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

  if (genders_to_gendname(handle, nodePtr, buf, buflen) < 0)
    err_exit("gendersllnl_clusterlist_get_nodename: genders_to_gendname: %s",
	     genders_errormsg(handle));

  if (free_flag)
    Free(nodePtr);

  return 0;
}

struct cerebrod_clusterlist_module_info clusterlist_module_info =
  {
    "gendersllnl",
  };

struct cerebrod_clusterlist_module_ops clusterlist_module_ops =
  {
    &gendersllnl_clusterlist_parse_options,
    &gendersllnl_clusterlist_init,
    &gendersllnl_clusterlist_finish,
    &gendersllnl_clusterlist_get_all_nodes,
    &gendersllnl_clusterlist_numnodes,
    &gendersllnl_clusterlist_node_in_cluster,
    &gendersllnl_clusterlist_get_nodename,
  };

