/*****************************************************************************\
 *  $Id: cerebrod_clusterlist_none.c,v 1.1 2005-03-14 22:17:35 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <assert.h>
#include <errno.h>

#include "cerebrod.h"
#include "cerebrod_clusterlist.h"
#include "error.h"
#include "wrappers.h"

int 
none_clusterlist_init(char *cmdline)
{
  return 0;
}

int
none_clusterlist_finish(void)
{
  return 0;
}

int
none_clusterlist_get_all_nodes(char **nodes, unsigned int nodeslen)
{
  return 0;
}

int 
none_clusterlist_numnodes(void)
{
  return 0;
}

int
none_clusterlist_node_in_cluster(char *node)
{
  /* Must assume it is */
  return 1;
}

int
none_clusterlist_get_nodename(char *node, char *buf, int buflen)
{
  int len;

  assert(node);
  assert(buf);
  assert(buflen > 0);

  len = strlen(node);

  if ((len + 1) > buflen)
    err_exit("none_clusterlist_get_nodename: buflen too small: %d %d",
	     len, buflen);

  strcpy(buf, node);

  return 0;
}

struct cerebrod_clusterlist_ops clusterlist_ops =
  {
    &none_clusterlist_init,
    &none_clusterlist_finish,
    &none_clusterlist_get_all_nodes,
    &none_clusterlist_numnodes,
    &none_clusterlist_node_in_cluster,
    &none_clusterlist_get_nodename,
  };


