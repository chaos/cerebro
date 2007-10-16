/*****************************************************************************\
 *  $Id: clusterlist_module_default.c,v 1.8 2007-10-16 22:43:17 chu11 Exp $
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
 *  with Cerebro; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */

#include "cerebro/cerebro_clusterlist_module.h"

#include "debug.h"

#define DEFAULT_CLUSTERLIST_MODULE_NAME "default"

int
default_clusterlist_setup(void)
{
  return 0;
}

int
default_clusterlist_cleanup(void)
{
  return 0;
}

int
default_clusterlist_numnodes(void)
{
  return 0;
}

int
default_clusterlist_get_all_nodes(char ***nodes)
{
  return 0;
}

int
default_clusterlist_node_in_cluster(const char *node)
{
  /* Must assume it is in the cluster */
  return 1;
}

int
default_clusterlist_get_nodename(const char *node, char *buf, unsigned int buflen)
{
  int len;

  if (!node || !buf || !buflen)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }
  
  len = strlen(node);
 
  if ((len + 1) > buflen)
    {
      CEREBRO_DBG(("buflen too small: %d %d", len, buflen));
      return -1;
    }
 
  strcpy(buf, node);

  return 0;
}

struct cerebro_clusterlist_module_info default_clusterlist_module_info =
  {
    DEFAULT_CLUSTERLIST_MODULE_NAME,
    &default_clusterlist_setup,
    &default_clusterlist_cleanup,
    &default_clusterlist_numnodes,
    &default_clusterlist_get_all_nodes,
    &default_clusterlist_node_in_cluster,
    &default_clusterlist_get_nodename,
  };
