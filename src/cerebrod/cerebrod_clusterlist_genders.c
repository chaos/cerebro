/*****************************************************************************\
 *  $Id: cerebrod_clusterlist_genders.c,v 1.3 2005-03-14 23:46:02 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#if HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */
#include <assert.h>
#include <errno.h>

#include <genders.h>

#include "cerebrod.h"
#include "cerebrod_clusterlist.h"
#include "error.h"
#include "wrappers.h"

genders_t handle = NULL;
char *genders_file = NULL;

static void
_cmdline_parse(int argc, char **argv)
{
  char c;
  char *options = "f:";

  assert(argv);

#if HAVE_GETOPT_LONG
  struct option long_options[] =
    {
      {"filename",            1, NULL, 'c'},
    };
#endif /* HAVE_GETOPT_LONG */

  /* turn off output messages */
  opterr = 0;

#if HAVE_GETOPT_LONG
  while ((c = getopt_long(argc, argv, options, long_options, NULL)) != -1)
#else
  while ((c = getopt(argc, argv, options)) != -1)
#endif
    {
      switch (c)
	{
	case 'f':
	  genders_file = optarg;
	  break;
	case '?':
	default:
	  err_exit("genders_clusterlist: unknown command line option '%c'", optopt);
	}
    }
}

int 
genders_clusterlist_init(char *cmdline)
{
  assert(!handle);

  if (cmdline)
    {
      char **argv;
      int argc;

      argv_create(cmdline, "", &argc, &argv);

      _cmdline_parse(argc, argv);

      argv_destroy(argv);
    }

  if (!(handle = genders_handle_create()))
    err_exit("genders_clusterlist_init: genders_handle_create");

  if (genders_load_data(handle, genders_file) < 0)
    {
      if (genders_errnum(handle) == GENDERS_ERR_OPEN)
	{
	  if (genders_file)
	    err_exit("genders clusterlist file '%s' cannot be opened", genders_file);
	  else
	    err_exit("genders clusterlist file '%s' cannot be opened", GENDERS_DEFAULT_FILE);
	}
      else
	err_exit("genders_clusterlist_init: genders_load_data: %s",
		 genders_errormsg(handle));
    }

  return 0;
}

int
genders_clusterlist_finish(void)
{
  assert(handle);

  if (genders_handle_destroy(handle) < 0)
    err_exit("genders_clusterlist_finish: genders_handle_destroy: %s",
	     genders_errormsg(handle));

  handle = NULL;
  genders_file = NULL;

  return 0;
}

int
genders_clusterlist_get_all_nodes(char **nodes, unsigned int nodeslen)
{
  char **nodelist;
  int i, nodelistlen, numnodes;
  
  assert(handle);
  assert(nodes);

  if ((nodelistlen = genders_nodelist_create(handle, &nodelist)) < 0)
    err_exit("genders_clusterlist_get_all_nodes: genders_nodelist_create: %s",
	     genders_errormsg(handle));
  
  if ((numnodes = genders_getnodes(handle, nodelist, nodelistlen, NULL, NULL)) < 0)
    err_exit("genders_clusterlist_get_all_nodes: genders_getnodes: %s",
	     genders_errormsg(handle));

  if (numnodes > nodeslen)
    err_exit("genders_clusterlist_get_all_nodes: nodeslen too small");

  for (i = 0; i < numnodes; i++)
    nodes[i] = Strdup(nodelist[i]);

  if (genders_nodelist_destroy(handle, nodelist) < 0)
    err_exit("genders_clusterlist_get_all_nodes: genders_nodelist_destroy: %s",
	     genders_errormsg(handle));
  
  return numnodes;
}

int 
genders_clusterlist_numnodes(void)
{
  int ret;

  assert(handle);

  if ((ret = genders_getnumnodes(handle)) < 0)
    err_exit("genders_clusterlist_numnodes: genders_getnumnodes: %s",
	     genders_errormsg(handle));

  return ret;
}

int
genders_clusterlist_node_in_cluster(char *node)
{
  int ret;
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
    }
  else
    nodePtr = node;

  if ((ret = genders_isnode(handle, nodePtr)) < 0)
    err_exit("genders_clusterlist_node_in_cluster: genders_isnode: %s",
	     genders_errormsg(handle));

  Free(nodePtr);

  return ret;
}

int
genders_clusterlist_get_nodename(char *node, char *buf, int buflen)
{
  int len;

  assert(handle);
  assert(node);
  assert(buf);
  assert(buflen > 0);

  len = strlen(node);

  if ((len + 1) > buflen)
    err_exit("genders_clusterlist_get_nodename: buflen too small: %d %d",
	     len, buflen);

  strcpy(buf, node);

  return 0;
}

struct cerebrod_clusterlist_ops clusterlist_ops =
  {
    &genders_clusterlist_init,
    &genders_clusterlist_finish,
    &genders_clusterlist_get_all_nodes,
    &genders_clusterlist_numnodes,
    &genders_clusterlist_node_in_cluster,
    &genders_clusterlist_get_nodename,
  };
