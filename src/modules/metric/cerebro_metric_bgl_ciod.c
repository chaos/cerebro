/*****************************************************************************\
 *  $Id: cerebro_metric_bgl_ciod.c,v 1.12 2005-09-14 22:34:19 achu Exp $
 *****************************************************************************
 *  Copyright (C) 2005 The Regents of the University of California.
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
 *  with Genders; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <errno.h>

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_metric_module.h"

#include "debug.h"

#include "conffile.h"
#include "fd.h"

#include "network_util.h"

#define BGL_CIOD_METRIC_MODULE_NAME      "bgl_ciod"
#define BGL_CIOD_METRIC_NAME             "bgl_ciod"
#define BGL_CIOD_HOSTNAME                "localhost"
#define BGL_CIOD_PORT                    7000
#define BGL_CIOD_PERIOD_DEFAULT          60
#define BGL_CIOD_FAILURE_MAX_DEFAULT     3
#define BGL_CIOD_CONNECT_TIMEOUT_DEFAULT 5
#define BGL_CIOD_MAX_SOCKETS             16
#define BGL_CIOD_SIGNATURE               "ciod"

#define BGL_PROC_DIR                     "/proc"
#define BGL_EXE_FILE                     "exe"
#define BGL_PROC_FD_DIR                  "fd"
#define BGL_TCP_FILE                     "net/tcp"
#define BGL_MAX_PID_LEN                  16
#define BGL_DATA_BUFLEN                  4096
#define BGL_PORT_BUFLEN                  64
#define BGL_INODE_BUFLEN                 64
#define BGL_SOCKET_INODE                 "socket"

/*
 * bgl_ciod_state
 *
 * cached system bgl_ciod state
 */
static u_int32_t bgl_ciod_state = 0;

/* 
 * bgl_ciod_failures
 *
 * counts consecutive connection failures
 */
static unsigned int bgl_ciod_failures = 0;

/*  
 * bgl_ciod_was_up
 *
 * Flag did indicates if the ciod daemon was at some point detected up
 */
static int bgl_ciod_was_up = 0;

/* 
 * bgl_ciod_period
 *
 * the monitoring period.
 */
static unsigned int bgl_ciod_period = BGL_CIOD_PERIOD_DEFAULT;

/* 
 * bgl_ciod_failure_max
 *
 * count of consecutive failures at which we determine the ciod daemon
 * is in fact down.
 */
static unsigned int bgl_ciod_failure_max = BGL_CIOD_FAILURE_MAX_DEFAULT;

/* 
 * bgl_ciod_connect_timeout
 *
 * count of consecutive failures at which we determine the ciod daemon
 * is in fact down.
 */
static unsigned int bgl_ciod_connect_timeout = BGL_CIOD_CONNECT_TIMEOUT_DEFAULT;

/* 
 * bgl_ciod_sockets
 * bgl_ciod_sockets_len
 *
 * List of paths to ciod sockets
 */
static char bgl_ciod_sockets[BGL_CIOD_MAX_SOCKETS][CEREBRO_MAX_PATH_LEN+1];
static char bgl_ciod_inode[BGL_INODE_BUFLEN + 1];
static int bgl_ciod_sockets_len = 0;

/*
 * _readline
 *
 * read a line.  Buffer guaranteed to be null terminated.
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
      CEREBRO_DBG(("fd_read_line: %s", strerror(errno)));
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
 * _parse_net_tcp_fields
 *
 * Parse and store fields on success
 *
 * Returns 0 on successful parse, -1 on error
 */
