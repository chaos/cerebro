/*****************************************************************************\
 *  $Id: cerebrod_clusterlist.h,v 1.2 2005-03-17 05:05:52 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_CLUSTERLIST_H
#define _CEREBROD_CLUSTERLIST_H

#include "list.h"
#include "wrappers.h"

typedef int (*Cerebrod_clusterlist_init)(char **options);
typedef int (*Cerebrod_clusterlist_finish)(void);
typedef int (*Cerebrod_clusterlist_get_all_nodes)(char **nodes, unsigned int nodeslen);
typedef int (*Cerebrod_clusterlist_numnodes)(void);
typedef int (*Cerebrod_clusterlist_node_in_cluster)(char *);
typedef int (*Cerebrod_clusterlist_get_nodename)(char *, char *, int);
	     
struct cerebrod_clusterlist_ops 
{
  Cerebrod_clusterlist_init init;
  Cerebrod_clusterlist_finish finish;
  Cerebrod_clusterlist_get_all_nodes get_all_nodes;
  Cerebrod_clusterlist_numnodes numnodes;
  Cerebrod_clusterlist_node_in_cluster node_in_cluster;
  Cerebrod_clusterlist_get_nodename get_nodename;
};

int cerebrod_clusterlist_init(void);
int cerebrod_clusterlist_finish(void);
int cerebrod_clusterlist_get_all_nodes(char **nodes, unsigned int nodeslen);
int cerebrod_clusterlist_numnodes(void);
int cerebrod_clusterlist_node_in_cluster(char *node);
int cerebrod_clusterlist_get_nodename(char *node, char *buf, int buflen);

#endif /* _CEREBROD_CLUSTERLIST_H */
