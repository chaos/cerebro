/*****************************************************************************\
 *  $Id: cerebrod_clusterlist_hostsfile.c,v 1.3 2005-03-15 23:14:39 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#include <ctype.h>
#endif /* STDC_HEADERS */
#if HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */
#include <assert.h>
#include <errno.h>

#include "cerebrod.h"
#include "cerebrod_clusterlist.h"
#include "error.h"
#include "fd.h"
#include "wrappers.h"

List hosts = NULL;
char *hostsfile_file = NULL;

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

static int
_readline(int fd, char *buf, int buflen)
{
  int ret;

  assert(buf);

  if ((ret = fd_read_line(fd, buf, buflen)) < 0)
    err_exit("_readline: fd_read_line: %s", strerror(errno));

  /* buflen - 1 b/c fd_read_line guarantees null termination */
  if (ret >= (buflen-1))
    err_exit("_readline: fd_read_line: line truncation");

  return ret;
}

static int
_remove_comments(char *buf, int buflen)
{
  int i, comment_flag, retlen;

  assert(buf);

  if (strchr(buf, '#') == NULL)
    return buflen;

  i = 0;
  comment_flag = 0;
  retlen = buflen;
  while (i < buflen)
    {
      if (comment_flag)
        {
          buf[i] = '\0';
          retlen--;
        }

      if (buf[i] == '#')
        {
          buf[i] = '\0';
          comment_flag++;
          retlen--;
        }
    }

  return retlen;
}

static int
_remove_trailing_whitespace(char *buf, int buflen)
{
  char *temp;
  
  assert(buf);

  temp = buf + buflen;
  for (--temp; temp >= buf; temp--) 
    {
      if (isspace(*temp))
        *temp = '\0';
      else
        break;
      buflen--;
    }

  return buflen;
}

static char *
_move_past_whitespace(char *buf)
{
  assert(buf);

  while (*buf != '\0' && isspace(*buf))
    buf++;

  if (*buf == '\0')
    return NULL;

  return buf;
}

int 
hostsfile_clusterlist_init(char *cmdline)
{
  int fd, len;
  char buf[CEREBROD_PARSE_BUFLEN];

  assert(!hosts);

  if (cmdline)
    {
      char **argv;
      int argc;

      argv_create(cmdline, "", &argc, &argv);

      _cmdline_parse(argc, argv);

      argv_destroy(argv);
    }

  hosts = List_create((ListDelF)_Free);

  hostsfile_file = CEREBROD_CLUSTERLIST_HOSTSFILE_DEFAULT;
  if ((fd = open(hostsfile_file, O_RDONLY)) < 0)
    err_exit("hostsfile clusterlist file '%s' cannot be opened", hostsfile_file);

  while ((len = _readline(fd, buf, CEREBROD_PARSE_BUFLEN)) > 0)
    {
      char *hostPtr;
      char *str;

      if ((len = _remove_comments(buf, len)) == 0)
        continue;

      if ((len = _remove_trailing_whitespace(buf, len)) == 0)
        continue;

      if ((hostPtr = _move_past_whitespace(buf)) == NULL)
        continue;

      if (hostPtr[0] == '\0')
        continue;

      if (strchr(hostPtr, ' ') || strchr(hostPtr, '\t'))
        err_exit("hostsfile clusterlist parse error: host contains whitespace");

      if (strlen(hostPtr) > MAXHOSTNAMELEN)
        err_exit("hostsfile clusterlist parse error: hostname '%s' exceeds "
                 "maximum length", hostPtr);
      
      str = Strdup(hostPtr);
      List_append(hosts, str);
    }
  
  close(fd);
  return 0;
}

int
hostsfile_clusterlist_finish(void)
{
  assert(hosts);

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

  assert(hosts);
  assert(nodes);

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
  assert(hosts);

  return list_count(hosts);
}

int
hostsfile_clusterlist_node_in_cluster(char *node)
{
  void *ret;

  assert(hosts);
  assert(node);

  ret = list_find_first(hosts, (ListFindF)strcmp, node);

  return ((ret) ? 1: 0);
}

int
hostsfile_clusterlist_get_nodename(char *node, char *buf, int buflen)
{
  int len;

  assert(hosts);
  assert(node);
  assert(buf);
  assert(buflen > 0);

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


