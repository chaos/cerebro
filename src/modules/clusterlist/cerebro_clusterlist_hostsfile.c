/*****************************************************************************\
 *  $Id: cerebro_clusterlist_hostsfile.c,v 1.40 2010-02-02 01:01:21 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2015 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2005-2007 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <http://www.llnl.gov/linux/cerebro/>.
 *
 *  Cerebro is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  Cerebro is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with Cerebro.  If not, see <http://www.gnu.org/licenses/>.
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

#include "debug.h"
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
 * hostsfile_clusterlist_interface_version
 *
 * hostsfile clusterlist module interface_version function
 */
static int 
hostsfile_clusterlist_interface_version(void)
{
  return CEREBRO_CLUSTERLIST_INTERFACE_VERSION;
}

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

  if (fd <= 0 || !buf || !buflen)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  if ((len = fd_read_line(fd, buf, buflen)) < 0)
    {
      CEREBRO_ERR(("fd_read_line: %s", strerror(errno)));
      return -1;
    }
  
  /* buflen - 1 b/c fd_read_line guarantees null termination */
  if (len >= (buflen-1))
    {
      CEREBRO_DBG(("fd_read_line: line truncation"));
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
      CEREBRO_DBG(("invalid parameters"));
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
      CEREBRO_DBG(("invalid parameters"));
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
      CEREBRO_DBG(("invalid parameters"));
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
      CEREBRO_DBG(("hosts non-null"));
      return 0;
    }

  if (!(hosts = list_create((ListDelF)free)))
    {
      CEREBRO_ERR(("list_create: %s", strerror(errno)));
      goto cleanup;
    }

  if ((fd = open(CEREBRO_CLUSTERLIST_HOSTSFILE_DEFAULT, O_RDONLY)) < 0)
    {
      CEREBRO_ERR(("hostsfile '%s' cannot be opened: %s", 
                   CEREBRO_CLUSTERLIST_HOSTSFILE_DEFAULT, strerror(errno)));
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
          cerebro_err_output("hostsfile host contains whitespace");
          goto cleanup;
        }

      if (strlen(hostPtr) > CEREBRO_MAX_NODENAME_LEN)
        {
          cerebro_err_output("hostsfile node '%s' exceeds max length", hostPtr);
          goto cleanup;
        }
      
      /* Shorten hostname if necessary */
      if ((p = strchr(hostPtr, '.')))
        *p = '\0';

      if (!(str = strdup(hostPtr)))
        {
          CEREBRO_ERR(("strdup: %s", strerror(errno)));
          goto cleanup;
        }

      if (!list_append(hosts, str))
        {
          CEREBRO_ERR(("list_append: %s", strerror(errno)));
          goto cleanup;
        }
    }
  
  if (len < 0)
    goto cleanup;

  /* ignore potential error, just return result */
  close(fd);
  return 0;

 cleanup:
  /* ignore potential error, just return result */
  close(fd);
  if (hosts)
    list_destroy(hosts);
  hosts = NULL;
  return -1;
}

/*
 * hostsfile_clusterlist_cleanup
 *
 * hostsfile clusterlist module cleanup function
 */
static int
hostsfile_clusterlist_cleanup(void)
{
  if (!hosts)
    return 0;

  list_destroy(hosts);
  hosts = NULL;

  return 0;
}

/*
 * hostsfile_clusterlist_numnodes
 *
 * hostsfile clusterlist module numnodes function
 */
static int 
hostsfile_clusterlist_numnodes(void)
{
  if (!hosts)
    {
      CEREBRO_DBG(("hosts null"));
      return -1;
    }

  return list_count(hosts);
}

/*
 * hostsfile_clusterlist_get_all_nodes
 *
 * hostsfile clusterlist module get_all_nodes function
 */
