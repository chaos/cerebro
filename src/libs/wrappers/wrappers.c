/*****************************************************************************\
 *  $Id: wrappers.c,v 1.16 2006-12-21 17:48:44 chu11 Exp $
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

#include <limits.h>
#include <assert.h>
#include <errno.h>

#include "wrappers.h"

extern int h_errno;

#define MALLOC_MAGIC      0xf0e0d0c0
#define MALLOC_PAD_DATA   0xab
#define MALLOC_PAD_LEN    16

void *
wrap_malloc(WRAPPERS_ARGS, size_t size)
{
  void *ptr;

  assert(file && function);

  if (!(size > 0 || size <= INT_MAX))
    WRAPPERS_ERR_INVALID_PARAMETERS("malloc");

  if (!(ptr = malloc(2*sizeof(int) + size + MALLOC_PAD_LEN))) 
    WRAPPERS_ERR_ERRNO("malloc");

  *((int *)(ptr)) = MALLOC_MAGIC;
  *((int *)(ptr+sizeof(int))) = size;
  memset(ptr+2*sizeof(int), '\0', size);
  memset(ptr+2*sizeof(int) + size, MALLOC_PAD_DATA, MALLOC_PAD_LEN);
  return (ptr + 2*sizeof(int));
}

void
wrap_free(WRAPPERS_ARGS, void *ptr)
{
  void *p = ptr - 2*sizeof(int);
  int i, size;
  char *c;

  assert(file && function);

  if (!ptr)
    return;

  if (!(*((int *)p) == MALLOC_MAGIC))
    WRAPPERS_ERR_MSG("free", "memory corruption");

  size = *((int *)(p + sizeof(int)));
  c = (char *)(p + 2*sizeof(int) + size);
  for (i = 0; i < MALLOC_PAD_LEN; i++)
    assert(c[i] == (char)MALLOC_PAD_DATA);
  free(p);
}

void 
_Free(void *ptr)
{
  wrap_free(__FILE__, __FUNCTION__, __LINE__, ptr);
}

char *
wrap_strdup(WRAPPERS_ARGS, const char *s)
{
  char *ptr;

  assert(file && function);

  if (!s)
    WRAPPERS_ERR_INVALID_PARAMETERS("strdup");

  ptr = wrap_malloc(file, function, line, strlen(s) + 1);
  strcpy(ptr, s);
  return ptr;
}

char *
wrap_strncpy(WRAPPERS_ARGS, char *dest, const char *src, size_t n)
{
  char *rv;

  assert(file && function);

  if (!dest || !src)
    WRAPPERS_ERR_INVALID_PARAMETERS("strncpy");

  rv = strncpy(dest, src, n);
  dest[n-1] = '\0';
  return rv;
}

int 
wrap_open(WRAPPERS_ARGS, const char *pathname, int flags, int mode)
{
  int fd;

  assert(file && function);
  
  if (!pathname)
    WRAPPERS_ERR_INVALID_PARAMETERS("open");

  if ((fd = open(pathname, flags, mode)) < 0)
    WRAPPERS_ERR_ERRNO("open");

  return fd;
}

int 
wrap_close(WRAPPERS_ARGS, int fd)
{
  int rv;
                                 
  assert(file && function);
                                                   
  if ((rv = close(fd)) < 0)
    WRAPPERS_ERR_ERRNO("close");

  return rv;
}

ssize_t
wrap_read(WRAPPERS_ARGS, int fd, void *buf, size_t count)
{
  ssize_t rv;
  
  assert(file && function);

  if (!buf || !(count > 0 || count <= INT_MAX))
    WRAPPERS_ERR_INVALID_PARAMETERS("read");

  do {
    rv = read(fd, buf, count);
  } while (rv < 0 && errno == EINTR);

  if (rv < 0)
    WRAPPERS_ERR_ERRNO("read");
  
  return rv;
}

ssize_t
wrap_write(WRAPPERS_ARGS, int fd, const void *buf, size_t count)
{
  ssize_t rv;

  assert(file && function);

  if (!buf || !(count > 0 || count <= INT_MAX))
    WRAPPERS_ERR_INVALID_PARAMETERS("write");

  do {
    rv = write(fd, buf, count);
  } while (rv < 0 && errno == EINTR);

  if (rv < 0)
    WRAPPERS_ERR_ERRNO("write");

  return rv;
}

int
wrap_chdir(WRAPPERS_ARGS, const char *path)
{
  int rv;

  assert(file && function);

  if (!path)
    WRAPPERS_ERR_INVALID_PARAMETERS("chdir");

  if ((rv = chdir(path)) < 0)
    WRAPPERS_ERR_ERRNO("chdir");

  return rv;
}

int 
wrap_stat(WRAPPERS_ARGS, const char *path, struct stat *buf)
{
  int rv;

  assert(file && function);

  if (!path || !buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("stat");

  if ((rv = stat(path, buf)) < 0)
    WRAPPERS_ERR_ERRNO("stat");

  return rv;
}

mode_t
wrap_umask(WRAPPERS_ARGS, mode_t mask)
{
  assert(file && function);

  /* achu: never supposed to fail.  Go fig. */
  return umask(mask);
}

