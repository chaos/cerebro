/*****************************************************************************\
 *  $Id: wrappers.c,v 1.8 2005-01-18 18:43:35 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <limits.h>
#include <assert.h>
#include <errno.h>

#include "error.h"
#include "fd.h"
#include "wrappers.h"

#define MALLOC_MAGIC      0xf0e0d0c0
#define MALLOC_PAD_DATA   0xab
#define MALLOC_PAD_LEN    16

void *
wrap_malloc(const char *file, int line, size_t size)
{
  void *ptr;

  assert(file != NULL);
  assert(size > 0 && size <= INT_MAX);

  if ((ptr = malloc(2*sizeof(int) + size + MALLOC_PAD_LEN)) == NULL) 
    err_exit("malloc(%s:%d): %s", file, line, strerror(errno));

  *((int *)(ptr)) = MALLOC_MAGIC;
  *((int *)(ptr+sizeof(int))) = size;
  memset(ptr+2*sizeof(int), '\0', size);
  memset(ptr+2*sizeof(int) + size, MALLOC_PAD_DATA, MALLOC_PAD_LEN);
  return (ptr + 2*sizeof(int));
}

void
wrap_free(const char *file, int line, void *ptr)
{
  assert(file != NULL);

  if (ptr != NULL)
    {
      void *p = ptr - 2*sizeof(int);
      int i, size;
      char *c;

      assert(*((int *)p) == MALLOC_MAGIC);
      size = *((int *)(p + sizeof(int)));
      
      c = (char *)(p + 2*sizeof(int) + size);
      for (i = 0; i < MALLOC_PAD_LEN; i++)
        assert(c[i] == (char)MALLOC_PAD_DATA);
      
      free(p);
    }
}

char *
wrap_strdup(const char *file, int line, const char *s)
{
  char *ptr;

  assert(file != NULL);
  assert(s != NULL);

  ptr = wrap_malloc(file, line, strlen(s) + 1);
  strcpy(ptr, s);
  return ptr;
}

char *
wrap_strncpy(const char *file, int line, char *dest, const char *src, size_t n)
{
  char *ret;

  assert(file != NULL);

  ret = strncpy(dest, src, n);
  dest[n-1] = '\0';
  
  return ret;
}

int 
wrap_open(const char *file, int line, const char *pathname, int flags, int mode)
{
  int fd;

  assert(file != NULL);

  if ((fd = open(pathname, flags, mode)) < 0)
    err_exit("open(%s:%d): %s", file, line, strerror(errno));
  return fd;
}

int 
wrap_close(const char *file, int line, int fd)
{
  int ret;
                                 
  assert(file != NULL);
                                                   
  if ((ret = close(fd)) < 0)
    err_exit("close(%s:%d): %s", file, line, strerror(errno));
  return ret;
}

ssize_t
wrap_read(const char *file, int line, int fd, void *buf, size_t count)
{
  ssize_t ret;

  assert(file != NULL);

  if ((ret = fd_read_n(fd, buf, count)) < 0)
    err_exit("read(%s:%d): %s", file, line, strerror(errno));
    
  return ret;
}

ssize_t
wrap_write(const char *file, int line, int fd, const void *buf, size_t count)
{
  ssize_t ret;

  assert(file != NULL);

  if ((ret = fd_write_n(fd, buf, count)) < 0)
    err_exit("write(%s:%d): %s", file, line, strerror(errno));

  return ret;
}

int
wrap_chdir(const char *file, int line, const char *path)
{
  int ret;

  assert(file != NULL);

  if ((ret = chdir(path)) < 0)
    err_exit("chdir(%s:%d): %s", file, line, strerror(errno));

  return ret;
}

mode_t
wrap_umask(const char *file, int line, mode_t mask)
{
  assert(file != NULL);

  /* achu: never supposed to fail.  Go fig. */
  return umask(mask);
}

int
wrap_socket(const char *file, int line, int domain, int type, int protocol)
{
  int fd;

  assert(file != NULL);

  if ((fd = socket(domain, type, protocol)) < 0)
    err_exit("socket(%s:%d): %s", file, line, strerror(errno));

  return fd;
}

int 
wrap_bind(const char *file, int line, int sockfd, struct sockaddr *my_addr, socklen_t addrlen)
{
  int ret;
  
  assert(file != NULL);

  if ((ret = bind(sockfd, my_addr, addrlen)) < 0)
    err_exit("bind(%s:%d): %s", file, line, strerror(errno));

  return ret;
}

int 
wrap_connect(const char *file, int line, int sockfd, struct sockaddr *serv_addr, socklen_t addrlen)
{
  int ret;

  assert(file != NULL);

  if ((ret = connect(sockfd, serv_addr, addrlen)) < 0)
    err_exit("connect(%s:%d): %s", file, line, strerror(errno));

  return ret;
}

int
wrap_getsockopt(const char *file, int line, int s, int level, int optname, void *optval, socklen_t *optlen)
{
  int ret;

  assert(file != NULL);

  if ((ret = getsockopt(s, level, optname, optval, optlen)) < 0)
    err_exit("socket(%s:%d): %s", file, line, strerror(errno));

  return ret;
}