static int
_parse_net_tcp_fields(char *buf,
                      unsigned int buflen,
                      char **sl,
                      char **local_address,
                      char **local_address_port,
                      char **rem_address,
                      char **rem_address_port,
                      char **st,
                      char **tx_queue,
                      char **rx_queue,
                      char **tr,
                      char **tm_when,
                      char **retrnsmt,
                      char **uid,
                      char **timeout,
                      char **inode)
{
  char *delim = ": \t\n\0";
  char *ptr, *ptrptr;

  if (!buf || !buflen)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  if (!(ptr = strtok_r(buf, delim, &ptrptr)))
    goto done;
  else if (sl)
    *sl = ptr;

  if (!(ptr = strtok_r(NULL, delim, &ptrptr)))
    goto done;
  else if (local_address)
    *local_address = ptr;

  if (!(ptr = strtok_r(NULL, delim, &ptrptr)))
    goto done;
  else if (local_address_port)
    *local_address_port = ptr;

  if (!(ptr = strtok_r(NULL, delim, &ptrptr)))
    goto done;
  else if (rem_address)
    *rem_address = ptr;

  if (!(ptr = strtok_r(NULL, delim, &ptrptr)))
    goto done;
  else if (rem_address_port)
    *rem_address_port = ptr;

  if (!(ptr = strtok_r(NULL, delim, &ptrptr)))
    goto done;
  else if (st)
    *st = ptr;

  if (!(ptr = strtok_r(NULL, delim, &ptrptr)))
    goto done;
  else if (tx_queue)
    *tx_queue = ptr;

  if (!(ptr = strtok_r(NULL, delim, &ptrptr)))
    goto done;
  else if (rx_queue)
    *rx_queue = ptr;

  if (!(ptr = strtok_r(NULL, delim, &ptrptr)))
    goto done;
  else if (tr)
    *tr = ptr;

  if (!(ptr = strtok_r(NULL, delim, &ptrptr)))
    goto done;
  else if (tm_when)
    *tm_when = ptr;

  if (!(ptr = strtok_r(NULL, delim, &ptrptr)))
    goto done;
  else if (retrnsmt)
    *retrnsmt = ptr;

  if (!(ptr = strtok_r(NULL, delim, &ptrptr)))
    goto done;
  else if (uid)
    *uid = ptr;

  if (!(ptr = strtok_r(NULL, delim, &ptrptr)))
    goto done;
  else if (timeout)
    *timeout = ptr;

  if (!(ptr = strtok_r(NULL, delim, &ptrptr)))
    goto done;
  else if (inode)
    *inode = ptr;

  /* Ensure '\0' after inode field */
  ptr = strtok_r(NULL, delim, &ptrptr);

 done:
  return 0;
}

/*
 * _find_tcp_inode
 *
 * Find the inode of the tcp socket with a particular port
 *
 * Returns 1 if found, 0 if not, -1 on error
 */
static int
_find_tcp_inode(unsigned int port, char *inode_buf, unsigned int inode_buflen)
{
  char filebuf[CEREBRO_MAX_PATH_LEN+1];
  char linebuf[BGL_DATA_BUFLEN+1];
  char portbuf[BGL_PORT_BUFLEN+1];
  int len, fd = -1, rv = -1;

  if (!port || !inode_buf || !inode_buflen)
    {
      CEREBRO_DBG(("invalid parameters"));
      goto cleanup;
    }

  /* Calculate the port in hex */
  memset(portbuf, '\0', BGL_PORT_BUFLEN+1);
  snprintf(portbuf, BGL_PORT_BUFLEN, "%X", port);

  memset(filebuf, '\0', CEREBRO_MAX_PATH_LEN+1);
  snprintf(filebuf, CEREBRO_MAX_PATH_LEN, "%s/%s",
           BGL_PROC_DIR, BGL_TCP_FILE);

  if ((fd = open(filebuf, O_RDONLY)) < 0)
    {
      CEREBRO_DBG(("open: %s", strerror(errno)));
      goto cleanup;
    }

  /* Read and skip over the first line of column descriptions */
  if (_readline(fd, linebuf, BGL_DATA_BUFLEN) < 0)
    goto cleanup;

  while ((len = _readline(fd, linebuf, BGL_DATA_BUFLEN)) > 0)
    {
      char *local_address_port = NULL, *inode = NULL;

      if (_parse_net_tcp_fields(linebuf,
                                len,
                                NULL,
                                NULL,
                                &local_address_port,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                &inode) < 0)
        goto cleanup;

      if (!strcmp(portbuf, local_address_port))
        {
          if (inode_buflen > strlen(inode))
            {
              strcpy(inode_buf, inode);
              rv = 0;
              break;
            }
        }
    }

  if (len < 0)
    goto cleanup;

 cleanup:
  close(fd);
  return rv;
}

/*
 * _is_pid
 *
 * Determines if the string passed in is a pid
 *
 * Returns 1 if true, 0 if false, -1 on error
 */
static int
_is_pid(char *str)
{
  if (!str)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  /* First char cannot be a zero */
  if (!(*str >= '1' && *str <= '9'))
    return 0;
  str++;

  while (str && *str != '\0')
    {
      if (!(*str >= '0' && *str <= '9'))
        return 0;
      str++;
    }

  return 1;
}

/*
 * _find_processes
 *
 * Find processes matching the specified string.
 *
 * Returns number of matches on success and array of pids.  Return -1
 * on error.  Will return -1 on overflow.
 *
 * Note: Array of pids required because there may be multiple
 * processes matching the name or multiple threads within a process
 * (i.e. on Linux 2.4).
 */