DIR *
wrap_opendir(WRAPPERS_ARGS, const char *name)
{
  DIR *rv;
  
  assert(file && function);

  if (!name)
    WRAPPERS_ERR_INVALID_PARAMETERS("opendir");

  if (!(rv = opendir(name)))
    WRAPPERS_ERR_ERRNO("opendir");

  return rv;
}

int
wrap_closedir(WRAPPERS_ARGS, DIR *dir)
{
  int rv;

  assert(file && function);

  if (!dir)
    WRAPPERS_ERR_INVALID_PARAMETERS("closedir");

  if ((rv = closedir(dir)) < 0)
    WRAPPERS_ERR_ERRNO("closedir");

  return rv;
}

int
wrap_socket(WRAPPERS_ARGS, int domain, int type, int protocol)
{
  int fd;

  assert(file && function);

  if ((fd = socket(domain, type, protocol)) < 0)
    WRAPPERS_ERR_ERRNO("socket");

  return fd;
}

int 
wrap_bind(WRAPPERS_ARGS, int sockfd, struct sockaddr *my_addr, socklen_t addrlen)
{
  int rv;
  
  assert(file && function);

  if (!my_addr || !(addrlen > 0 || addrlen <= INT_MAX))
    WRAPPERS_ERR_INVALID_PARAMETERS("bind");

  if ((rv = bind(sockfd, my_addr, addrlen)) < 0)
    WRAPPERS_ERR_ERRNO("bind");

  return rv;
}

int 
wrap_connect(WRAPPERS_ARGS, int sockfd, struct sockaddr *serv_addr, socklen_t addrlen)
{
  int rv;

  assert(file && function);

  if (!serv_addr || !(addrlen > 0 || addrlen <= INT_MAX))
    WRAPPERS_ERR_INVALID_PARAMETERS("connect");

  if ((rv = connect(sockfd, serv_addr, addrlen)) < 0)
    WRAPPERS_ERR_ERRNO("connect");

  return rv;
}

int 
wrap_listen(WRAPPERS_ARGS, int s, int backlog)
{
  int rv;

  assert(file && function);

  if (!(backlog > 0 || backlog <= INT_MAX))
    WRAPPERS_ERR_INVALID_PARAMETERS("listen");

  if ((rv = listen(s, backlog)) < 0)
    WRAPPERS_ERR_ERRNO("listen");

  return rv;
}

int 
wrap_accept(WRAPPERS_ARGS, int s, struct sockaddr *addr, socklen_t *addrlen)
{
  int rv;

  assert(file && function);

  if (!addr || !addrlen || !(*addrlen > 0 || *addrlen <= INT_MAX))
    WRAPPERS_ERR_INVALID_PARAMETERS("accept");

  if ((rv = accept(s, addr, addrlen)) < 0)
    WRAPPERS_ERR_ERRNO("accept");

  return rv;
}

int 
wrap_select(WRAPPERS_ARGS, int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
  int rv;
  struct timeval timeout_orig;
  struct timeval start, end, delta;
  
  assert(file && function);

  /* readfds, writefds, exceptfds, and timeout could each be null, but
   * not all can be null at the same time
   */
  if (!readfds && !writefds && !exceptfds && !timeout)
    WRAPPERS_ERR_INVALID_PARAMETERS("select");

  if (timeout) 
    {
      timeout_orig = *timeout;
      Gettimeofday(&start, NULL);
    }

  do 
    {
      rv = select(n, readfds, writefds, exceptfds, timeout);
      if (rv < 0 && errno != EINTR)
        WRAPPERS_ERR_ERRNO("select");
      if (rv < 0 && timeout) 
	{
	  Gettimeofday(&end, NULL);
	  /* delta = end-start */
	  timersub(&end, &start, &delta);     
	  /* timeout = timeout_orig-delta */
	  timersub(&timeout_orig, &delta, timeout);     
	}
    } 
  while (rv < 0);

  return rv;
}

int 
wrap_poll(WRAPPERS_ARGS, struct pollfd *ufds, unsigned int nfds, int timeout)
{
  int rv;
  struct timeval timeout_orig, timeout_current;
  struct timeval start, end, delta;
                                                                         
  if (!ufds)  
    WRAPPERS_ERR_INVALID_PARAMETERS("poll");

  /* Poll uses timeout in milliseconds */
  if (timeout >= 0) 
    {
      timeout_orig.tv_sec = (long)timeout/1000;
      timeout_orig.tv_usec = (timeout % 1000) * 1000;
      Gettimeofday(&start, NULL);
    }

  do {
    rv = poll(ufds, nfds, timeout);
    if (rv < 0 && errno != EINTR)
      WRAPPERS_ERR_ERRNO("poll");
    if (rv < 0 && timeout >= 0) {
      Gettimeofday(&end, NULL);
      /* delta = end-start */
      timersub(&end, &start, &delta);     
      /* timeout_current = timeout_orig-delta */
      timersub(&timeout_orig, &delta, &timeout_current);
      timeout = (timeout_current.tv_sec * 1000) + (timeout_current.tv_usec/1000);
    }
  } while (rv < 0);
                                                                         
  return rv;
}