int
wrap_setsockopt(const char *file, int line, int s, int level, int optname, const void *optval, socklen_t optlen)
{
  int ret;

  assert(file != NULL);

  if ((ret = setsockopt(s, level, optname, optval, optlen)) < 0)
    err_exit("socket(%s:%d): %s", file, line, strerror(errno));

  return ret;
}

int 
wrap_inet_pton(const char *file, int line, int af, const char *src, void *dst)
{
  int ret;
  
  assert(file != NULL);

  if ((ret = inet_pton(af, src, dst)) < 0)
    err_exit("inet_pton(%s:%d): %s", file, line, strerror(errno));

  return ret;
}

int
wrap_gettimeofday(const char *file, int line, struct timeval *tv, struct timezone *tz)
{
  int ret;

  assert(file != NULL);

  if ((ret = gettimeofday(tv, tz)) < 0)
    err_exit("gettimeofday(%s:%d): %s", file, line, strerror(errno));

  return ret;
}

time_t 
wrap_time(const char *file, int line, time_t *t)
{
  time_t ret;

  assert(file != NULL);

  if ((ret = time(t)) == ((time_t)-1))
    err_exit("time(%s:%d): %s", file, line, strerror(errno));

  return ret;
}

struct tm *
wrap_localtime(const char *file, int line, const time_t *timep)
{
  struct tm *tmptr;
  
  assert(file != NULL);

  if (!(tmptr = localtime(timep)))
    err_exit("localtime(%s:%d)", file, line);

  return tmptr;
}

struct tm *
wrap_localtime_r(const char *file, int line, const time_t *timep, struct tm *result)
{
  struct tm *tmptr;
  
  assert(file != NULL);

  if (!(tmptr = localtime_r(timep, result)))
    err_exit("localtime(%s:%d)", file, line);

  return tmptr;
}

int 
wrap_gethostname(const char *file, int line, char *name, size_t len)
{
  int ret;

  assert(file != NULL);

  if ((ret = gethostname(name, len)) < 0)
    err_exit("gethostname(%s:%d): %s", file, line, strerror(errno));

  return ret;
}

int 
wrap_pthread_create(const char *file, int line, pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *), void *arg)
{
  int ret;
  
  assert(file != NULL);

  if ((ret = pthread_create(thread, attr, start_routine, arg)) != 0)
    err_exit("pthread_create(%s:%d): %s", file, line, strerror(ret));

  return ret;
}

int 
wrap_pthread_attr_init(const char *file, int line, pthread_attr_t *attr)
{
  int ret;
  
  assert(file != NULL);

  if ((ret = pthread_attr_init(attr)) != 0)
    err_exit("pthread_attr_init(%s:%d): %s", file, line, strerror(ret));

  return ret;
}

int 
wrap_pthread_attr_destroy(const char *file, int line, pthread_attr_t *attr)
{
  int ret;
  
  assert(file != NULL);

  if ((ret = pthread_attr_destroy(attr)) != 0)
    err_exit("pthread_attr_destroy(%s:%d): %s", file, line, strerror(ret));

  return ret;
}

int 
wrap_pthread_attr_setdetachstate(const char *file, int line, pthread_attr_t *attr, int detachstate)
{
  int ret;
  
  assert(file != NULL);

  if ((ret = pthread_attr_setdetachstate(attr, detachstate)) != 0)
    err_exit("pthread_attr_setdetachstate(%s:%d): %s", file, line, strerror(ret));

  return ret;
}

int
wrap_pthread_mutex_lock(const char *file, int line, pthread_mutex_t *mutex)
{
  int ret;
  
  assert(file != NULL);

  if ((ret = pthread_mutex_lock(mutex)) != 0)
    err_exit("pthread_mutex_lock(%s:%d): %s", file, line, strerror(ret));

  return ret;
}

int 
wrap_pthread_mutex_trylock(const char *file, int line, pthread_mutex_t *mutex)
{
  int ret;
  
  assert(file != NULL);

  ret = pthread_mutex_trylock(mutex);
  if (ret != 0 && ret != EBUSY)
    err_exit("pthread_mutex_trylock(%s:%d): %s", file, line, strerror(ret));

  return ret;
}

int 
wrap_pthread_mutex_unlock(const char *file, int line, pthread_mutex_t *mutex)
{
  int ret;
  
  assert(file != NULL);

  if ((ret = pthread_mutex_unlock(mutex)) != 0)
    err_exit("pthread_mutex_unlock(%s:%d): %s", file, line, strerror(ret));

  return ret;
}

pid_t 
wrap_fork(const char *file, int line)
{
  pid_t pid;

  assert(file != NULL);

  if ((pid = fork()) < 0)
    err_exit("fork(%s:%d): %s", file, line, strerror(errno));

  return pid;
}

Sighandler_t
wrap_signal(const char *file, int line, int signum, Sighandler_t handler)
{
  Sighandler_t ret;

  assert(file != NULL);

  if ((ret = signal(signum, handler)) == SIG_ERR)
    err_exit("signal(%s:%d): %s", file, line, strerror(errno));

  return ret;
}
