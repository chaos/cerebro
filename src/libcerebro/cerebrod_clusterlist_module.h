/*****************************************************************************\
 *  $Id: cerebrod_clusterlist_module.h,v 1.1 2005-03-24 01:29:21 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_CLUSTERLIST_MODULE_H
#define _CEREBROD_CLUSTERLIST_MODULE_H

/*
 * Cerebrod_clusterlist_parse_options
 *
 * function prototype for clusterlist module function to parse options
 *
 * - options - string of arrays.  The strings are usually key=value pairs
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebrod_clusterlist_parse_options)(char **options);

/*
 * Cerebrod_clusterlist_init
 *
 * function prototype for clusterlist module function to initialize
 * module
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebrod_clusterlist_init)(void);

/*
 * Cerebrod_clusterlist_finish
 *
 * function prototype for clusterlist module function to finish up
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebrod_clusterlist_finish)(void);

/*
 * Cerebrod_clusterlist_get_all_nodes
 *
 * function prototype for clusterlist module function to get all
 * cluster nodes.  Caller is responsible for allocating a char * array
 * of appropriate length and freeing returned Strdup()'ed entries.
 *
 * - nodes - array of char * pointers
 * - nodeslen - array length, usually retrieved by numnodes
 *   clusterlist module function
 *
 * Returns number of cluster nodes copied in the buffer  on success, -1 on error
 */
typedef int (*Cerebrod_clusterlist_get_all_nodes)(char **nodes, unsigned int nodeslen);

/*
 * Cerebrod_clusterlist_numnodes
 *
 * function prototype for clusterlist module function to determine the
 * number of nodes in the cluster.
 *
 * Returns number of cluster nodes on success, -1 on error
 */
typedef int (*Cerebrod_clusterlist_numnodes)(void);

/*
 * Cerebrod_clusterlist_node_in_cluster
 *
 * function prototype for clusterlist module function to determine if
 * a node is in the cluser.
 *
 * - node - node string
 *
 * Returns 1 if node is in cluster, 0 if not, -1 on error
 */
typedef int (*Cerebrod_clusterlist_node_in_cluster)(char *node);

/*
 * Cerebrod_clusterlist_get_nodename
 *
 * function prototype for clusterlist module function to determine the
 * nodename to use for hashing.  Typically, this function will only
 * copy the node passed in into the buffer passed in.  However, in
 * some circumstances, nodes with duplicate names (perhaps aliased)
 * need to be identified with a single nodename key.
 *
 * - node - node string
 * - buf - buffer pointer
 * - buflen - buffer length
 *
 * Returns nodename in buffer, 0 on success, -1 on error
 */
typedef int (*Cerebrod_clusterlist_get_nodename)(char *node, char *buf, unsigned int buflen);
	     
/*  
 * struct cerebrod_clusterlist_module_info
 *
 * contains clusterlist module information and operations.  Required
 * to be defined in each clusterlist module.
 */
struct cerebrod_clusterlist_module_info
{
  char *clusterlist_module_name;
  Cerebrod_clusterlist_parse_options parse_options;
  Cerebrod_clusterlist_init init;
  Cerebrod_clusterlist_finish finish;
  Cerebrod_clusterlist_get_all_nodes get_all_nodes;
  Cerebrod_clusterlist_numnodes numnodes;
  Cerebrod_clusterlist_node_in_cluster node_in_cluster;
  Cerebrod_clusterlist_get_nodename get_nodename;
};

#endif /* _CEREBROD_CLUSTERLIST_MODULE_H */
