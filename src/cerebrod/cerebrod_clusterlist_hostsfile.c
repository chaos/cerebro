/*****************************************************************************\
 *  $Id: cerebrod_clusterlist_hostsfile.c,v 1.1 2005-03-14 17:05:14 achu Exp $
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

#include "cerebrod.h"
#include "cerebrod_clusterlist.h"
#include "error.h"
#include "wrappers.h"

List hosts = NULL;
char *hostsfile_file = CEREBROD_HOSTSFILE_DEFAULT;

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
	  hostsfile_file = optarg;
	  break;
	case '?':
	default:
	  err_exit("hostsfile_clusterlist: unknown command line option '%c'", optopt);
	}
    }
}

int 
hostsfile_clusterlist_init(char *cmdline)
{
  int fd;

  assert(!handle);

  if (cmdline)
    {
      char **argv;
      int argc;

      argv_create(cmdline, "", &argc, &argv);

      _cmdline_parse(argc, argv);

      argv_destroy(argv);
    }

  hosts = List_create((ListDelF)Free);

  if ((fd = open(

  if (!(handle = hostsfile_handle_create()))
    err_exit("hostsfile_clusterlist_init: hostsfile_handle_create");

  if (hostsfile_load_data(handle, hostsfile_file) < 0)
    {
      if (hostsfile_errnum(handle) == HOSTSFILE_ERR_OPEN)
	{
	  if (hostsfile_file)
	    err_exit("hostsfile clusterlist file '%s' not found", hostsfile_file);
	  else
	    err_exit("hostsfile clusterlist file '%s' not found", HOSTSFILE_DEFAULT_FILE);
	}
      else
	err_exit("hostsfile_clusterlist_init: hostsfile_load_data: %s",
		 hostsfile_errormsg(handle));
    }

  return 0;
}

int
hostsfile_clusterlist_finish(void)
{
  assert(handle);

  List_destroy(hosts);
  hosts = NULL;
  hostsfile_file = NULL;

  return 0;
}

int
hostsfile_clusterlist_get_all_nodes(char **nodes, unsigned int nodeslen)
{
  char *node;
  ListIterator itr;
  int numnodes, i = 0;

  assert(handle && nodes);

  numnodes = list_count(hosts);

  if (numnodes > nodeslen)
    err_exit("hostsfile_clusterlist_get_all_nodes: nodeslen too smalll");

  itr = List_iterator_create(hosts);

  while ((node = list_next(itr)) && (i < numnodes))
    nodes[i++] = Strdup(node);

  if (i > numnodes)
    err_exit("hostsfile_clusterlist_get_all_nodes: iterator count error");

  List_iterator_destroy(itr);

  return numnodes;
}

int 
hostsfile_clusterlist_numnodes(void)
{
  assert(handle);

  return list_count(hosts);
}

int
hostsfile_clusterlist_node_in_cluster(char *node)
{
  void *ret;

  assert(handle && node);

  ret = list_find_first(l, (ListFindF)strcmp, node);

  return ((ret) ? 1: 0);
}

int
hostsfile_clusterlist_get_nodename(char *node, char *buf, int buflen)
{
  int len;

  assert(handle && node && buf && buflen > 0);

  len = strlen(node);

  if ((len + 1) > buflen)
    err_exit("hostsfile_clusterlist_get_nodename: buflen too small: %d %d",
	     len, buflen);

  strcpy(buf, node);

  return 0;
}

struct cerebrod_clusterlist_ops clusterlist_ops =
  {
    &hostsfile_clusterlist_init,
    &hostsfile_clusterlist_finish,
    &hostsfile_clusterlist_get_all_nodes,
    &hostsfile_clusterlist_numnodes,
    &hostsfile_clusterlist_node_in_cluster,
    &hostsfile_clusterlist_get_nodename,
  };