int
wrap_getsockopt(WRAPPERS_ARGS, int s, int level, int optname, void *optval, socklen_t *optlen)
{
  int rv;

  assert(file && function);

  if (!optval || !optlen || !(*optlen > 0 || *optlen <= INT_MAX))
    WRAPPERS_ERR_INVALID_PARAMETERS("getsockopt");

  if ((rv = getsockopt(s, level, optname, optval, optlen)) < 0)
    WRAPPERS_ERR_ERRNO("getsockopt");

  return rv;
}

int
wrap_setsockopt(WRAPPERS_ARGS, int s, int level, int optname, const void *optval, socklen_t optlen)
{
  int rv;

  assert(file && function);

  if (!optval || !(optlen > 0 || optlen <= INT_MAX))
    WRAPPERS_ERR_INVALID_PARAMETERS("setsockopt");

  if ((rv = setsockopt(s, level, optname, optval, optlen)) < 0)
    WRAPPERS_ERR_ERRNO("setsockopt");

  return rv;
}

struct hostent *
wrap_gethostbyname(WRAPPERS_ARGS, const char *name)
{
  struct hostent *rv;

  assert(file && function);

  if (!name)
    WRAPPERS_ERR_INVALID_PARAMETERS("gethostbyname");

  if (!(rv = gethostbyname(name)))
    WRAPPERS_ERR_MSG("gethostbyname", hstrerror(h_errno));

  return rv;
}

const char *
wrap_inet_ntop(WRAPPERS_ARGS, int af, const void *src, char *dst, socklen_t cnt)
{
  const char *rv;

  assert(file && function);

  if (!src || !dst)
    WRAPPERS_ERR_INVALID_PARAMETERS("inet_ntop");

  if ((rv = inet_ntop(af, src, dst, cnt)) < 0)
    WRAPPERS_ERR_ERRNO("inet_ntop");
  
  return rv;
}

int 
wrap_inet_pton(WRAPPERS_ARGS, int af, const char *src, void *dst)
{
  int rv;
  
  assert(file && function);

  if (!src || !dst)
    WRAPPERS_ERR_INVALID_PARAMETERS("inet_pton");

  if ((rv = inet_pton(af, src, dst)) < 0)
    WRAPPERS_ERR_ERRNO("inet_pton");

  return rv;
}

struct tm *
wrap_localtime(WRAPPERS_ARGS, const time_t *timep)
{
  struct tm *tmptr;
  
  assert(file && function);
  
  if (!timep)
    WRAPPERS_ERR_INVALID_PARAMETERS("localtime");
  
  if (!(tmptr = localtime(timep)))
    WRAPPERS_ERR_ERRNO("localtime");
  
  return tmptr;
}

struct tm *
wrap_localtime_r(WRAPPERS_ARGS, const time_t *timep, struct tm *result)
{
  struct tm *tmptr;
  
  assert(file && function);
  
  if (!timep || !result)
    WRAPPERS_ERR_INVALID_PARAMETERS("localtime_r");
  
  if (!(tmptr = localtime_r(timep, result)))
    WRAPPERS_ERR_ERRNO("localtime_r");
  
  return tmptr;
}

int
wrap_gettimeofday(WRAPPERS_ARGS, struct timeval *tv, struct timezone *tz)
{
  int rv;

  assert(file && function);

  if (!tv)
    WRAPPERS_ERR_INVALID_PARAMETERS("gettimeofday");

  if ((rv = gettimeofday(tv, tz)) < 0)
    WRAPPERS_ERR_ERRNO("gettimeofday");

  return rv;
}

pid_t 
wrap_fork(WRAPPERS_ARGS)
{
  pid_t pid;

  assert(file && function);

  if ((pid = fork()) < 0)
    WRAPPERS_ERR_ERRNO("fork");

  return pid;
}

Sighandler_t
wrap_signal(WRAPPERS_ARGS, int signum, Sighandler_t handler)
{
  Sighandler_t rv;

  assert(file && function);

  if (!handler)
    WRAPPERS_ERR_INVALID_PARAMETERS("signal");

  if ((rv = signal(signum, handler)) == SIG_ERR)
    WRAPPERS_ERR_ERRNO("signal");

  return rv;
}

int 
wrap_gethostname(WRAPPERS_ARGS, char *name, size_t len)
{
  int rv;

  assert(file && function);

  if (!name || !(len > 0 || len <= INT_MAX))
    WRAPPERS_ERR_INVALID_PARAMETERS("gethostbyname");

  if ((rv = gethostname(name, len)) < 0)
    WRAPPERS_ERR_ERRNO("gethostname");

  return rv;
}

