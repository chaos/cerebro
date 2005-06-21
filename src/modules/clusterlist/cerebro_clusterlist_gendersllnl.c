/*****************************************************************************\
 *  $Id: cerebro_clusterlist_gendersllnl.c,v 1.22 2005-06-21 20:56:28 achu Exp $
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

#include <gendersllnl.h>

#include "cerebro/cerebro_clusterlist_module.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_error.h"

#include "cerebro_clusterlist_genders_util.h"
#include "cerebro_clusterlist_util.h"

#define GENDERSLLNL_CLUSTERLIST_MODULE_NAME "gendersllnl"

/* 
 * gendersllnl_handle
 *
 * genders handle
 */
static genders_t gendersllnl_handle = NULL;

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
      cerebro_err_debug("%s(%s:%d): gendersllnl_handle non-null",
			__FILE__, __FUNCTION__, __LINE__);
      return 0;
    }

  rv = cerebro_clusterlist_genders_setup(&gendersllnl_handle);

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
    return 0;

  return cerebro_clusterlist_genders_cleanup(&gendersllnl_handle);
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
      cerebro_err_debug("%s(%s:%d): gendersllnl_handle null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  return cerebro_clusterlist_genders_numnodes(gendersllnl_handle);
}

/*
 * gendersllnl_clusterlist_get_all_nodes
 *
 * gendersllnl clusterlist module get all nodes function
 */
static int
gendersllnl_clusterlist_get_all_nodes(char ***nodes)
{
  if (!gendersllnl_handle)
    {
      cerebro_err_debug("%s(%s:%d): gendersllnl_handle null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!nodes)
    {
      cerebro_err_debug("%s(%s:%d): nodes null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  return cerebro_clusterlist_genders_get_all_nodes(gendersllnl_handle, 
                                                   nodes);
}

/*
 * gendersllnl_clusterlist_node_in_cluster
 *
 * gendersllnl clusterlist module node in cluster function
 */
static int
gendersllnl_clusterlist_node_in_cluster(const char *node)
{
  char nodebuf[CEREBRO_MAXNODENAMELEN+1];
  char *nodePtr = NULL;
  int flag;

  if (!gendersllnl_handle)
    {
      cerebro_err_debug("%s(%s:%d): gendersllnl_handle null",
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

  if ((flag = genders_isnode_or_altnode(gendersllnl_handle, nodePtr)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): genders_isnode_or_altnode: %s",
			__FILE__, __FUNCTION__, __LINE__, 
			genders_errormsg(gendersllnl_handle));
      return -1;
    }
  
  return flag;
}

/*
 * gendersllnl_clusterlist_get_nodename
 *
 * gendersllnl clusterlist module get nodename function
 */
static int
gendersllnl_clusterlist_get_nodename(const char *node, 
				     char *buf, 
				     unsigned int buflen)
{
  char nodebuf[CEREBRO_MAXNODENAMELEN+1];
  char *nodePtr = NULL;
  int rv;

  if (!gendersllnl_handle)
    {
      cerebro_err_debug("%s(%s:%d): gendersllnl_handle null",
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
      cerebro_err_debug("%s(%s:%d): buf null",
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

  if ((rv = gendersllnl_clusterlist_node_in_cluster(nodePtr)) < 0)
      return -1;

  if (!rv)
    {
      return cerebro_clusterlist_copy_nodename(nodePtr, 
                                               buf, 
                                               buflen);
    }
  else
    {
      if (genders_to_gendname(gendersllnl_handle, nodePtr, buf, buflen) < 0)
	{
	  cerebro_err_debug("%s(%s:%d): genders_to_gendname: %s",
			    __FILE__, __FUNCTION__, __LINE__,
			    genders_errormsg(gendersllnl_handle));
	  return -1;
	}
    }

  return 0;
}

struct cerebro_clusterlist_module_info clusterlist_module_info =
  {
    GENDERSLLNL_CLUSTERLIST_MODULE_NAME,
    &gendersllnl_clusterlist_setup,
    &gendersllnl_clusterlist_cleanup,
    &gendersllnl_clusterlist_numnodes,
    &gendersllnl_clusterlist_get_all_nodes,
    &gendersllnl_clusterlist_node_in_cluster,
    &gendersllnl_clusterlist_get_nodename,
  };