static int
hostsfile_clusterlist_get_all_nodes(char ***nodes)
{
  char **nodelist = NULL;
  char *node;
  ListIterator itr = NULL;
  int numnodes, i = 0;

  if (!hosts)
    {
      CEREBRO_DBG(("hosts null"));
      return -1;
    }

  if (!nodes)
    {     
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  if (!(numnodes = list_count(hosts)))
    return 0;

  if (!(itr = list_iterator_create(hosts)))
    {
      CEREBRO_ERR(("list_iterator_create: %s", strerror(errno)));
      goto cleanup;
    }

  if (!(nodelist = (char **)malloc(sizeof(char *) * (numnodes + 1))))
    {
      CEREBRO_ERR(("malloc: %s", strerror(errno)));
      goto cleanup;
    }
  memset(nodelist, '\0', sizeof(char *) * (numnodes + 1));

  while ((node = list_next(itr)) && (i < numnodes))
    {
      if (!(nodelist[i] = strdup(node)))
        {
          CEREBRO_ERR(("strdup: %s", strerror(errno)));
          goto cleanup;
        }
      i++;
    }

  if (i > numnodes)
    {
      CEREBRO_DBG(("iterator count error"));
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

/*
 * list_strcmp
 *
 * strcmp for list data structure 
 */
static int
list_strcmp(void *x, void *key)
{
  if (!x || !key)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  if (!strcmp((char *)x, (char *)key))
    return 1;
  return 0;
}

/*
 * hostsfile_clusterlist_node_in_cluster
 *
 * hostsfile clusterlist module node_in_cluster function
 */
static int
hostsfile_clusterlist_node_in_cluster(const char *node)
{
  char nodebuf[CEREBRO_MAX_NODENAME_LEN+1];
  char *nodePtr = NULL;
  void *ptr;

  if (!hosts)
    {
      CEREBRO_DBG(("hosts null"));
      return -1;
    }

  if (!node)
    {     
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  /* Shorten hostname if necessary */
  if (strchr(node, '.'))
    {
      char *p;

      memset(nodebuf, '\0', CEREBRO_MAX_NODENAME_LEN+1);
      strncpy(nodebuf, node, CEREBRO_MAX_NODENAME_LEN);
      p = strchr(nodebuf, '.');
      *p = '\0';
      nodePtr = nodebuf;
    }
  else
    nodePtr = (char *)node;

  ptr = list_find_first(hosts, list_strcmp, nodePtr);

  return ((ptr) ? 1: 0);
}

/*
 * hostsfile_clusterlist_get_nodename
 *
 * hostsfile clusterlist module get_nodename function
 */
static int
hostsfile_clusterlist_get_nodename(const char *node, 
                                   char *buf, 
                                   unsigned int buflen)
{
  char nodebuf[CEREBRO_MAX_NODENAME_LEN+1];
  char *nodePtr = NULL;

  if (!hosts)
    {
      CEREBRO_DBG(("hosts null"));
      return -1;
    }

  if (!node || !buf || !buflen)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  /* Shorten hostname if necessary */
  if (strchr(node, '.'))
    {
      char *p;

      memset(nodebuf, '\0', CEREBRO_MAX_NODENAME_LEN+1);
      strncpy(nodebuf, node, CEREBRO_MAX_NODENAME_LEN);
      p = strchr(nodebuf, '.');
      *p = '\0';
      nodePtr = nodebuf;
    }
  else
    nodePtr = (char *)node;

  return cerebro_copy_nodename(nodePtr, buf, buflen);
}

#if WITH_STATIC_MODULES
struct cerebro_clusterlist_module_info hostsfile_clusterlist_module_info =
#else  /* !WITH_STATIC_MODULES */
struct cerebro_clusterlist_module_info clusterlist_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    HOSTSFILE_CLUSTERLIST_MODULE_NAME,
    &hostsfile_clusterlist_interface_version,
    &hostsfile_clusterlist_setup,
    &hostsfile_clusterlist_cleanup,
    &hostsfile_clusterlist_numnodes,
    &hostsfile_clusterlist_get_all_nodes,
    &hostsfile_clusterlist_node_in_cluster,
    &hostsfile_clusterlist_get_nodename,
  };
