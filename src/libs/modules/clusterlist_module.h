/*****************************************************************************\
 *  $Id: clusterlist_module.h,v 1.10 2010-02-02 01:01:20 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2011 Lawrence Livermore National Security, LLC.
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

#ifndef _CLUSTERLIST_MODULE_H
#define _CLUSTERLIST_MODULE_H

typedef struct clusterlist_module *clusterlist_module_t;

/*
 * clusterlist_module_load
 *
 * Find and load the clusterlist module.  If none is found, will
 * assume a default clusterlist module.
 * 
 * Returns clusterlist module handle on success, NULL on error
 */
clusterlist_module_t clusterlist_module_load(void);

/*
 * clusterlist_module_unload
 *
 * Unload/cleanup the clusterlist module specified by the handle.
 *
 * Returns 0 on success, -1 on error
 */
int clusterlist_module_unload(clusterlist_module_t handle);

/*
 * clusterlist_module_found
 *
 * Return bool if clusterlist module found or not.
 */
int clusterlist_module_found(clusterlist_module_t handle);

/*
 * clusterlist_module_name
 *
 * Return clusterlist module name
 */
char *clusterlist_module_name(clusterlist_module_t handle);

/*
 * clusterlist_module_interface_version
 *
 * Return clusterlist interface version
 */
int clusterlist_module_interface_version(clusterlist_module_t handle);

/*
 * clusterlist_module_setup
 *
 * call clusterlist module setup function
 */
int clusterlist_module_setup(clusterlist_module_t handle);

/*
 * clusterlist_module_cleanup
 *
 * call clusterlist module cleanup function
 */
int clusterlist_module_cleanup(clusterlist_module_t handle);

/*
 * clusterlist_module_numnodes
 *
 * call clusterlist module numnodes function
 */
int clusterlist_module_numnodes(clusterlist_module_t handle);

/*
 * clusterlist_module_get_all_nodes
 *
 * call clusterlist module get all nodes function
 */
int clusterlist_module_get_all_nodes(clusterlist_module_t handle, char ***nodes);

/*
 * clusterlist_module_node_in_cluster
 *
 * call clusterlist module node in cluster function
 */
int clusterlist_module_node_in_cluster(clusterlist_module_t handle, 
                                       const char *node);

/*
 * clusterlist_module_get_nodename
 *
 * call clusterlist module get nodename function
 */
int clusterlist_module_get_nodename(clusterlist_module_t handle,
				    const char *node, 
				    char *buf, 
				    unsigned int buflen);

#endif /* _CLUSTERLIST_MODULE_H */
