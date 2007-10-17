/*****************************************************************************\
 *  $Id: cerebro_clusterlist_module.h,v 1.5 2007-10-17 22:04:49 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2005-2007 The Regents of the University of California.
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
 *  with Cerebro. If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#ifndef _CEREBRO_CLUSTERLIST_MODULE_H
#define _CEREBRO_CLUSTERLIST_MODULE_H

/*
 * Cerebro_clusterlist_setup
 *
 * function prototype for clusterlist module function to setup the
 * module.  Required to be defined by each clusterlist module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_clusterlist_setup)(void);

/*
 * Cerebro_clusterlist_cleanup
 *
 * function prototype for clusterlist module function to
 * cleanup. Required to be defined by each clusterlist module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_clusterlist_cleanup)(void);

/*
 * Cerebro_clusterlist_numnodes
 *
 * function prototype for clusterlist module function to determine the
 * number of nodes in the cluster.  Required to be defined by each
 * clusterlist module.
 *
 * Returns number of cluster nodes on success, -1 on error
 */
typedef int (*Cerebro_clusterlist_numnodes)(void);

/*
 * Cerebro_clusterlist_get_all_nodes
 *
 * function prototype for clusterlist module function to get all
 * cluster nodes.  Caller is responsible for freeing the created 
 * char ** array of nodes.  The returned array of strings will
 * be NULL terminated.   Required to be defined by each clusterlist 
 * module.
 *
 * - nodes - pointer to return char ** array of nodes
 *
 * Returns number of cluster nodes retrieved on success, -1
 * on error
 */
typedef int (*Cerebro_clusterlist_get_all_nodes)(char ***nodes);

/*
 * Cerebro_clusterlist_node_in_cluster
 *
 * function prototype for clusterlist module function to determine if
 * a node is in the cluser. Required to be defined by each clusterlist
 * module.
 *
 * - node - node string
 *
 * Returns 1 if node is in cluster, 0 if not, -1 on error
 */
typedef int (*Cerebro_clusterlist_node_in_cluster)(const char *node);

/*
 * Cerebro_clusterlist_get_nodename
 *
 * function prototype for clusterlist module function to determine the
 * nodename to use for hashing.  Typically, this function will only
 * copy the node passed in into the buffer passed in.  However, in
 * some circumstances, nodes with duplicate names (perhaps aliased)
 * need to be identified with a single nodename key. Required to be
 * defined by each clusterlist module.
 *
 * - node - node string
 * - buf - buffer pointer
 * - buflen - buffer length
 *
 * Returns nodename in buffer, 0 on success, -1 on error
 */
typedef int (*Cerebro_clusterlist_get_nodename)(const char *node, 
                                                char *buf, 
                                                unsigned int buflen);
	     
/*  
 * struct cerebro_clusterlist_module_info
 *
 * contains clusterlist module information and operations.  Required
 * to be defined in each clusterlist module.
 */
struct cerebro_clusterlist_module_info
{
  char *clusterlist_module_name;
  Cerebro_clusterlist_setup setup;
  Cerebro_clusterlist_cleanup cleanup;
  Cerebro_clusterlist_numnodes numnodes;
  Cerebro_clusterlist_get_all_nodes get_all_nodes;
  Cerebro_clusterlist_node_in_cluster node_in_cluster;
  Cerebro_clusterlist_get_nodename get_nodename;
};

#endif /* _CEREBRO_CLUSTERLIST_MODULE_H */
