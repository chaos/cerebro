/*****************************************************************************\
 *  $Id: cerebro_util.c,v 1.5 2005-07-22 17:21:07 achu Exp $
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
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>

#include "cerebro.h"
#include "cerebro_api.h"
#include "cerebro_util.h"

#include "debug.h"

int 
_cerebro_handle_check(cerebro_t handle)
{
  if (!handle || handle->magic != CEREBRO_MAGIC_NUMBER)
    return -1;

  if (!handle->metriclists)
    {
      CEREBRO_DBG(("metriclists null"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (!handle->nodelists)
    {
      CEREBRO_DBG(("nodelists null"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  
  return 0;
}

int
_cerebro_low_timeout_connect(cerebro_t handle,
			     const char *hostname,
			     unsigned int port,
			     unsigned int connect_timeout)
{
  int rv, old_flags, fd = -1;
  struct sockaddr_in servaddr;
  struct hostent *hptr;
 
  if (!hostname || !port || !connect_timeout)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  /* valgrind will report a mem-leak in gethostbyname() */
  if (!(hptr = gethostbyname(hostname)))
    {
      handle->errnum = CEREBRO_ERR_HOSTNAME;
      return -1;
    }
 
  /* Alot of this code is from Unix Network Programming, by Stevens */
 
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      CEREBRO_DBG(("socket: %s", strerror(errno)));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }
 
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  servaddr.sin_addr = *((struct in_addr *)hptr->h_addr);

  if ((old_flags = fcntl(fd, F_GETFL, 0)) < 0)
    {
      CEREBRO_DBG(("fcntl: %s", strerror(errno)));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }
   
  if (fcntl(fd, F_SETFL, old_flags | O_NONBLOCK) < 0)
    {
      CEREBRO_DBG(("fcntl: %s", strerror(errno)));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }
   
  rv = connect(fd,
	       (struct sockaddr *)&servaddr,
	       sizeof(struct sockaddr_in));
  if (rv < 0 && errno != EINPROGRESS)
    {
      handle->errnum = CEREBRO_ERR_CONNECT;
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
	  CEREBRO_DBG(("select: %s", strerror(errno)));
	  handle->errnum = CEREBRO_ERR_INTERNAL;
          goto cleanup;
        }
 
      if (!rv)
        {
          handle->errnum = CEREBRO_ERR_CONNECT_TIMEOUT;
          goto cleanup;
        }
      else
        {
          if (FD_ISSET(fd, &rset) || FD_ISSET(fd, &wset))
            {
              int len, error;
 
              len = sizeof(int);
               
              if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
                {
		  CEREBRO_DBG(("getsockopt: %s", strerror(errno)));
                  handle->errnum = CEREBRO_ERR_INTERNAL;
                  goto cleanup;
                }
	      
              if (error != 0)
                {
                  errno = error;
                  handle->errnum = CEREBRO_ERR_CONNECT;
                  goto cleanup;
                }
              /* else no error, connected within timeout length */
            }
          else
            {
	      CEREBRO_DBG(("select returned bad data"));
              handle->errnum = CEREBRO_ERR_INTERNAL;
              goto cleanup;
            }
        }
    }
   
  /* reset flags */
  if (fcntl(fd, F_SETFL, old_flags) < 0)
    {
      CEREBRO_DBG(("fcntl: %s", strerror(errno)));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }
   
  return fd;
 
 cleanup:
  close(fd);
  return -1;
}