static int
_find_processes(char *str, char pids[][BGL_MAX_PID_LEN], unsigned int pids_len)
{
  DIR *dir = NULL;
  struct dirent *dirent;
  int matches = -1, count = 0;

  if (!str || !pids || !pids_len)
    {
      CEREBRO_DBG(("invalid parameters"));
      goto cleanup;
    }

  if (!(dir = opendir(BGL_PROC_DIR)))
    {
      CEREBRO_DBG(("opendir: %s", strerror(errno)));
      goto cleanup;
    }

  /*
   * Significant amounts of code will accept errors b/c there is no
   * locking being done here.  For example, a process could die in
   * between a readdir() and the attempt to read something in that
   * directory.
   */

  while ((dirent = readdir(dir)))
    {
      char filebuf[CEREBRO_MAX_PATH_LEN+1];
      char databuf[CEREBRO_MAX_PATH_LEN+1];
      struct stat statbuf;
      int rv;
      
      if (!_is_pid(dirent->d_name))
        continue;
      
      memset(filebuf, '\0', CEREBRO_MAX_PATH_LEN+1);
      snprintf(filebuf, CEREBRO_MAX_PATH_LEN, "%s/%s/%s",
               BGL_PROC_DIR, dirent->d_name, BGL_EXE_FILE);
      
      if (stat(filebuf, &statbuf) < 0)
        continue;
      
      memset(databuf, '\0', CEREBRO_MAX_PATH_LEN+1);
      if ((rv = readlink(filebuf, databuf, CEREBRO_MAX_PATH_LEN)) < 0)
        {
          /* EINVAL often occurs due to '.' or '..' directory entries */
          if (errno != EINVAL)
            CEREBRO_DBG(("readlink: %s", strerror(errno)));
          continue;
        }
      
      if (strstr(databuf, str))
        {
          if (count >= pids_len)
            goto cleanup;
          
          strncpy(pids[count], dirent->d_name, BGL_MAX_PID_LEN);
          pids[count][BGL_MAX_PID_LEN-1] = '\0';
          count++;
        }
    }

  matches = count;

 cleanup:
  closedir(dir);
  return matches;
}

/* 
 * _contains_inode_socket
 *
 * Check if the path contains the inode socket
 * 
 * Returns 1 on yes, 0 on no, -1 on error
 */
static int
_contains_inode_socket(char *path, char *inode)
{
  if (!path || !inode)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  if (strstr(path, BGL_SOCKET_INODE))
    {
      /* Format is socket:[inode] */
      char *p;

      if (!(p = strchr(inode, ']')))
        return 0;
      *p = '\0';

      if (!(p = strchr(inode, '[')))
        return 0;
      p++;

      if (!strcmp(p, inode))
        return 1;
    }

  return 0; 
}


/*
 * _find_inode_socket
 *
 * Determine if the process with the specified pid contains a socket
 * with the specified inode.
 *
 * Returns 1 on match and copies path into buffer, 0 if not, -1 on
 * error
 */
static int
_find_inode_socket(char *pid, 
                   char *inode, 
                   char *path_buf, 
                   unsigned int path_buflen)
{
  char fdpathbuf[CEREBRO_MAX_PATH_LEN+1];
  DIR *dir = NULL;
  struct dirent *dirent;
  int found = -1;

  if (!pid || !inode || !path_buf || !path_buflen)
    {
      CEREBRO_DBG(("invalid parameters"));
      goto cleanup;
    }

  memset(fdpathbuf, '\0', CEREBRO_MAX_PATH_LEN+1);
  snprintf(fdpathbuf, CEREBRO_MAX_PATH_LEN, "%s/%s/%s",
           BGL_PROC_DIR, pid, BGL_PROC_FD_DIR);

  if (!(dir = opendir(fdpathbuf)))
    {
      CEREBRO_DBG(("opendir: %s", strerror(errno)));
      goto cleanup;
    }

  while ((dirent = readdir(dir)))
    {
      char filebuf[CEREBRO_MAX_PATH_LEN+1];
      char databuf[CEREBRO_MAX_PATH_LEN+1];
      struct stat statbuf;
      int len, rv;

      memset(filebuf, '\0', CEREBRO_MAX_PATH_LEN+1);
      len = snprintf(filebuf, CEREBRO_MAX_PATH_LEN, "%s/%s/%s/%s",
                     BGL_PROC_DIR, pid, BGL_PROC_FD_DIR, dirent->d_name);

      if (stat(filebuf, &statbuf) < 0)
        continue;

      memset(databuf, '\0', CEREBRO_MAX_PATH_LEN+1);
      if ((rv = readlink(filebuf, databuf, CEREBRO_MAX_PATH_LEN)) < 0)
        {
          /* EINVAL often occurs due to '.' or '..' directory entries */
          if (errno != EINVAL)
            CEREBRO_DBG(("readlink: %s", strerror(errno)));
          continue;
        }

      if ((rv = _contains_inode_socket(databuf, inode)) < 0)
        continue;

      if (rv)
        {
          if (len < path_buflen)
            {
              strcpy(path_buf, filebuf);
              found = 1;
              break;
            }
        }
    }
  
  if (found < 0)
    found = 0;

 cleanup:
  closedir(dir);
  return found;
}

