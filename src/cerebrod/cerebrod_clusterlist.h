/*****************************************************************************\
 *  $Id: cerebrod_clusterlist.h,v 1.7 2005-03-21 18:28:38 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_CLUSTERLIST_H
#define _CEREBROD_CLUSTERLIST_H

#include "list.h"
#include "wrappers.h"

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

/*  
 * cerebrod_clusterlist_setup
 *
 * load clusterlist module.  search for clusterlist module to load if
 * necessary
 *
 * Return 0 on success, -1 on error
 */
int cerebrod_clusterlist_setup(void);

/*  
 * cerebrod_clusterlist_cleanup
 *
 * cleanup clusterlist module state
 *
 * Return 0 on success, -1 on error
 */
int cerebrod_clusterlist_cleanup(void);

/* 
 * cerebrod_clusterlist_module_name
 *
 * Return clusterlist module name
 */
char *cerebrod_clusterlist_module_name(void);

/* 
 * cerebrod_clusterlist_parse_options
 * 
 * call clusterlist module parse options function
 */
int cerebrod_clusterlist_parse_options(void);

/* 
 * cerebrod_clusterlist_init
 * 
 * call clusterlist module init function
 */
int cerebrod_clusterlist_init(void);

/* 
 * cerebrod_clusterlist_finish
 * 
 * call clusterlist module parse finish function
 */
int cerebrod_clusterlist_finish(void);

/* 
 * cerebrod_clusterlist_get_all_nodes
 * 
 * call clusterlist module get all nodes function
 */
int cerebrod_clusterlist_get_all_nodes(char **nodes, unsigned int nodeslen);

/* 
 * cerebrod_clusterlist_numnodes
 * 
 * call clusterlist module numnodes function
 */
int cerebrod_clusterlist_numnodes(void);

/* 
 * cerebrod_clusterlist_node_in_cluster
 * 
 * call clusterlist module node in cluster function
 */
int cerebrod_clusterlist_node_in_cluster(char *node);

/* 
 * cerebrod_clusterlist_get_nodename
 * 
 * call clusterlist module get nodename function
 */
int cerebrod_clusterlist_get_nodename(char *node, char *buf, unsigned int buflen);

#endif /* _CEREBROD_CLUSTERLIST_H */
