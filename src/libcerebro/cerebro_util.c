/*****************************************************************************\
 *  $Id: cerebro_util.c,v 1.13 2005-05-05 16:26:56 achu Exp $
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
#include "cerebro/cerebro_error.h"

int 
cerebro_handle_check(cerebro_t handle)
{
  if (!handle || handle->magic != CEREBRO_MAGIC_NUMBER)
    return -1;

  return 0;
}

int
cerebro_low_timeout_connect(cerebro_t handle,
                            const char *hostname,
                            unsigned int port,
                            unsigned int connect_timeout)
{
  int rv, old_flags, fd = -1;
  struct sockaddr_in servaddr;
  struct hostent *hptr;
 
  if (!hostname)
    {
      cerebro_err_debug_lib("%s(%s:%d): hostname null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!port)
    {
      cerebro_err_debug_lib("%s(%s:%d): port invalid",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!connect_timeout)
    {
      cerebro_err_debug_lib("%s(%s:%d): connect_timeout invalid",
			    __FILE__, __FUNCTION__, __LINE__);
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
      cerebro_err_debug_lib("%s(%s:%d): socket: %s",
			    __FILE__, __FUNCTION__, __LINE__, 
			    strerror(errno));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }
 
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  servaddr.sin_addr = *((struct in_addr *)hptr->h_addr);

  if ((old_flags = fcntl(fd, F_GETFL, 0)) < 0)
    {
      cerebro_err_debug_lib("%s(%s:%d): fcntl: %s",
			    __FILE__, __FUNCTION__, __LINE__, 
			    strerror(errno));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }
   
  if (fcntl(fd, F_SETFL, old_flags | O_NONBLOCK) < 0)
    {
      cerebro_err_debug_lib("%s(%s:%d): fcntl: %s",
			    __FILE__, __FUNCTION__, __LINE__, 
			    strerror(errno));
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
	  cerebro_err_debug_lib("%s(%s:%d): select: %s",
				__FILE__, __FUNCTION__, __LINE__, 
				strerror(errno));
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
		  cerebro_err_debug_lib("%s(%s:%d): getsockopt: %s",
					__FILE__, __FUNCTION__, __LINE__, 
					strerror(errno));
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
	      cerebro_err_debug_lib("%s(%s:%d): select returned bad data",
				    __FILE__, __FUNCTION__, __LINE__);
              handle->errnum = CEREBRO_ERR_INTERNAL;
              goto cleanup;
            }
        }
    }
   
  /* reset flags */
  if (fcntl(fd, F_SETFL, old_flags) < 0)
    {
      cerebro_err_debug_lib("%s(%s:%d): fcntl: %s",
			    __FILE__, __FUNCTION__, __LINE__,
			    strerror(errno));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }
   
  return fd;
 
 cleanup:
  close(fd);
  return -1;
}