/* 
 * _bgl_socket_paths_setup
 *
 * Setup bgl socket paths
 */
static void
_bgl_socket_paths_setup(void)
{
  char inode[BGL_INODE_BUFLEN + 1];
  char pids[BGL_CIOD_MAX_SOCKETS][BGL_MAX_PID_LEN];
  int i, pids_len, sockets_count = 0;

  if (bgl_ciod_sockets_len)
    return;

  memset(inode, '\0', BGL_INODE_BUFLEN+1);
  if (_find_tcp_inode(BGL_CIOD_PORT, inode, BGL_INODE_BUFLEN) <= 0)
    return;

  if ((pids_len = _find_processes(BGL_CIOD_SIGNATURE, 
                                  pids, 
                                  BGL_CIOD_MAX_SOCKETS)) <= 0)
    return;

  for (i = 0; i < pids_len; i++)
    {
      char pathbuf[CEREBRO_MAX_PATH_LEN+1];
      int rv;

      memset(pathbuf, '\0', CEREBRO_MAX_PATH_LEN+1);
      if ((rv = _find_inode_socket(pids[i], 
                                   inode,
                                   pathbuf,
                                   CEREBRO_MAX_PATH_LEN)) < 0)
        return;

      if (rv)
        {
          strcpy(bgl_ciod_sockets[sockets_count], pathbuf);
          sockets_count++;
        }
    }

  if (sockets_count > 0)
    {
      strcpy(bgl_ciod_inode, inode);
      bgl_ciod_sockets_len = sockets_count;
    }
}

/*
 * bgl_ciod_metric_setup
 *
 * bgl_ciod metric module setup function.
 */
static int
bgl_ciod_metric_setup(void)
{
  int period = 0, period_flag = 0, failure_max = 0, failure_max_flag = 0, 
    connect_timeout = 0, connect_timeout_flag = 0;

  struct conffile_option options[] =
    {
      {
        "cerebro_bgl_ciod_period",
        CONFFILE_OPTION_INT,
        -1,
        conffile_int,
        1,
        0,
        &period_flag,
        &period,
        0
      },
      {
        "cerebro_bgl_ciod_failure_max",
        CONFFILE_OPTION_INT,
        -1,
        conffile_int,
        1,
        0,
        &failure_max_flag,
        &failure_max,
        0
      },
      {
        "cerebro_bgl_ciod_connect_timeout",
        CONFFILE_OPTION_INT,
        -1,
        conffile_int,
        1,
        0,
        &connect_timeout_flag,
        &connect_timeout,
        0
      },
    };
  conffile_t cf = NULL;
  int num;

  /* 
   * If any of this fails, who cares, just move on.
   */

  if (!(cf = conffile_handle_create()))
    {
      CEREBRO_DBG(("conffile_handle_create"));
      goto cleanup;
    }

  num = sizeof(options)/sizeof(struct conffile_option);
  if (conffile_parse(cf, BGL_CIOD_CONFIG_FILE, options, num, NULL, 0, 0) < 0)
    {
      char buf[CONFFILE_MAX_ERRMSGLEN];

      /* Its not an error if the configuration file doesn't exist */
      if (conffile_errnum(cf) == CONFFILE_ERR_EXIST)
        goto cleanup;

      if (conffile_errmsg(cf, buf, CONFFILE_MAX_ERRMSGLEN) < 0)
        CEREBRO_DBG(("conffile_parse: %d", conffile_errnum(cf)));
      else
        CEREBRO_DBG(("conffile_parse: %s", buf));

      goto cleanup;
    }

  if (period_flag)
    {
      if (period > 0)
        bgl_ciod_period = period;
      else
        CEREBRO_DBG(("invalid period input: %d", period));
    }

  if (failure_max_flag)
    {
      if (failure_max > 0)
        bgl_ciod_failure_max = failure_max;
      else
        CEREBRO_DBG(("invalid failure_max input: %d", failure_max));
    }

  if (connect_timeout_flag)
    {
      if (connect_timeout > 0)
        bgl_ciod_connect_timeout = connect_timeout;
      else
        CEREBRO_DBG(("invalid connect_timeout input: %d", connect_timeout));
    }


  _bgl_socket_paths_setup();

 cleanup:
  conffile_handle_destroy(cf);
  return 0;
}

