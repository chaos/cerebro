/*****************************************************************************\
 *  $Id: cerebro_clusterlist_gendersllnl.c,v 1.7 2005-04-29 18:59:26 achu Exp $
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

#include "module.h"
#include "cerebro_defs.h"
#include "cerebro_error.h"
#include "cerebro_clusterlist_module.h"
#include "cerebro_clusterlist_genders_util.h"
#include "cerebro_clusterlist_util.h"
#include "wrappers.h"

#define GENDERSLLNL_CLUSTERLIST_MODULE_NAME "gendersllnl"

/* 
 * gendersllnl_handle
 *
 * genders handle
 */
static genders_t gendersllnl_handle = NULL;

/*  
 * gendersllnl_file
 *
 * gendersllnl database
 */
static char *gendersllnl_file = NULL;

/* 
 * gendersllnl_clusterlist_parse_options
 *
 * parse options for the gendersllnl clusterlist module
 */
static int
gendersllnl_clusterlist_parse_options(char **options)
{
  if (gendersllnl_handle)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "gendersllnl_handle non-null",
                        __FILE__, __FUNCTION__, __LINE__,
                        GENDERSLLNL_CLUSTERLIST_MODULE_NAME);
      return -1;
    }

  if (options)
    return module_parse_filename(options, 
				 &gendersllnl_file, 
				 GENDERSLLNL_CLUSTERLIST_MODULE_NAME);

  return 0;
}

/* 
 * gendersllnl_clusterlist_setup
 *
 * gendersllnl clusterlist module setup function
 */
static int
gendersllnl_clusterlist_setup(void)
{
  int rv;

  if (gendersllnl_handle)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "gendersllnl_handle non-null",
                        __FILE__, __FUNCTION__, __LINE__,
                        GENDERSLLNL_CLUSTERLIST_MODULE_NAME);
      return 0;
    }

  rv = cerebro_clusterlist_genders_setup(&gendersllnl_handle, 
                                         gendersllnl_file,
                                         GENDERSLLNL_CLUSTERLIST_MODULE_NAME);

#if HAVE_GENDERS_INDEX_ATTRVALS
  if (!rv)
    {
      /* This is for performance improvements if the indexing API
       * functions are available.  Don't fail and return -1, since the
       * rest is not dependent on this section of code.
       */
      genders_index_attrvals(gendersllnl_handle, GENDERS_ALTNAME_ATTRIBUTE);
    }
#endif /* HAVE_GENDERS_INDEX_ATTRVALS */

  return rv;
}

/* 
 * gendersllnl_clusterlist_cleanup
 *
 * gendersllnl clusterlist module cleanup function
 */
static int
gendersllnl_clusterlist_cleanup(void)
{
  if (!gendersllnl_handle)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "gendersllnl_handle null",
                        __FILE__, __FUNCTION__, __LINE__,
                        GENDERSLLNL_CLUSTERLIST_MODULE_NAME);
      return -1;
    }

  return cerebro_clusterlist_genders_cleanup(&gendersllnl_handle, 
                                             &gendersllnl_file,
                                             GENDERSLLNL_CLUSTERLIST_MODULE_NAME);
}

/*
 * gendersllnl_clusterlist_get_all_nodes
 *
 * gendersllnl clusterlist module get all nodes function
 */
static int
gendersllnl_clusterlist_get_all_nodes(char **nodes, unsigned int nodeslen)
{
  if (!gendersllnl_handle)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "gendersllnl_handle null",
                        __FILE__, __FUNCTION__, __LINE__,
                        GENDERSLLNL_CLUSTERLIST_MODULE_NAME);
      return -1;
    }
                                                                                        
  if (!nodes)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "invalid nodes parameter",
                        GENDERSLLNL_CLUSTERLIST_MODULE_NAME,
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  return cerebro_clusterlist_genders_get_all_nodes(gendersllnl_handle, 
                                                   nodes, 
                                                   nodeslen,
                                                   GENDERSLLNL_CLUSTERLIST_MODULE_NAME);
}

