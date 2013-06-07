/*****************************************************************************\
 *  $Id: network_util.c,v 1.15 2010-02-02 01:01:21 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2011 Lawrence Livermore National Security, LLC.
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
 *  with Cerebro. If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else  /* !TIME_WITH_SYS_TIME */
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else /* !HAVE_SYS_TIME_H */
#  include <time.h>
# endif /* !HAVE_SYS_TIME_H */
#endif /* !TIME_WITH_SYS_TIME */
#include <sys/types.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */

#include <errno.h>

#include "cerebro.h"

#include "network_util.h"

#include "debug.h"

#define GETHOSTBYNAME_AUX_BUFLEN 1024

int
receive_data(int fd,
             unsigned int bytes_to_read,
             char *buf,
             unsigned int buflen,
             unsigned int timeout_len,
             unsigned int *errnum)
{
  int bytes_read = 0;
  
  if (!buf 
      || !buflen 
      || !bytes_to_read 
      || buflen < bytes_to_read
      || !timeout_len)
    {
      CEREBRO_DBG(("invalid parameters"));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }
  
  memset(buf, '\0', buflen);
  
  while (bytes_read < bytes_to_read)
    {
      fd_set rfds;
      struct timeval tv;
      int num;
      
      tv.tv_sec = timeout_len;
      tv.tv_usec = 0;
      
      FD_ZERO(&rfds);
      FD_SET(fd, &rfds);
      
      if ((num = select(fd + 1, &rfds, NULL, NULL, &tv)) < 0)
        {
          CEREBRO_ERR(("select: %s", strerror(errno)));
          if (errnum)
            *errnum = CEREBRO_ERR_INTERNAL;
          goto cleanup;
        }

      if (!num)
        {
          /* Timed out.  If atleast some bytes were read, return data
           * back to the caller to let them possibly unmarshall the
           * data.  Its possible we are expecting more bytes than the
           * client is sending, perhaps because we are using a
           * different protocol version.  This will allow the server
           * to return a invalid version number back to the user.
           */
          if (!bytes_read)
            {
              if (errnum)
                *errnum = CEREBRO_ERR_PROTOCOL_TIMEOUT;
              goto cleanup;
            }
          else
            goto out;
        }

      if (FD_ISSET(fd, &rfds))
        {
          int n;

          /* Don't use fd_read_n b/c it loops until exactly
           * bytes_to_read is read.  Due to version incompatability or
           * error packets, we may want to read a smaller packet.
           */
          if ((n = read(fd, buf + bytes_read, bytes_to_read - bytes_read)) < 0)
            {
              CEREBRO_ERR(("read: %s", strerror(errno)));
              if (errnum)
                *errnum = CEREBRO_ERR_INTERNAL;
              goto cleanup;
            }
          
          /* Pipe closed */
          if (!n)
            {
              if (bytes_read)
                goto out;
              
              if (errnum)
                *errnum = CEREBRO_ERR_PROTOCOL;
              goto cleanup;
            }
          
          bytes_read += n;
        }
      else
        {
          CEREBRO_DBG(("num != 0 but fd not set"));
          if (errnum)
            *errnum = CEREBRO_ERR_INTERNAL;
          goto cleanup;
        }
    }
  
 out:
  return bytes_read;
  
 cleanup:
  return -1;
}

