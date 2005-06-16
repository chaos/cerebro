/*****************************************************************************\
 *  $Id: cerebro_clusterlist_genders.c,v 1.21 2005-06-16 21:35:34 achu Exp $
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

#include <genders.h>

#include "cerebro/cerebro_clusterlist_module.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_error.h"

#include "cerebro_clusterlist_genders_util.h"
#include "cerebro_clusterlist_util.h"

#define GENDERS_CLUSTERLIST_MODULE_NAME "genders"

/* 
 * genders_handle
 *
 * genders handle
 */
static genders_t genders_handle = NULL;

/* 
 * genders_clusterlist_setup
 *
 * genders clusterlist module setup function
 */
static int 
genders_clusterlist_setup(void)
{
  if (genders_handle)
    {
      cerebro_err_debug("%s(%s:%d): genders_handle non-null",
			__FILE__, __FUNCTION__, __LINE__);
      return 0;
    }

  return cerebro_clusterlist_genders_setup(&genders_handle);
}

/* 
 * genders_clusterlist_cleanup
 *
 * genders clusterlist module cleanup function
 */
static int
genders_clusterlist_cleanup(void)
{
  if (!genders_handle)
    {
      cerebro_err_debug("%s(%s:%d): genders_handle null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  return cerebro_clusterlist_genders_cleanup(&genders_handle);
}

/*
 * genders_clusterlist_numnodes
 *
 * genders clusterlist module numnodes function
 */
static int 
genders_clusterlist_numnodes(void)
{
  if (!genders_handle)
    {
      cerebro_err_debug("%s(%s:%d): genders_handle null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  return cerebro_clusterlist_genders_numnodes(genders_handle);
}

/*
 * genders_clusterlist_get_all_nodes
 *
 * genders clusterlist module get all nodes function
 */
static int
genders_clusterlist_get_all_nodes(char ***nodes)
{
  if (!genders_handle)
    {
      cerebro_err_debug("%s(%s:%d): genders_handle null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!nodes)
    {
      cerebro_err_debug("%s(%s:%d): nodes null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  return cerebro_clusterlist_genders_get_all_nodes(genders_handle, 
                                                   nodes);
}

/*
 * genders_clusterlist_node_in_cluster
 *
 * genders clusterlist module node in cluster function
 */
static int
genders_clusterlist_node_in_cluster(const char *node)
{
  char nodebuf[CEREBRO_MAXNODENAMELEN+1];
  char *nodePtr = NULL;
  int flag;

  if (!genders_handle)
    {
      cerebro_err_debug("%s(%s:%d): genders_handle null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!node)
    {
      cerebro_err_debug("%s(%s:%d): node null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

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
    nodePtr = (char *)node;

  if ((flag = genders_isnode(genders_handle, nodePtr)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): genders_isnode: %s",
			__FILE__, __FUNCTION__, __LINE__,
			genders_errormsg(genders_handle));
      return -1;
    }

  return flag;
}

/*
 * genders_clusterlist_get_nodename
 *
 * genders clusterlist module get nodename function
 */
static int
genders_clusterlist_get_nodename(const char *node, 
				 char *buf, 
				 unsigned int buflen)
{
  char nodebuf[CEREBRO_MAXNODENAMELEN+1];
  char *nodePtr = NULL;

  if (!genders_handle)
    {
      cerebro_err_debug("%s(%s:%d): genders_handle null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!node)
    {
      cerebro_err_debug("%s(%s:%d): node null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!buf)
    {
      cerebro_err_debug("%s(%s:%d): buf invalid",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!buflen)
    {
      cerebro_err_debug("%s(%s:%d): buflen invalid",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
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
    nodePtr = (char *)node;

  return cerebro_clusterlist_copy_nodename(nodePtr, 
                                           buf, 
                                           buflen);
}

struct cerebro_clusterlist_module_info clusterlist_module_info =
  {
    GENDERS_CLUSTERLIST_MODULE_NAME,
    &genders_clusterlist_setup,
    &genders_clusterlist_cleanup,
    &genders_clusterlist_numnodes,
    &genders_clusterlist_get_all_nodes,
    &genders_clusterlist_node_in_cluster,
    &genders_clusterlist_get_nodename,
  };
