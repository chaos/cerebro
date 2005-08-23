/*****************************************************************************\
 *  $Id: cerebro_clusterlist_genders.c,v 1.33 2005-08-23 21:10:15 achu Exp $
 *****************************************************************************
 *  Copyright (C) 2005 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <http://www.llnl.gov/linux/cerebro/>.
 *
 *  Cerebro is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  Cerebro is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with Genders; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
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

#include "cerebro_clusterlist_genders_util.h"
#include "cerebro_clusterlist_util.h"

#include "debug.h"

#define GENDERS_CLUSTERLIST_MODULE_NAME "genders"

/*
 * gh
 *
 * genders handle
 */
static genders_t gh = NULL;

/*
 * genders_clusterlist_setup
 *
 * genders clusterlist module setup function
 */
static int 
genders_clusterlist_setup(void)
{
  if (gh)
    {
      CEREBRO_DBG(("gh non-null"));
      return 0;
    }

  return cerebro_clusterlist_genders_setup(&gh);
}

/*
 * genders_clusterlist_cleanup
 *
 * genders clusterlist module cleanup function
 */
static int
genders_clusterlist_cleanup(void)
{
  if (!gh)
    return 0;

  return cerebro_clusterlist_genders_cleanup(&gh);
}

/*
 * genders_clusterlist_numnodes
 *
 * genders clusterlist module numnodes function
 */
static int 
genders_clusterlist_numnodes(void)
{
  if (!gh)
    {
      CEREBRO_DBG(("gh null"));
      return -1;
    }

  return cerebro_clusterlist_genders_numnodes(gh);
}

/*
 * genders_clusterlist_get_all_nodes
 *
 * genders clusterlist module get_all_nodes function
 */
static int
genders_clusterlist_get_all_nodes(char ***nodes)
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
 * genders_clusterlist_node_in_cluster
 *
 * genders clusterlist module node_in_cluster function
 */
static int
genders_clusterlist_node_in_cluster(const char *node)
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

  if ((flag = genders_isnode(gh, nodePtr)) < 0)
    {
      CEREBRO_DBG(("genders_isnode: %s", genders_errormsg(gh)));
      return -1;
    }

  return flag;
}

/*
 * genders_clusterlist_get_nodename
 *
 * genders clusterlist module get_nodename function
 */
static int
genders_clusterlist_get_nodename(const char *node, 
                                 char *buf, 
                                 unsigned int buflen)
{
  char nodebuf[CEREBRO_MAX_NODENAME_LEN+1];
  char *nodePtr = NULL;

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

  return cerebro_copy_nodename(nodePtr, buf, buflen);
}

#if WITH_STATIC_MODULES
struct cerebro_clusterlist_module_info genders_clusterlist_module_info =
#else  /* !WITH_STATIC_MODULES */
struct cerebro_clusterlist_module_info clusterlist_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    GENDERS_CLUSTERLIST_MODULE_NAME,
    &genders_clusterlist_setup,
    &genders_clusterlist_cleanup,
    &genders_clusterlist_numnodes,
    &genders_clusterlist_get_all_nodes,
    &genders_clusterlist_node_in_cluster,
    &genders_clusterlist_get_nodename,
  };