int 
low_timeout_connect(const char *hostname,
                    unsigned int port,
                    unsigned int connect_timeout,
                    unsigned int *errnum)
{
  int rv, old_flags, fd = -1;
  struct sockaddr_in addr;
#ifdef HAVE_FUNC_GETHOSTBYNAME_R_6
  struct hostent hent;
  int h_errnop;
  char buf[GETHOSTBYNAME_AUX_BUFLEN];
#endif /* HAVE_FUNC_GETHOSTBYNAME_R_6 */
  struct hostent *hptr;
  
  if (!hostname || !port || !connect_timeout)
    {
      CEREBRO_DBG(("invalid parameters"));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  
#ifdef HAVE_FUNC_GETHOSTBYNAME_R_6
  memset(&hent, '\0', sizeof(struct hostent));
  if (gethostbyname_r(hostname, 
                      &hent, 
                      buf, 
                      GETHOSTBYNAME_AUX_BUFLEN, 
                      &hptr, 
                      &h_errnop) != 0)
    {
      CEREBRO_ERR(("gethostbyname_r: %s", hstrerror(h_errnop)));
      if (errnum)
        *errnum = CEREBRO_ERR_HOSTNAME;
      return -1;
    }
  if (!hptr)
    {
      CEREBRO_ERR(("gethostbyname_r hptr: %s", hstrerror(h_errnop)));
      if (errnum)
        *errnum = CEREBRO_ERR_HOSTNAME;
      return -1;
    }
#else  /* !HAVE_FUNC_GETHOSTBYNAME_R */
  /* valgrind will report a mem-leak in gethostbyname() */
  if (!(hptr = gethostbyname(hostname)))
    {
      CEREBRO_ERR(("gethostbyname: %s", hstrerror(h_errno)));
      if (errnum)
        *errnum = CEREBRO_ERR_HOSTNAME;
      return -1;
    }
#endif /* !HAVE_FUNC_GETHOSTBYNAME_R */
  
  /* Alot of this code is from Unix Network Programming, by Stevens */
  
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      CEREBRO_ERR(("socket: %s", strerror(errno)));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr = *((struct in_addr *)hptr->h_addr);
  
  if ((old_flags = fcntl(fd, F_GETFL, 0)) < 0)
    {
      CEREBRO_ERR(("fcntl: %s", strerror(errno)));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }
  
  if (fcntl(fd, F_SETFL, old_flags | O_NONBLOCK) < 0)
    {
      CEREBRO_ERR(("fcntl: %s", strerror(errno)));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  rv = connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
  if (rv < 0 && errno != EINPROGRESS)
    {
      if (errnum)
        *errnum = CEREBRO_ERR_CONNECT;
      goto cleanup;
    }
  else if (rv < 0 && errno == EINPROGRESS)
    {
      fd_set rset, wset;
      struct timeval tval;
      
      FD_ZERO(&rset);
      FD_SET(fd, &rset);
      FD_ZERO(&wset);
      FD_SET(fd, &wset);
      tval.tv_sec = connect_timeout;
      tval.tv_usec = 0;
      
      if ((rv = select(fd+1, &rset, &wset, NULL, &tval)) < 0)
        {
          CEREBRO_ERR(("select: %s", strerror(errno)));
          if (errnum)
            *errnum = CEREBRO_ERR_INTERNAL;
          goto cleanup;
        }
      
      if (!rv)
        {
          if (errnum)
            *errnum = CEREBRO_ERR_CONNECT_TIMEOUT;
          goto cleanup;
        }
      else
        {
          if (FD_ISSET(fd, &rset) || FD_ISSET(fd, &wset))
            {
              unsigned int len;
              int error;
              
              len = sizeof(int);
              
              if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
                {
                  CEREBRO_ERR(("getsockopt: %s", strerror(errno)));
                  if (errnum)
                    *errnum = CEREBRO_ERR_INTERNAL;
                  goto cleanup;
                }
              
              if (error != 0)
                {
                  errno = error;
                  if (error == ECONNREFUSED)
                    *errnum = CEREBRO_ERR_CONNECT;
                  else if (error == ETIMEDOUT)
                    *errnum = CEREBRO_ERR_CONNECT_TIMEOUT;
                  else
                    *errnum = CEREBRO_ERR_INTERNAL;
                  goto cleanup;
                }
              /* else no error, connected within timeout length */
            }
          else
            {
              CEREBRO_DBG(("select returned bad data"));
              if (errnum)
                *errnum = CEREBRO_ERR_INTERNAL;
              goto cleanup;
            }
        }
    }
  
  /* reset flags */
  if (fcntl(fd, F_SETFL, old_flags) < 0)
    {
      CEREBRO_ERR(("fcntl: %s", strerror(errno)));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  return fd;

 cleanup:
  /* ignore potential error, just return error */
  close(fd);
  return -1;
}
