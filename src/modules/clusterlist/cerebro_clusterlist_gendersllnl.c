/*****************************************************************************\
 *  $Id: cerebro_clusterlist_gendersllnl.c,v 1.27 2005-06-28 17:08:38 achu Exp $
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

#include "cerebro_clusterlist_genders_util.h"
#include "cerebro_clusterlist_util.h"

#include "debug.h"

#define GENDERSLLNL_CLUSTERLIST_MODULE_NAME "gendersllnl"

/*
 * gh
 *
 * genders handle
 */
static genders_t gh = NULL;

/*
 * gendersllnl_clusterlist_setup
 *
 * gendersllnl clusterlist module setup function
 */
static int
gendersllnl_clusterlist_setup(void)
{
  int rv;

  if (gh)
    {
      CEREBRO_DBG(("gh non-null"));
      return 0;
    }

  rv = cerebro_clusterlist_genders_setup(&gh);

#if HAVE_GENDERS_INDEX_ATTRVALS
  if (!rv)
    {
      /* This is for performance improvements if the indexing API
       * functions are available.  Don't fail and return -1, since the
       * rest is not dependent on this section of code.
       */
      genders_index_attrvals(gh, GENDERS_ALTNAME_ATTRIBUTE);
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
  if (!gh)
    return 0;

  return cerebro_clusterlist_genders_cleanup(&gh);
}

/*
 * gendersllnl_clusterlist_numnodes
 *
 * gendersllnl clusterlist module numnodes function
 */
static int
gendersllnl_clusterlist_numnodes(void)
{
  if (!gh)
    {
      CEREBRO_DBG(("gh null"));
      return -1;
    }

  return cerebro_clusterlist_genders_numnodes(gh);
}

/*
 * gendersllnl_clusterlist_get_all_nodes
 *
 * gendersllnl clusterlist module get_all_nodes function
 */
static int
gendersllnl_clusterlist_get_all_nodes(char ***nodes)
{
  if (!gh)
    {
      CEREBRO_DBG(("gh null"));
      return -1;
    }

  if (!nodes)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  return cerebro_clusterlist_genders_get_all_nodes(gh, nodes);
}

/*
 * gendersllnl_clusterlist_node_in_cluster
 *
 * gendersllnl clusterlist module node_in_cluster function
 */
static int
gendersllnl_clusterlist_node_in_cluster(const char *node)
{
  char nodebuf[CEREBRO_MAX_NODENAME_LEN+1];
  char *nodePtr = NULL;
  int flag;

  if (!gh)
    {
      CEREBRO_DBG(("gh null"));
      return -1;
    }

  if (!node)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  /* Shorten hostname if necessary */
  if (strchr(node, '.'))
    {
      char *p;

      memset(nodebuf, '\0', CEREBRO_MAX_NODENAME_LEN+1);
      strncpy(nodebuf, node, CEREBRO_MAX_NODENAME_LEN);
      p = strchr(nodebuf, '.');
      *p = '\0';
      nodePtr = nodebuf;
    }
  else
    nodePtr = (char *)node;

  if ((flag = genders_isnode_or_altnode(gh, nodePtr)) < 0)
    {
      CEREBRO_DBG(("genders_isnode_or_altnode: %s", genders_errormsg(gh)));
      return -1;
    }
  
  return flag;
}

/*
 * gendersllnl_clusterlist_get_nodename
 *
 * gendersllnl clusterlist module get_nodename function
 */
static int
gendersllnl_clusterlist_get_nodename(const char *node, 
				     char *buf, 
				     unsigned int buflen)
{
  char nodebuf[CEREBRO_MAX_NODENAME_LEN+1];
  char *nodePtr = NULL;
  int rv;

  if (!gh)
    {
      CEREBRO_DBG(("gh null"));
      return -1;
    }

  if (!node || !buf || !buflen)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  /* Shorten hostname if necessary */
  if (strchr(node, '.'))
    {
      char *p;

      memset(nodebuf, '\0', CEREBRO_MAX_NODENAME_LEN+1);
      strncpy(nodebuf, node, CEREBRO_MAX_NODENAME_LEN);
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
      if (genders_to_gendname(gh, nodePtr, buf, buflen) < 0)
	{
	  CEREBRO_DBG(("genders_to_gendname: %s", genders_errormsg(gh)));
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
