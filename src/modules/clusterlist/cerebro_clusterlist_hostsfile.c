/*****************************************************************************\
 *  $Id: cerebro_clusterlist_hostsfile.c,v 1.23 2005-06-22 15:56:13 achu Exp $
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
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */

#include "cerebro/cerebro_clusterlist_module.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_error.h"

#include "cerebro_clusterlist_util.h"

#include "fd.h"
#include "list.h"

#define HOSTSFILE_CLUSTERLIST_MODULE_NAME "hostsfile"
#define HOSTSFILE_PARSE_BUFLEN            4096

/* 
 * hosts
 *
 * Store all of the hosts found in the hostsfile
 */
static List hosts = NULL;

/* 
 * _readline
 * 
 * read a line from the hostsfile.  Buffer guaranteed to be null
 * terminated.
 *
 * - fd - file descriptor to read from
 * - buf - buffer pointer
 * - buflen - buffer length
 *
 * Return amount of data read into the buffer, -1 on error
 */
static int
_readline(int fd, char *buf, unsigned int buflen)
{
  int len;

  if (fd <= 0)
    {
      cerebro_err_debug("%s(%s:%d): fd invalid",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!buf)
    {
      cerebro_err_debug("%s(%s:%d): buf null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!buflen)
    {
      cerebro_err_debug("%s(%s:%d): buflen invalid",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if ((len = fd_read_line(fd, buf, buflen)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): fd_read_line: %s", 
			__FILE__, __FUNCTION__, __LINE__,
			strerror(errno));
      return -1;
    }
  
  /* buflen - 1 b/c fd_read_line guarantees null termination */
  if (len >= (buflen-1))
    {
      cerebro_err_debug("%s(%s:%d): fd_read_line: line truncation",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  return len;
}

/* 
 * _remove_comments
 *
 * remove comments from the buffer
 *
 * - buf - buffer pointer
 * - buflen - buffer length
 *
 * Return length of buffer left after comments were removed, -1 on
 * error
 */
static int
_remove_comments(char *buf, int buflen)
{
  int i, comment_flag, lenleft;

  if (!buf)
    {
      cerebro_err_debug("%s(%s:%d): buf null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!strchr(buf, '#'))
    return buflen;

  i = 0;
  comment_flag = 0;
  lenleft = buflen;
  while (i < buflen)
    {
      if (comment_flag)
        {
          buf[i] = '\0';
          lenleft--;
        }

      if (buf[i] == '#')
        {
          buf[i] = '\0';
          comment_flag++;
          lenleft--;
        }
      i++;
    }

  return lenleft;
}

/* 
 * _remove_trailing_whitespace
 *
 * remove trailing whitespace from the buffer
 *
 * - buf - buffer pointer
 * - buflen - buffer length
 *
 * Return length of buffer left after trailing whitespace was removed,
 * -1 on error
 */
static int
_remove_trailing_whitespace(char *buf, int buflen)
{
  char *temp;
  
  if (!buf)
    {
      cerebro_err_debug("%s(%s:%d): buf null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

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
 * Return pointer to beginning of first non-whitespace char, NULL on
 * error
 */
static char *
_move_past_whitespace(char *buf)
{
  if (!buf)
    {
      cerebro_err_debug("%s(%s:%d): buf null", 
			__FILE__, __FUNCTION__, __LINE__);
      return NULL;
    }

  while (*buf != '\0' && isspace(*buf))
    buf++;

  return buf;
}

/* 
 * hostsfile_clusterlist_setup
 *
 * hostsfile clusterlist module setup function.  Open hostsfile, read
 * each line of the hostsfile, and save hosts into hosts list.
 */
static int 
hostsfile_clusterlist_setup(void)
{
  int len, fd = -1;
  char buf[HOSTSFILE_PARSE_BUFLEN];
  char *p;

  if (hosts)
    {
      cerebro_err_debug("%s(%s:%d): hosts non-null", 
			__FILE__, __FUNCTION__, __LINE__);
      return 0;
    }

  if (!(hosts = list_create((ListDelF)free)))
    {
      cerebro_err_debug("%s(%s:%d): list_create: %s", 
			__FILE__, __FUNCTION__, __LINE__, 
			strerror(errno));
      goto cleanup;
    }

  if ((fd = open(CEREBRO_CLUSTERLIST_HOSTSFILE_DEFAULT, O_RDONLY)) < 0)
    {
      cerebro_err_debug("hostsfile '%s' cannot be opened: %s", 
			CEREBRO_CLUSTERLIST_HOSTSFILE_DEFAULT, 
			strerror(errno));
      goto cleanup;
    }
 
  while ((len = _readline(fd, buf, HOSTSFILE_PARSE_BUFLEN)) > 0)
    {
      char *hostPtr;
      char *str;

      if (!(len = _remove_comments(buf, len)))
        continue;

      if (len < 0)
        goto cleanup;

      if (!(len = _remove_trailing_whitespace(buf, len)))
        continue;

      if (len < 0)
        goto cleanup;

      if (!(hostPtr = _move_past_whitespace(buf)))
        goto cleanup;

      if (hostPtr[0] == '\0')
        continue;

      if (strchr(hostPtr, ' ') || strchr(hostPtr, '\t'))
        {
          cerebro_err_debug("hostsfile parse error: "
			    "host contains whitespace");
          goto cleanup;
        }

      if (strlen(hostPtr) > CEREBRO_MAXNODENAMELEN)
        {
          cerebro_err_debug("hostsfile parse error: "
			    "nodename '%s' exceeds maximum length", 
			    hostPtr);
          goto cleanup;
        }
      
      /* Shorten hostname if necessary */
      if ((p = strchr(hostPtr, '.')))
        *p = '\0';

      if (!(str = strdup(hostPtr)))
        {
          cerebro_err_debug("%s(%s:%d): strdup: %s", 
			    __FILE__, __FUNCTION__, __LINE__,
			    strerror(errno));
          goto cleanup;
        }

      if (!list_append(hosts, str))
        {
          cerebro_err_debug("%s(%s:%d): list_append: %s", 
			    __FILE__, __FUNCTION__, __LINE__,
			    strerror(errno));
          goto cleanup;
        }
    }
  
  if (len < 0)
    goto cleanup;

  close(fd);
  return 0;

 cleanup:
  close(fd);
  if (hosts)
    list_destroy(hosts);
  hosts = NULL;
  return -1;
}

static int
hostsfile_clusterlist_cleanup(void)
{
  if (!hosts)
    return 0;

  list_destroy(hosts);
  hosts = NULL;

  return 0;
}

static int 
hostsfile_clusterlist_numnodes(void)
{
  if (!hosts)
    {
      cerebro_err_debug("%s(%s:%d): hosts null", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  return list_count(hosts);
}

static int
hostsfile_clusterlist_get_all_nodes(char ***nodes)
{
  char **nodelist = NULL;
  char *node;
  ListIterator itr = NULL;
  int numnodes, i = 0;

  if (!hosts)
    {
      cerebro_err_debug("%s(%s:%d): hosts null", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!nodes)
    {     
      cerebro_err_debug("%s(%s:%d): nodes null"
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!(numnodes = list_count(hosts)))
    return 0;

  if (!(itr = list_iterator_create(hosts)))
    {
      cerebro_err_debug("%s(%s:%d): list_iterator_create: %s", 
			__FILE__, __FUNCTION__, __LINE__,
			strerror(errno));
      goto cleanup;
    }

  if (!(nodelist = (char **)malloc(sizeof(char *) * (numnodes + 1))))
    {
      cerebro_err_debug("%s(%s:%d): malloc: %s", 
			__FILE__, __FUNCTION__, __LINE__,
			strerror(errno));
      goto cleanup;
    }
  memset(nodelist, '\0', sizeof(char *) * (numnodes + 1));

  while ((node = list_next(itr)) && (i < numnodes))
    {
      if (!(nodelist[i] = strdup(node)))
        {
          cerebro_err_debug("%s(%s:%d): strdup: %s", 
			    __FILE__, __FUNCTION__, __LINE__,
			    strerror(errno));
          goto cleanup;
        }
      i++;
    }

  if (i > numnodes)
    {
      cerebro_err_debug("%s(%s:%d): iterator count error",
			__FILE__, __FUNCTION__, __LINE__);
      goto cleanup;
    }

  list_iterator_destroy(itr);

  *nodes = nodelist;
  return numnodes;

 cleanup:
  if (itr)
    list_iterator_destroy(itr);
  if (nodelist)
    {
      int j;
      for (j = 0; j < i; j++)
        free(nodelist[j]);
      free(nodelist);
    }
  return -1;
}

static int
hostsfile_clusterlist_node_in_cluster(const char *node)
{
  char nodebuf[CEREBRO_MAXNODENAMELEN+1];
  char *nodePtr = NULL;
  void *ptr;

  if (!hosts)
    {
      cerebro_err_debug("%s(%s:%d): hosts null", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!node)
    {     
      cerebro_err_debug("%s(%s:%d): node null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  /* Shorten hostname if necessary */
  if (strchr(node, '.'))
    {
      char *p;

      memset(nodebuf, '\0', CEREBRO_MAXNODENAMELEN+1);
      strncpy(nodebuf, node, CEREBRO_MAXNODENAMELEN);
      p = strchr(nodebuf, '.');
      *p = '\0';
      nodePtr = nodebuf;
    }
  else
    nodePtr = (char *)node;

  ptr = list_find_first(hosts, (ListFindF)strcmp, nodePtr);

  return ((ptr) ? 1: 0);
}

static int
hostsfile_clusterlist_get_nodename(const char *node, 
				   char *buf, 
				   unsigned int buflen)
{
  char nodebuf[CEREBRO_MAXNODENAMELEN+1];
  char *nodePtr = NULL;

  if (!hosts)
    {
      cerebro_err_debug("%s(%s:%d): hosts null", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!node)
    {     
      cerebro_err_debug("%s(%s:%d): node null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!buf)
    {     
      cerebro_err_debug("%s(%s:%d): buf null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!buflen)
    {
      cerebro_err_debug("%s(%s:%d): buflen invalid",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  /* Shorten hostname if necessary */
  if (strchr(node, '.'))
    {
      char *p;

      memset(nodebuf, '\0', CEREBRO_MAXNODENAMELEN+1);
      strncpy(nodebuf, node, CEREBRO_MAXNODENAMELEN);
      p = strchr(nodebuf, '.');
      *p = '\0';
      nodePtr = nodebuf;
    }
  else
    nodePtr = (char *)node;

  return cerebro_clusterlist_copy_nodename(nodePtr, 
                                           buf, 
                                           buflen);
}

struct cerebro_clusterlist_module_info clusterlist_module_info =
  {
    HOSTSFILE_CLUSTERLIST_MODULE_NAME,
    &hostsfile_clusterlist_setup,
    &hostsfile_clusterlist_cleanup,
    &hostsfile_clusterlist_numnodes,
    &hostsfile_clusterlist_get_all_nodes,
    &hostsfile_clusterlist_node_in_cluster,
    &hostsfile_clusterlist_get_nodename,
  };
