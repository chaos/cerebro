/*****************************************************************************\
 *  $Id: cerebrod_clusterlist.h,v 1.4 2005-03-19 03:28:29 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_CLUSTERLIST_H
#define _CEREBROD_CLUSTERLIST_H

#include "list.h"
#include "wrappers.h"

typedef int (*Cerebrod_clusterlist_parse_options)(char **options);
typedef int (*Cerebrod_clusterlist_init)(void);
typedef int (*Cerebrod_clusterlist_finish)(void);
typedef int (*Cerebrod_clusterlist_get_all_nodes)(char **nodes, unsigned int nodeslen);
typedef int (*Cerebrod_clusterlist_numnodes)(void);
typedef int (*Cerebrod_clusterlist_node_in_cluster)(char *);
typedef int (*Cerebrod_clusterlist_get_nodename)(char *, char *, int);
	     
struct cerebrod_clusterlist_module_info
{
  char *clusterlist_module_name;
};

struct cerebrod_clusterlist_module_ops 
{
  Cerebrod_clusterlist_parse_options parse_options;
  Cerebrod_clusterlist_init init;
  Cerebrod_clusterlist_finish finish;
  Cerebrod_clusterlist_get_all_nodes get_all_nodes;
  Cerebrod_clusterlist_numnodes numnodes;
  Cerebrod_clusterlist_node_in_cluster node_in_cluster;
  Cerebrod_clusterlist_get_nodename get_nodename;
};

int cerebrod_clusterlist_setup(void);
int cerebrod_clusterlist_cleanup(void);

int cerebrod_clusterlist_parse_options(void);
int cerebrod_clusterlist_init(void);
int cerebrod_clusterlist_finish(void);
int cerebrod_clusterlist_get_all_nodes(char **nodes, unsigned int nodeslen);
int cerebrod_clusterlist_numnodes(void);
int cerebrod_clusterlist_node_in_cluster(char *node);
int cerebrod_clusterlist_get_nodename(char *node, char *buf, int buflen);

char *cerebrod_clusterlist_module_name(void);

#endif /* _CEREBROD_CLUSTERLIST_H */