/*
 * gendersllnl_clusterlist_numnodes
 *
 * gendersllnl clusterlist module numnodes function
 */
static int
gendersllnl_clusterlist_numnodes(void)
{
  if (!gendersllnl_handle)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "gendersllnl_handle null",
                        __FILE__, __FUNCTION__, __LINE__,
                        GENDERSLLNL_CLUSTERLIST_MODULE_NAME);
      return -1;
    }

  return cerebro_clusterlist_genders_numnodes(gendersllnl_handle,
                                              GENDERSLLNL_CLUSTERLIST_MODULE_NAME);
}

/*
 * gendersllnl_clusterlist_node_in_cluster
 *
 * gendersllnl clusterlist module node in cluster function
 */
static int
gendersllnl_clusterlist_node_in_cluster(char *node)
{
  char nodebuf[CEREBRO_MAXNODENAMELEN+1];
  char *nodePtr = NULL;
  int ret;

  if (!gendersllnl_handle)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "gendersllnl_handle null",
                        __FILE__, __FUNCTION__, __LINE__,
                        GENDERSLLNL_CLUSTERLIST_MODULE_NAME);
      return -1;
    }
                                                                                        
  if (!node)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "invalid node parameter",
                        GENDERSLLNL_CLUSTERLIST_MODULE_NAME,
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

  if ((ret = genders_isnode_or_altnode(gendersllnl_handle, nodePtr)) < 0)
    cerebro_err_exit("%s(%s:%d): %s clusterlist module: genders_isnode: %s",
                     __FILE__, __FUNCTION__, __LINE__, 
                     GENDERSLLNL_CLUSTERLIST_MODULE_NAME,
                     genders_errormsg(gendersllnl_handle));
  
  return ret;
}

/*
 * gendersllnl_clusterlist_get_nodename
 *
 * gendersllnl clusterlist module get nodename function
 */
static int
gendersllnl_clusterlist_get_nodename(char *node, char *buf, unsigned int buflen)
{
  char nodebuf[CEREBRO_MAXNODENAMELEN+1];
  char *nodePtr = NULL;

  if (!gendersllnl_handle)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "gendersllnl_handle null",
                        __FILE__, __FUNCTION__, __LINE__,
                        GENDERSLLNL_CLUSTERLIST_MODULE_NAME);
      return -1;
    }
                                                                                        
  if (!node)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "invalid node parameter",
                        GENDERSLLNL_CLUSTERLIST_MODULE_NAME,
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
                                                                                        
  if (!buf)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "invalid buf parameter",
                        GENDERSLLNL_CLUSTERLIST_MODULE_NAME,
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!(buflen > 0))
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "invalid buflen parameter",
                        GENDERSLLNL_CLUSTERLIST_MODULE_NAME,
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

  if (genders_to_gendname(gendersllnl_handle, nodePtr, buf, buflen) < 0)
    cerebro_err_exit("%s(%s:%d): %s clusterlist module: "
                     "genders_to_gendname: %s",
                     __FILE__, __FUNCTION__, __LINE__,
                     GENDERSLLNL_CLUSTERLIST_MODULE_NAME, 
                     genders_errormsg(gendersllnl_handle));
  return 0;
}

#if WITH_STATIC_MODULES
struct cerebro_clusterlist_module_info gendersllnl_clusterlist_module_info =
#else /* !WITH_STATIC_MODULES */
struct cerebro_clusterlist_module_info clusterlist_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    GENDERSLLNL_CLUSTERLIST_MODULE_NAME,
    &gendersllnl_clusterlist_parse_options,
    &gendersllnl_clusterlist_setup,
    &gendersllnl_clusterlist_cleanup,
    &gendersllnl_clusterlist_get_all_nodes,
    &gendersllnl_clusterlist_numnodes,
    &gendersllnl_clusterlist_node_in_cluster,
    &gendersllnl_clusterlist_get_nodename,
  };
