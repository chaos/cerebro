/*****************************************************************************\
 *  $Id: cerebro_clusterlist_module.h,v 1.3 2005-04-20 23:36:26 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_CLUSTERLIST_MODULE_H
#define _CEREBRO_CLUSTERLIST_MODULE_H

/*
 * Cerebro_clusterlist_parse_options
 *
 * function prototype for clusterlist module function to parse options
 *
 * - options - string of arrays.  The strings are usually key=value pairs
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_clusterlist_parse_options)(char **options);

/*
 * Cerebro_clusterlist_init
 *
 * function prototype for clusterlist module function to initialize
 * module
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_clusterlist_init)(void);

/*
 * Cerebro_clusterlist_finish
 *
 * function prototype for clusterlist module function to finish up
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_clusterlist_finish)(void);

/*
 * Cerebro_clusterlist_get_all_nodes
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
typedef int (*Cerebro_clusterlist_get_all_nodes)(char **nodes, unsigned int nodeslen);

/*
 * Cerebro_clusterlist_numnodes
 *
 * function prototype for clusterlist module function to determine the
 * number of nodes in the cluster.
 *
 * Returns number of cluster nodes on success, -1 on error
 */
typedef int (*Cerebro_clusterlist_numnodes)(void);

/*
 * Cerebro_clusterlist_node_in_cluster
 *
 * function prototype for clusterlist module function to determine if
 * a node is in the cluser.
 *
 * - node - node string
 *
 * Returns 1 if node is in cluster, 0 if not, -1 on error
 */
typedef int (*Cerebro_clusterlist_node_in_cluster)(char *node);

/*
 * Cerebro_clusterlist_get_nodename
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
typedef int (*Cerebro_clusterlist_get_nodename)(char *node, char *buf, unsigned int buflen);
	     
/*  
 * struct cerebro_clusterlist_module_info
 *
 * contains clusterlist module information and operations.  Required
 * to be defined in each clusterlist module.
 */
struct cerebro_clusterlist_module_info
{
  char *clusterlist_module_name;
  Cerebro_clusterlist_parse_options parse_options;
  Cerebro_clusterlist_init init;
  Cerebro_clusterlist_finish finish;
  Cerebro_clusterlist_get_all_nodes get_all_nodes;
  Cerebro_clusterlist_numnodes numnodes;
  Cerebro_clusterlist_node_in_cluster node_in_cluster;
  Cerebro_clusterlist_get_nodename get_nodename;
};

#endif /* _CEREBRO_CLUSTERLIST_MODULE_H */
