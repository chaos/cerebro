/*****************************************************************************\
 *  $Id: cerebro_clusterlist_hostsfile.c,v 1.2 2005-04-21 17:59:15 achu Exp $
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
#include <assert.h>
#include <errno.h>

#include "cerebro_defs.h"

#include "cerebro_clusterlist_module.h"
#include "cerebro_clusterlist_util.h"
#include "error.h"
#include "fd.h"
#include "wrappers.h"

#define HOSTSFILE_CLUSTERLIST_MODULE_NAME "hostsfile"
#define HOSTSFILE_PARSE_BUFLEN            4096

/* 
 * hosts
 *
 * list of all hosts
 */
List hosts = NULL;

/*  
 * hostsfile_file
 *
 * hostsfile database
 */
char *hostsfile_file = NULL;

/* 
 * hostsfile_clusterlist_parse_options
 *
 * parse options for the hostsfile clusterlist module
 */
int
hostsfile_clusterlist_parse_options(char **options)
{
  assert(!hosts);

  if (options)
    cerebro_clusterlist_parse_filename(options, 
                                        &hostsfile_file, 
                                        HOSTSFILE_CLUSTERLIST_MODULE_NAME);

  return 0;
}

/* 
 * _readline
 * 
 * read a line from the hostsfile.  Buffer guaranteed to be null terminated.
 *
 * - fd - file descriptor to read from
 * - buf - buffer pointer
 * - buflen - buffer length
 *
 * Return amount of data read into the buffer
 */
static int
_readline(int fd, char *buf, int buflen)
{
  int ret;

  assert(buf);

  if ((ret = fd_read_line(fd, buf, buflen)) < 0)
    err_exit("%s(%s:%d): %s clusterlist module: fd_read_line: %s", 
             __FILE__, __FUNCTION__, __LINE__,
             HOSTSFILE_CLUSTERLIST_MODULE_NAME, strerror(errno));
  
  /* buflen - 1 b/c fd_read_line guarantees null termination */
  if (ret >= (buflen-1))
    err_exit("%s(%s:%d): %s clusterlist module: fd_read_line: line truncation",
             __FILE__, __FUNCTION__, __LINE__, 
             HOSTSFILE_CLUSTERLIST_MODULE_NAME);

  return ret;
}

/* 
 * _remove_comments
 *
 * remove comments from the buffer
 *
 * - buf - buffer pointer
 * - buflen - buffer length
 *
 * Return length of buffer left after comments were removed
 */
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
      i++;
    }

  return retlen;
}

/* 
 * _remove_trailing_whitespace
 *
 * remove trailing whitespace from the buffer
 *
 * - buf - buffer pointer
 * - buflen - buffer length
 *
 * Return length of buffer left after trailing whitespace was removed
 */
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

/* 
 * _move_past_whitespace
 *
 * move past whitespace at the beginning of the buffer
 *
 * - buf - buffer pointer
 *
 * Return pointer to beginning of first non-whitespace char
 */
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

/* 
 * hostsfile_clusterlist_init
 *
 * hostsfile clusterlist module init function.  Open hostsfile, read
 * each line of the hostsfile, and save hosts into hosts list.
 */
int 
hostsfile_clusterlist_init(void)
{
  int fd, len;
  char buf[HOSTSFILE_PARSE_BUFLEN];
  char *file;

  assert(!hosts);

  hosts = List_create((ListDelF)_Free);

  if (hostsfile_file)
    file = hostsfile_file;
  else
    file = CEREBRO_CLUSTERLIST_HOSTSFILE_DEFAULT;

  if ((fd = open(file, O_RDONLY)) < 0)
    err_exit("hostsfile clusterlist file '%s' cannot be opened", file);
 
  while ((len = _readline(fd, buf, HOSTSFILE_PARSE_BUFLEN)) > 0)
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

      if (strlen(hostPtr) > CEREBRO_MAXNODENAMELEN)
        err_exit("hostsfile clusterlist parse error: "
                 "nodename '%s' exceeds maximum length", 
                 hostPtr);
      
      str = Strdup(hostPtr);
      List_append(hosts, str);
    }
  
  close(fd);
  return 0;
}

/* 
 * hostsfile_clusterlist_finish
 *
 * hostsfile clusterlist module finish function
 */
int
hostsfile_clusterlist_finish(void)
{
  assert(hosts);

  List_destroy(hosts);
  Free(hostsfile_file);
  hosts = NULL;
  hostsfile_file = NULL;

  return 0;
}

/*
 * hostsfile_clusterlist_get_all_nodes
 *
 * hostsfile clusterlist module get all nodes function
 */
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
    err_exit("%s(%s:%d): %s clusterlist module: nodeslen too small",
             __FILE__, __FUNCTION__, __LINE__,
             HOSTSFILE_CLUSTERLIST_MODULE_NAME);

  itr = List_iterator_create(hosts);

  while ((node = list_next(itr)) && (i < numnodes))
    nodes[i++] = Strdup(node);

  if (i > numnodes)
    err_exit("%s(%s:%d): %s clusterlist module: iterator count error",
             __FILE__, __FUNCTION__, __LINE__,
             HOSTSFILE_CLUSTERLIST_MODULE_NAME);

  List_iterator_destroy(itr);

  return numnodes;
}

/*
 * hostsfile_clusterlist_numnodes
 *
 * hostsfile clusterlist module numnodes function
 */
int 
hostsfile_clusterlist_numnodes(void)
{
  assert(hosts);

  return list_count(hosts);
}

/*
 * hostsfile_clusterlist_node_in_cluster
 *
 * hostsfile clusterlist module node in cluster function
 */
int
hostsfile_clusterlist_node_in_cluster(char *node)
{
  void *ret;

  assert(hosts);
  assert(node);

  ret = list_find_first(hosts, (ListFindF)strcmp, node);

  return ((ret) ? 1: 0);
}

/*
 * hostsfile_clusterlist_get_nodename
 *
 * hostsfile clusterlist module get nodename function
 */
int
hostsfile_clusterlist_get_nodename(char *node, char *buf, unsigned int buflen)
{
  assert(hosts);
  assert(node);
  assert(buf);

  return cerebro_clusterlist_copy_nodename(node, 
                                            buf, 
                                            buflen, 
                                            HOSTSFILE_CLUSTERLIST_MODULE_NAME);
}

#if WITH_STATIC_MODULES
struct cerebro_clusterlist_module_info hostsfile_clusterlist_module_info =
#else /* !WITH_STATIC_MODULES */
struct cerebro_clusterlist_module_info clusterlist_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    HOSTSFILE_CLUSTERLIST_MODULE_NAME,
    &hostsfile_clusterlist_parse_options,
    &hostsfile_clusterlist_init,
    &hostsfile_clusterlist_finish,
    &hostsfile_clusterlist_get_all_nodes,
    &hostsfile_clusterlist_numnodes,
    &hostsfile_clusterlist_node_in_cluster,
    &hostsfile_clusterlist_get_nodename,
  };
