/*****************************************************************************\
 *  $Id: cerebro_clusterlist_genders.c,v 1.6 2005-04-29 06:33:38 achu Exp $
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

#include "module.h"
#include "cerebro_defs.h"
#include "cerebro_error.h"
#include "cerebro_clusterlist_module.h"
#include "cerebro_clusterlist_genders_util.h"
#include "cerebro_clusterlist_util.h"
#include "wrappers.h"

#define GENDERS_CLUSTERLIST_MODULE_NAME "genders"

/* 
 * genders_handle
 *
 * genders handle
 */
static genders_t genders_handle = NULL;

/*  
 * genders_file
 *
 * genders database
 */
static char *genders_file = NULL;

/* 
 * genders_clusterlist_parse_options
 *
 * parse options for the genders clusterlist module
 */
static int
genders_clusterlist_parse_options(char **options)
{
  if (genders_handle)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "genders_handle non-null",
                        __FILE__, __FUNCTION__, __LINE__,
                        GENDERS_CLUSTERLIST_MODULE_NAME);
      return -1;
    }

  if (options)
    return module_parse_filename(options,
				 &genders_file,
				 GENDERS_CLUSTERLIST_MODULE_NAME);

  return 0;
}

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
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "genders_handle non-null",
                        __FILE__, __FUNCTION__, __LINE__,
                        GENDERS_CLUSTERLIST_MODULE_NAME);
      return 0;
    }

  return cerebro_clusterlist_genders_setup(&genders_handle, 
                                           genders_file,
                                           GENDERS_CLUSTERLIST_MODULE_NAME);
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
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "genders_handle null",
                        __FILE__, __FUNCTION__, __LINE__,
                        GENDERS_CLUSTERLIST_MODULE_NAME);
      return -1;
    }

  return cerebro_clusterlist_genders_cleanup(&genders_handle, 
                                             &genders_file,
                                             GENDERS_CLUSTERLIST_MODULE_NAME);
}

/*
 * genders_clusterlist_get_all_nodes
 *
 * genders clusterlist module get all nodes function
 */
static int
genders_clusterlist_get_all_nodes(char **nodes, unsigned int nodeslen)
{
  if (!genders_handle)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "genders_handle null",
                        __FILE__, __FUNCTION__, __LINE__,
                        GENDERS_CLUSTERLIST_MODULE_NAME);
      return -1;
    }

  if (!nodes)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "invalid nodes parameter",
                        GENDERS_CLUSTERLIST_MODULE_NAME,
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  return cerebro_clusterlist_genders_get_all_nodes(genders_handle, 
                                                   nodes,
                                                   nodeslen,
                                                   GENDERS_CLUSTERLIST_MODULE_NAME);
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
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "genders_handle null",
                        __FILE__, __FUNCTION__, __LINE__,
                        GENDERS_CLUSTERLIST_MODULE_NAME);
      return -1;
    }

  return cerebro_clusterlist_genders_numnodes(genders_handle,
                                              GENDERS_CLUSTERLIST_MODULE_NAME);
}

/*
 * genders_clusterlist_node_in_cluster
 *
 * genders clusterlist module node in cluster function
 */
static int
genders_clusterlist_node_in_cluster(char *node)
{
  char nodebuf[CEREBRO_MAXNODENAMELEN+1];
  char *nodePtr = NULL;
  int ret;

  if (!genders_handle)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "genders_handle null",
                        __FILE__, __FUNCTION__, __LINE__,
                        GENDERS_CLUSTERLIST_MODULE_NAME);
      return -1;
    }

  if (!node)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "invalid node parameter",
                        GENDERS_CLUSTERLIST_MODULE_NAME,
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
    nodePtr = node;

  if ((ret = genders_isnode(genders_handle, nodePtr)) < 0)
    cerebro_err_exit("%s(%s:%d): %s clusterlist module: genders_isnode: %s",
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
static int
genders_clusterlist_get_nodename(char *node, char *buf, unsigned int buflen)
{
  char nodebuf[CEREBRO_MAXNODENAMELEN+1];
  char *nodePtr = NULL;

  if (!genders_handle)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "genders_handle null",
                        __FILE__, __FUNCTION__, __LINE__,
                        GENDERS_CLUSTERLIST_MODULE_NAME);
      return -1;
    }

  if (!node)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "invalid node parameter",
                        GENDERS_CLUSTERLIST_MODULE_NAME,
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
                                                                                        
  if (!buf)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "invalid buf parameter",
                        GENDERS_CLUSTERLIST_MODULE_NAME,
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
    nodePtr = node;

  return cerebro_clusterlist_copy_nodename(nodePtr, 
                                           buf, 
                                           buflen, 
                                           GENDERS_CLUSTERLIST_MODULE_NAME);
}

#if WITH_STATIC_MODULES
struct cerebro_clusterlist_module_info genders_clusterlist_module_info =
#else /* !WITH_STATIC_MODULES */
struct cerebro_clusterlist_module_info clusterlist_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    GENDERS_CLUSTERLIST_MODULE_NAME,
    &genders_clusterlist_parse_options,
    &genders_clusterlist_setup,
    &genders_clusterlist_cleanup,
    &genders_clusterlist_get_all_nodes,
    &genders_clusterlist_numnodes,
    &genders_clusterlist_node_in_cluster,
    &genders_clusterlist_get_nodename,
  };
