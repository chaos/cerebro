/*****************************************************************************\
 *  $Id: cerebrod_clusterlist_gendersllnl.c,v 1.6 2005-03-17 05:05:52 achu Exp $
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

#include <gendersllnl.h>

#include "cerebrod.h"
#include "cerebrod_clusterlist.h"
#include "error.h"
#include "wrappers.h"

genders_t handle = NULL;
char *gendersllnl_file = NULL;

static void
_parse_options(char **options)
{
  int i = 0;

  assert(options);

  while (options[i] != NULL)
    {
      if (strstr(options[i], "filename"))
	{
	  char *p = strchr(options[i], '=');

	  if (!p)
	    err_exit("hostsfile clusterlist module: filename unspecified");

	  p++;
	  if (p == '\0')
	    err_exit("hostsfile clusterlist module: filename unspecified");

	  hostsfile_file = Strdup(p);
	}
      else
	err_exit("hostsfile clusterlist module: option '%s' unrecognized", options[i]);

      i++;
    }
}

int 
gendersllnl_clusterlist_init(char **options)
{
  assert(!handle);

  if (options)
    _parse_options(options);

  if (!(handle = genders_handle_create()))
    err_exit("gendersllnl_clusterlist_init: genders_handle_create");

  if (genders_load_data(handle, gendersllnl_file) < 0)
    {
      if (genders_errnum(handle) == GENDERS_ERR_OPEN)
	{
	  if (gendersllnl_file)
	    err_exit("gendersllnl clusterlist file '%s' cannot be opened", gendersllnl_file);
	  else
	    err_exit("gendersllnl clusterlist file '%s' cannot be opened", GENDERS_DEFAULT_FILE);
	}
      else
	err_exit("gendersllnl_clusterlist_init: genders_load_data: %s",
		 genders_errormsg(handle));
    }

  return 0;
}

int
gendersllnl_clusterlist_finish(void)
{
  assert(handle);

  if (genders_handle_destroy(handle) < 0)
    err_exit("gendersllnl_clusterlist_finish: genders_handle_destroy: %s",
	     genders_errormsg(handle));

  Free(gendersllnl_file);
  handle = NULL;
  gendersllnl_file = NULL;

  return 0;
}

int
gendersllnl_clusterlist_get_all_nodes(char **nodes, unsigned int nodeslen)
{
  char **nodelist;
  int i, nodelistlen, numnodes;
  
  assert(handle);
  assert(nodes);

  if ((nodelistlen = genders_nodelist_create(handle, &nodelist)) < 0)
    err_exit("gendersllnl_clusterlist_get_all_nodes: genders_nodelist_create: %s",
	     genders_errormsg(handle));
  
  if ((numnodes = genders_getnodes(handle, nodelist, nodelistlen, NULL, NULL)) < 0)
    err_exit("gendersllnl_clusterlist_get_all_nodes: genders_getnodes: %s",
	     genders_errormsg(handle));

  if (numnodes > nodeslen)
    err_exit("gendersllnl_clusterlist_get_all_nodes: nodeslen too small");

  for (i = 0; i < numnodes; i++)
    nodes[i] = Strdup(nodelist[i]);

  if (genders_nodelist_destroy(handle, nodelist) < 0)
    err_exit("gendersllnl_clusterlist_get_all_nodes: genders_nodelist_destroy: %s",
	     genders_errormsg(handle));
  
  return numnodes;
}

int 
gendersllnl_clusterlist_numnodes(void)
{
  int ret;

  assert(handle);

  if ((ret = genders_getnumnodes(handle)) < 0)
    err_exit("gendersllnl_clusterlist_numnodes: genders_getnumnodes: %s",
	     genders_errormsg(handle));

  return ret;
}

int
gendersllnl_clusterlist_node_in_cluster(char *node)
{
  int ret, free_flag = 0;
  char *nodePtr = NULL;

  assert(handle);
  assert(node);

  /* Shorten hostname if necessary */
  if (strchr(node, '.'))
    {
      char *p;

      nodePtr = Strdup(node);
      p = strchr(nodePtr, '.');
      *p = '\0';
      free_flag++;
    }
  else
    nodePtr = node;

  if ((ret = genders_isnode_or_altnode(handle, nodePtr)) < 0)
    err_exit("gendersllnl_clusterlist_node_in_cluster: genders_isnode: %s",
	     genders_errormsg(handle));

  if (free_flag)
    Free(nodePtr);

  return ret;
}

int
gendersllnl_clusterlist_get_nodename(char *node, char *buf, int buflen)
{
  int len, free_flag = 0;
  char *nodePtr = NULL;

  assert(handle);
  assert(node);
  assert(buf);
  assert(buflen > 0);

  /* Shorten hostname if necessary */
  if (strchr(node, '.'))
    {
      char *p;

      nodePtr = Strdup(node);
      p = strchr(nodePtr, '.');
      *p = '\0';
      free_flag++;
    }
  else
    nodePtr = node;

  if (genders_to_gendname(handle, nodePtr, buf, buflen) < 0)
    err_exit("gendersllnl_clusterlist_get_nodename: genders_to_gendname: %s",
	     genders_errormsg(handle));

  if (free_flag)
    Free(nodePtr);

  return 0;
}

struct cerebrod_clusterlist_ops clusterlist_ops =
  {
    &gendersllnl_clusterlist_init,
    &gendersllnl_clusterlist_finish,
    &gendersllnl_clusterlist_get_all_nodes,
    &gendersllnl_clusterlist_numnodes,
    &gendersllnl_clusterlist_node_in_cluster,
    &gendersllnl_clusterlist_get_nodename,
  };