/*
 * bgl_ciod_metric_cleanup
 *
 * bgl_ciod metric module cleanup function
 */
static int
bgl_ciod_metric_cleanup(void)
{
  /* nothing to do */
  return 0;
}

/*
 * bgl_ciod_metric_get_metric_name
 *
 * bgl_ciod metric module get_metric_name function
 */
static char *
bgl_ciod_metric_get_metric_name(void)
{
  return BGL_CIOD_METRIC_NAME;
}

/*
 * bgl_ciod_metric_get_metric_period
 *
 * bgl_ciod metric module get_metric_period function
 */
static int
bgl_ciod_metric_get_metric_period(int *period)
{
  if (!period)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  *period = bgl_ciod_period;
  return 0;
}

/*
 * bgl_ciod_metric_get_metric_value
 *
 * bgl_ciod metric module get_metric_value function
 */
static int
bgl_ciod_metric_get_metric_value(unsigned int *metric_value_type,
                                 unsigned int *metric_value_len,
                                 void **metric_value)
{
  int fd, errnum;

  if (!metric_value_type || !metric_value_len || !metric_value)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  if ((fd = low_timeout_connect(BGL_CIOD_HOSTNAME,
                                BGL_CIOD_PORT,
                                bgl_ciod_connect_timeout,
                                &errnum)) < 0)
    {
      if (!(errnum == CEREBRO_ERR_CONNECT || errnum == CEREBRO_ERR_CONNECT_TIMEOUT))
        CEREBRO_DBG(("low_timeout_connect: %d", errnum));
      
      bgl_ciod_failures++;
      if (bgl_ciod_failures >= bgl_ciod_failure_max)
        {
          if (bgl_ciod_state && bgl_ciod_was_up)
            CEREBRO_DBG(("BGL ciod daemon via connect detected down, errnum: %d", 
                         errnum));

          /* 
           * This code gives the metric monitor a second chance to
           * detect the the ciod daemon's socket is open.  If atleast
           * one of the ciod daemons still has the socket with port
           * 7000 open, we'll say its up.
           */

          if (!bgl_ciod_sockets_len)
            _bgl_socket_paths_setup();

          if (bgl_ciod_sockets_len)
            {
              int i, ciod_port_alive = 0;

              for (i = 0; i < bgl_ciod_sockets_len; i++)
                {
                  int rv;

                  if ((rv = _contains_inode_socket(bgl_ciod_sockets[i],
                                                   bgl_ciod_inode)) < 0)
                    continue;

                  if (rv)
                    {
                      CEREBRO_DBG(("BGL ciod daemon via proc detected up"));
                      ciod_port_alive = 1;
                      break;
                    }
                }

              if (!ciod_port_alive)
                bgl_ciod_state = 0;
            }
          else
              bgl_ciod_state = 0;
        }
    }
  else
    {
      bgl_ciod_failures = 0;
      if (!bgl_ciod_state && bgl_ciod_was_up)
        CEREBRO_DBG(("BGL ciod daemon back up"));
      bgl_ciod_state = 1;
      bgl_ciod_was_up = 1;
      close(fd);
    }
  
  *metric_value_type = CEREBRO_METRIC_VALUE_TYPE_U_INT32;
  *metric_value_len = sizeof(u_int32_t);
  *metric_value = (void *)&bgl_ciod_state;

  return 0;
}

/*
 * bgl_ciod_metric_destroy_metric_value
 *
 * bgl_ciod metric module destroy_metric_value function
 */
static int
bgl_ciod_metric_destroy_metric_value(void *metric_value)
{
  return 0;
}

/*
 * bgl_ciod_metric_get_metric_thread
 *
 * bgl_ciod metric module get_metric_thread function
 */
static Cerebro_metric_thread_pointer
bgl_ciod_metric_get_metric_thread(void)
{
  return NULL;
}

#if WITH_STATIC_MODULES
struct cerebro_metric_module_info bgl_ciod_metric_module_info =
#else  /* !WITH_STATIC_MODULES */
struct cerebro_metric_module_info metric_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    BGL_CIOD_METRIC_MODULE_NAME,
    &bgl_ciod_metric_setup,
    &bgl_ciod_metric_cleanup,
    &bgl_ciod_metric_get_metric_name,
    &bgl_ciod_metric_get_metric_period,
    &bgl_ciod_metric_get_metric_value,
    &bgl_ciod_metric_destroy_metric_value,
    &bgl_ciod_metric_get_metric_thread,
  };
