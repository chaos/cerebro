/*****************************************************************************\
 *  $Id: wrappers.c,v 1.28 2005-03-30 00:36:34 achu Exp $
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

extern int h_errno;

#define MALLOC_MAGIC      0xf0e0d0c0
#define MALLOC_PAD_DATA   0xab
#define MALLOC_PAD_LEN    16

void *
wrap_malloc(const char *file, const char *function, int line, size_t size)
{
  void *ptr;

  assert(file != NULL);
  assert(function != NULL);

  if (!(size > 0 || size <= INT_MAX))
    err_exit("malloc(%s(%s:%d)): invalid size: %d", file, function, line, size);

  if ((ptr = malloc(2*sizeof(int) + size + MALLOC_PAD_LEN)) == NULL) 
    err_exit("malloc(%s(%s:%d)): %s", file, function, line, strerror(errno));

  *((int *)(ptr)) = MALLOC_MAGIC;
  *((int *)(ptr+sizeof(int))) = size;
  memset(ptr+2*sizeof(int), '\0', size);
  memset(ptr+2*sizeof(int) + size, MALLOC_PAD_DATA, MALLOC_PAD_LEN);
  return (ptr + 2*sizeof(int));
}

void
wrap_free(const char *file, const char *function, int line, void *ptr)
{
  assert(file != NULL);
  assert(function != NULL);

  if (ptr != NULL)
    {
      void *p = ptr - 2*sizeof(int);
      int i, size;
      char *c;

      /* assert(*((int *)p) == MALLOC_MAGIC); */
      if (!(*((int *)p) == MALLOC_MAGIC))
        err_exit("free(%s(%s:%d)): memory corruption", file, function, line);

      size = *((int *)(p + sizeof(int)));
      
      c = (char *)(p + 2*sizeof(int) + size);
      for (i = 0; i < MALLOC_PAD_LEN; i++)
        assert(c[i] == (char)MALLOC_PAD_DATA);
      
      free(p);
    }
}

void 
_Free(void *ptr)
{
  wrap_free(__FILE__, __FUNCTION__, __LINE__, ptr);
}

char *
wrap_strdup(const char *file, const char *function, int line, const char *s)
{
  char *ptr;

  assert(file != NULL);
  assert(function != NULL);

  if (!s)
    err_exit("strdup(%s(%s:%d)): null s pointer", file, function, line);

  ptr = wrap_malloc(file, function, line, strlen(s) + 1);
  strcpy(ptr, s);
  return ptr;
}

char *
wrap_strncpy(const char *file, const char *function, int line, char *dest, const char *src, size_t n)
{
  char *ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!dest)
    err_exit("strncpy(%s(%s:%d)): null dest pointer", file, function, line);

  if (!src)
    err_exit("strncpy(%s(%s:%d)): null src pointer", file, function, line);

  ret = strncpy(dest, src, n);
  dest[n-1] = '\0';
  
  return ret;
}

int 
wrap_open(const char *file, const char *function, int line, const char *pathname, int flags, int mode)
{
  int fd;

  assert(file != NULL);
  assert(function != NULL);
  
  if (!pathname)
    err_exit("open(%s(%s:%d)): null pathname pointer");

  if ((fd = open(pathname, flags, mode)) < 0)
    err_exit("open(%s(%s:%d)): pathname=%s flags=%x mode=%o: %s", 
             file, function, line, pathname, flags, mode, strerror(errno));

  return fd;
}

int 
wrap_close(const char *file, const char *function, int line, int fd)
{
  int ret;
                                 
  assert(file != NULL);
  assert(function != NULL);
                                                   
  if ((ret = close(fd)) < 0)
    err_exit("close(%s(%s:%d)): %s", file, function, line, strerror(errno));
  return ret;
}

ssize_t
wrap_read(const char *file, const char *function, int line, int fd, void *buf, size_t count)
{
  ssize_t ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!buf)
    err_exit("read(%s(%s:%d)): null buf pointer", file, function, line);

  if (!(count > 0 || count <= INT_MAX))
    err_exit("read(%s(%s:%d)): invalid count: %d", file, function, line, count);

  if ((ret = fd_read_n(fd, buf, count)) < 0)
    err_exit("read(%s(%s:%d)): count=%d: %s", 
             file, function, line, count, strerror(errno));
  
  return ret;
}

ssize_t
wrap_write(const char *file, const char *function, int line, int fd, const void *buf, size_t count)
{
  ssize_t ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!buf)
    err_exit("write(%s(%s:%d)): null buf pointer", file, function, line);

  if (!(count > 0 || count <= INT_MAX))
    err_exit("write(%s(%s:%d)): invalid count: %d", file, function, line, count);

  if ((ret = fd_write_n(fd, buf, count)) < 0)
    err_exit("write(%s(%s:%d)): count=%d: %s", 
             file, function, line, count, strerror(errno));

  return ret;
}

int
wrap_chdir(const char *file, const char *function, int line, const char *path)
{
  int ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!path)
    err_exit("chdir(%s(%s:%d)): null path pointer", file, function, line);

  if ((ret = chdir(path)) < 0)
    err_exit("chdir(%s(%s:%d)): path=%s: %s", file, function, line, path, strerror(errno));

  return ret;
}

int 
wrap_stat(const char *file, const char *function, int line, const char *path, struct stat *buf)
{
  int ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!path)
    err_exit("stat(%s(%s:%d)): null path pointer", file, function, line);

  if (!buf)
    err_exit("stat(%s(%s:%d)): null buf pointer", file, function, line);

  if ((ret = stat(path, buf)) < 0)
    err_exit("stat(%s(%s:%d)): path=%s: %s", file, function, line, path, strerror(errno));

  return ret;
}

mode_t
wrap_umask(const char *file, const char *function, int line, mode_t mask)
{
  assert(file != NULL);
  assert(function != NULL);

  /* achu: never supposed to fail.  Go fig. */
  return umask(mask);
}

DIR *
wrap_opendir(const char *file, const char *function, int line, const char *name)
{
  DIR *ret;
  
  assert(file != NULL);
  assert(function != NULL);

  if (!name)
    err_exit("opendir(%s(%s:%d)): null name pointer", file, function, line);

  if (!(ret = opendir(name)))
    err_exit("opendir(%s(%s:%d)): name=%s: %s", file, function, line, name, strerror(errno));

  return ret;
}

int
wrap_closedir(const char *file, const char *function, int line, DIR *dir)
{
  int ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!dir)
    err_exit("closedir(%s(%s:%d)): null dir pointer", file, function, line);

  if ((ret = closedir(dir)) < 0)
    err_exit("closedir(%s(%s:%d)): %s", file, function, line, strerror(errno));

  return ret;
}

int
wrap_socket(const char *file, const char *function, int line, int domain, int type, int protocol)
{
  int fd;

  assert(file != NULL);
  assert(function != NULL);

  if ((fd = socket(domain, type, protocol)) < 0)
    err_exit("socket(%s(%s:%d)): domain=%x type=%x protocol=%x: %s", 
             file, function, line, domain, type, protocol, strerror(errno));

  return fd;
}

int 
wrap_bind(const char *file, const char *function, int line, int sockfd, struct sockaddr *my_addr, socklen_t addrlen)
{
  int ret;
  
  assert(file != NULL);
  assert(function != NULL);

  if (!my_addr)
    err_exit("bind(%s(%s:%d)): null my_addr pointer", file, function, line);

  if (!(addrlen > 0 || addrlen <= INT_MAX))
    err_exit("bind(%s(%s:%d)): invalid addrlen: %d", file, function, line, addrlen);

  if ((ret = bind(sockfd, my_addr, addrlen)) < 0)
    err_exit("bind(%s(%s:%d)): addrlen=%d: %s", 
             file, function, line, addrlen, strerror(errno));

  return ret;
}

int 
wrap_connect(const char *file, const char *function, int line, int sockfd, struct sockaddr *serv_addr, socklen_t addrlen)
{
  int ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!serv_addr)
    err_exit("connect(%s(%s:%d)): null serv_addr pointer", file, function, line);

  if (!(addrlen > 0 || addrlen <= INT_MAX))
    err_exit("connect(%s(%s:%d)): invalid addrlen: %d", file, function, line, addrlen);

  if ((ret = connect(sockfd, serv_addr, addrlen)) < 0)
    err_exit("connect(%s(%s:%d)): addrlen=%d: %s", 
             file, function, line, addrlen, strerror(errno));

  return ret;
}

int 
wrap_listen(const char *file, const char *function, int line, int s, int backlog)
{
  int ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!(backlog > 0 || backlog <= INT_MAX))
    err_exit("listen(%s(%s:%d)): invalid backlog: %d", file, function, line, backlog);

  if ((ret = listen(s, backlog)) < 0)
    err_exit("listen(%s(%s:%d)): backlog=%d: %s", 
             file, function, line, backlog, strerror(errno));

  return ret;
}

int 
wrap_accept(const char *file, const char *function, int line, int s, struct sockaddr *addr, socklen_t *addrlen)
{
  int ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!addr)
    err_exit("accept(%s(%s:%d)): null addr pointer", file, function, line);

  if (!addrlen)
    err_exit("accept(%s(%s:%d)): null addrlen pointer", file, function, line);

  if (!(*addrlen > 0 || *addrlen <= INT_MAX))
    err_exit("accept(%s(%s:%d)): invalid addrlen: %d", file, function, line, *addrlen);

  if ((ret = accept(s, addr, addrlen)) < 0)
    err_exit("accept(%s(%s:%d)): %s", file, function, line, strerror(errno));

  return ret;
}

int 
wrap_select(const char *file, const char *function, int line, int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
  int ret;
  struct timeval timeout_orig, timeout_current;
  struct timeval start, end, delta;
  
  assert(file != NULL);
  assert(function != NULL);

  /* readfds, writefds, exceptfds, and timeout could each be null, but
   * not all can be null at the same time
   */
  if (!readfds && !writefds && !exceptfds && !timeout)
    err_exit("select(%s(%s:%d)): all null pointers", file, function, line);

  if (timeout) 
    {
      timeout_orig = *timeout;
      timeout_current = *timeout;
      Gettimeofday(&start, NULL);
    }

  do 
    {
      ret = select(n, readfds, writefds, exceptfds, &timeout_current);
      if (ret < 0 && errno != EINTR)
	err_exit("select(%s(%s:%d)): %s", strerror(errno));
      if (ret < 0 && timeout != NULL) 
	{
	  Gettimeofday(&end, NULL);
	  /* delta = end-start */
	  timersub(&end, &start, &delta);     
	  /* timeout_current = timeout_orig-delta */
	  timersub(&timeout_orig, &delta, &timeout_current);     
	}
    } 
  while (ret < 0);

  return ret;
}

int 
wrap_poll(const char *file, const char *function, int line, struct pollfd *ufds, unsigned int nfds, int timeout)
{
  int ret;
  struct timeval timeout_orig, timeout_current;
  struct timeval start, end, delta;
                                                                         
  if (!ufds)  
    err_exit("poll(%s(%s:%d)): null ufds pointer", file, function, line);

  /* timeout can be <= 0 */

  /* Poll uses timeout in milliseconds */
  if (timeout >= 0) 
    {
      timeout_orig.tv_sec = (long)timeout/1000;
      timeout_orig.tv_usec = (timeout % 1000) * 1000;
      Gettimeofday(&start, NULL);
    }

  do {
    ret = poll(ufds, nfds, timeout);
    if (ret < 0 && errno != EINTR)
      err_exit("poll(%s(%s:%d)): %s", strerror(errno));
    if (ret < 0 && timeout >= 0) {
      Gettimeofday(&end, NULL);
      /* delta = end-start */
      timersub(&end, &start, &delta);     
      /* timeout_current = timeout_orig-delta */
      timersub(&timeout_orig, &delta, &timeout_current);
      timeout = (timeout_current.tv_sec * 1000) + (timeout_current.tv_usec/1000);
    }
  } while (ret < 0);
                                                                         
  return ret;
}

int
wrap_getsockopt(const char *file, const char *function, int line, int s, int level, int optname, void *optval, socklen_t *optlen)
{
  int ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!optval)
    err_exit("getsockopt(%s(%s:%d)): null optval pointer", file, function, line);

  if (!optlen)  
    err_exit("getsockopt(%s(%s:%d)): null optlen pointer", file, function, line);

  if (!(*optlen > 0 || *optlen <= INT_MAX))
    err_exit("getsockopt(%s(%s:%d)): invalid *optlen: %d", file, function, line, *optlen);

  if ((ret = getsockopt(s, level, optname, optval, optlen)) < 0)
    err_exit("getsockopt(%s(%s:%d)): optname=%x: %s", 
             file, function, line, optname, strerror(errno));

  return ret;
}

int
wrap_setsockopt(const char *file, const char *function, int line, int s, int level, int optname, const void *optval, socklen_t optlen)
{
  int ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!optval)
    err_exit("setsockopt(%s(%s:%d)): null optval pointer", file, function, line);

  if (!(optlen > 0 || optlen <= INT_MAX))
    err_exit("setsockopt(%s(%s:%d)): invalid optlen: %d", file, function, line, optlen);

  if ((ret = setsockopt(s, level, optname, optval, optlen)) < 0)
    err_exit("setsockopt(%s(%s:%d)): %s", file, function, line, strerror(errno));

  return ret;
}

struct hostent *
wrap_gethostbyname(const char *file, const char *function, int line, const char *name)
{
  struct hostent *ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!name)
    err_exit("gethostbyname(%s(%s:%d)): null name pointer", file, function, line);

  if (!(ret = gethostbyname(name)))
    err_exit("gethostbyname(%s(%s:%d)): name=%s: %s", file, function, line, name, hstrerror(h_errno));

  return ret;
}

const char *
wrap_inet_ntop(const char *file, const char *function, int line, int af, const void *src, char *dst, socklen_t cnt)
{
  const char *ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!src)
    err_exit("inet_ntop(%s(%s:%d)): null src pointer", file, function, line);

  if (!dst)
    err_exit("inet_ntop(%s(%s:%d)): null dst pointer", file, function, line);

  if ((ret = inet_ntop(af, src, dst, cnt)) < 0)
    err_exit("inet_ntop(%s(%s:%d)): af=%x: %s", file, function, line, af, strerror(errno));
  
  return ret;
}

int 
wrap_inet_pton(const char *file, const char *function, int line, int af, const char *src, void *dst)
{
  int ret;
  
  assert(file != NULL);
  assert(function != NULL);

  if (!src)
    err_exit("inet_pton(%s(%s:%d)): null src pointer", file, function, line);

  if (!dst)
    err_exit("inet_pton(%s(%s:%d)): null dst pointer", file, function, line);

  if ((ret = inet_pton(af, src, dst)) < 0)
    err_exit("inet_pton(%s(%s:%d)): af=%x: %s", file, function, line, af, strerror(errno));

  return ret;
}

int
wrap_gettimeofday(const char *file, const char *function, int line, struct timeval *tv, struct timezone *tz)
{
  int ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!tv)
    err_exit("gettimeofday(%s(%s:%d)): null tv pointer", file, function, line);

  /* tz can be null */

  if ((ret = gettimeofday(tv, tz)) < 0)
    err_exit("gettimeofday(%s(%s:%d)): %s", file, function, line, strerror(errno));

  return ret;
}

time_t 
wrap_time(const char *file, const char *function, int line, time_t *t)
{
  time_t ret;

  assert(file != NULL);
  assert(function != NULL);

  /* t can be null */

  if ((ret = time(t)) == ((time_t)-1))
    err_exit("time(%s(%s:%d)): %s", file, function, line, strerror(errno));

  return ret;
}

struct tm *
wrap_localtime(const char *file, const char *function, int line, const time_t *timep)
{
  struct tm *tmptr;
  
  assert(file != NULL);
  assert(function != NULL);

  if (!timep)
    err_exit("localtime(%s(%s:%d)): null timep pointer", file, function, line);

  if (!(tmptr = localtime(timep)))
    err_exit("localtime(%s(%s:%d))", file, function, line);

  return tmptr;
}

struct tm *
wrap_localtime_r(const char *file, const char *function, int line, const time_t *timep, struct tm *result)
{
  struct tm *tmptr;
  
  assert(file != NULL);
  assert(function != NULL);

  if (!timep)
    err_exit("localtime_r(%s(%s:%d)): null timep pointer", file, function, line);

  if (!result)
    err_exit("localtime_r(%s(%s:%d)): null result pointer", file, function, line);

  if (!(tmptr = localtime_r(timep, result)))
    err_exit("localtime(%s(%s:%d))", file, function, line);

  return tmptr;
}

int 
wrap_pthread_create(const char *file, const char *function, int line, pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *), void *arg)
{
  int ret;
  
  assert(file != NULL);
  assert(function != NULL);

  if (!thread)
    err_exit("pthread_create(%s(%s:%d)): null thread pointer", file, function, line);

  /* attr can be null */

  if (!start_routine)
    err_exit("pthread_create(%s(%s:%d)): null start_routine pointer", file, function, line);

  if ((ret = pthread_create(thread, attr, start_routine, arg)) != 0)
    err_exit("pthread_create(%s(%s:%d)): %s", file, function, line, strerror(ret));

  return ret;
}

int 
wrap_pthread_attr_init(const char *file, const char *function, int line, pthread_attr_t *attr)
{
  int ret;
  
  assert(file != NULL);
  assert(function != NULL);

  if (!attr)
    err_exit("pthread_attr_init(%s(%s:%d)): null attr pointer", file, function, line);

  if ((ret = pthread_attr_init(attr)) != 0)
    err_exit("pthread_attr_init(%s(%s:%d)): %s", file, function, line, strerror(ret));

  return ret;
}

int 
wrap_pthread_attr_destroy(const char *file, const char *function, int line, pthread_attr_t *attr)
{
  int ret;
  
  assert(file != NULL);
  assert(function != NULL);

  if (!attr)
    err_exit("pthread_attr_destroy(%s(%s:%d)): null attr pointer", file, function, line);

  if ((ret = pthread_attr_destroy(attr)) != 0)
    err_exit("pthread_attr_destroy(%s(%s:%d)): %s", file, function, line, strerror(ret));

  return ret;
}

int 
wrap_pthread_attr_setdetachstate(const char *file, const char *function, int line, pthread_attr_t *attr, int detachstate)
{
  int ret;
  
  assert(file != NULL);
  assert(function != NULL);

  if (!attr)
    err_exit("pthread_attr_setdetachstate(%s(%s:%d)): null attr pointer", file, function, line);

  if ((ret = pthread_attr_setdetachstate(attr, detachstate)) != 0)
    err_exit("pthread_attr_setdetachstate(%s(%s:%d)): detachstate=%d: %s", 
             file, function, line, detachstate, strerror(ret));

  return ret;
}

int
wrap_pthread_mutex_lock(const char *file, const char *function, int line, pthread_mutex_t *mutex)
{
  int ret;
  
  assert(file != NULL);
  assert(function != NULL);

  if (!mutex)
    err_exit("pthread_mutex_lock(%s(%s:%d)): null mutex pointer", file, function, line);

  if ((ret = pthread_mutex_lock(mutex)) != 0)
    err_exit("pthread_mutex_lock(%s(%s:%d)): %s", file, function, line, strerror(ret));

  return ret;
}

int 
wrap_pthread_mutex_trylock(const char *file, const char *function, int line, pthread_mutex_t *mutex)
{
  int ret;
  
  assert(file != NULL);
  assert(function != NULL);

  if (!mutex)
    err_exit("pthread_mutex_trylock(%s(%s:%d)): null mutex pointer", file, function, line);

  ret = pthread_mutex_trylock(mutex);
  if (ret != 0 && ret != EBUSY)
    err_exit("pthread_mutex_trylock(%s(%s:%d)): %s", file, function, line, strerror(ret));

  return ret;
}

int 
wrap_pthread_mutex_unlock(const char *file, const char *function, int line, pthread_mutex_t *mutex)
{
  int ret;
  
  assert(file != NULL);
  assert(function != NULL);

  if (!mutex)
    err_exit("pthread_mutex_unlock(%s(%s:%d)): null mutex pointer", file, function, line);

  if ((ret = pthread_mutex_unlock(mutex)) != 0)
    err_exit("pthread_mutex_unlock(%s(%s:%d)): %s", file, function, line, strerror(ret));

  return ret;
}

int
wrap_pthread_mutex_init(const char *file, const char *function, int line, pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr)
{
  int ret;
  
  assert(file != NULL);
  assert(function != NULL);

  if (!mutex)
    err_exit("pthread_mutex_init(%s(%s:%d)): null mutex pointer", file, function, line);

  /* mutexattr can be null */

  if ((ret = pthread_mutex_init(mutex, mutexattr)) != 0)
    err_exit("pthread_mutex_init(%s(%s:%d)): %s", file, function, line, strerror(ret));

  return ret;
}

int 
wrap_pthread_cond_signal(const char *file, const char *function, int line, pthread_cond_t *cond)
{
  int ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!cond)
    err_exit("pthread_cond_signal(%s(%s:%d)): null cond pointer", file, function, line);

  if ((ret = pthread_cond_signal(cond)) != 0)
    err_exit("pthread_cond_signal(%s(%s:%d)): %s", file, function, line, strerror(ret));

  return ret;
}

int 
wrap_pthread_cond_wait(const char *file, const char *function, int line, pthread_cond_t *cond, pthread_mutex_t *mutex)
{
  int ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!cond)
    err_exit("pthread_cond_wait(%s(%s:%d)): null cond pointer", file, function, line);

  if (!mutex)
    err_exit("pthread_cond_wait(%s(%s:%d)): null mutex pointer", file, function, line);

  if ((ret = pthread_cond_wait(cond, mutex)) != 0)
    err_exit("pthread_cond_signal(%s(%s:%d)): %s", file, function, line, strerror(ret));

  return ret;
}

pid_t 
wrap_fork(const char *file, const char *function, int line)
{
  pid_t pid;

  assert(file != NULL);
  assert(function != NULL);

  if ((pid = fork()) < 0)
    err_exit("fork(%s(%s:%d)): %s", file, function, line, strerror(errno));

  return pid;
}

Sighandler_t
wrap_signal(const char *file, const char *function, int line, int signum, Sighandler_t handler)
{
  Sighandler_t ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!handler)
    err_exit("signal(%s(%s:%d)): null handler pointer", file, function, line);

  if ((ret = signal(signum, handler)) == SIG_ERR)
    err_exit("signal(%s(%s:%d)): %s", file, function, line, strerror(errno));

  return ret;
}

int 
wrap_gethostname(const char *file, const char *function, int line, char *name, size_t len)
{
  int ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!name)
    err_exit("gethostname(%s(%s:%d)): null name pointer", file, function, line);

  if (!(len > 0 || len <= INT_MAX))
    err_exit("gethostname(%s(%s:%d)): invalid len: %d", file, function, line, len);

  if ((ret = gethostname(name, len)) < 0)
    err_exit("gethostname(%s(%s:%d)): len=%d: %s", 
             file, function, line, len, strerror(errno));

  return ret;
}

#if !WITH_STATIC_MODULES
int
wrap_lt_dlinit(const char *file, const char *function, int line)
{
  int ret;

  assert(file != NULL);
  assert(function != NULL);

  if ((ret = lt_dlinit()) != 0)
    err_exit("lt_dlinit(%s(%s:%d)): %s", file, function, line, lt_dlerror());

  return ret;
}

int
wrap_lt_dlexit(const char *file, const char *function, int line)
{
  int ret;

  assert(file != NULL);
  assert(function != NULL);

  if ((ret = lt_dlexit()) != 0)
    err_exit("lt_dlexit(%s(%s:%d)): %s", file, function, line, lt_dlerror());

  return ret;
}

lt_dlhandle
wrap_lt_dlopen(const char *file, const char *function, int line, const char *filename)
{
  lt_dlhandle ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!filename)
    err_exit("lt_dlopen(%s(%s:%d)): null filename pointer");

  if (!(ret = lt_dlopen(filename)))
    err_exit("lt_dlopen(%s(%s:%d)): filename=%s: %s", file, function, line, filename, lt_dlerror());

  return ret;
}

lt_ptr
wrap_lt_dlsym(const char *file, const char *function, int line, void *handle, char *symbol)
{
  lt_ptr *ret;
  const char *err;

  assert(file != NULL);
  assert(function != NULL);

  if (!handle)
    err_exit("lt_dlsym(%s(%s:%d)): null handle pointer");

  if (!symbol)
    err_exit("lt_dlsym(%s(%s:%d)): null symbol pointer");

  /* "clear" lt_dlerror() */
  lt_dlerror();

  if (!(ret = lt_dlsym(handle, symbol)))
    {
      err = lt_dlerror();
      if (err != NULL)
        err_exit("lt_dlsym(%s(%s:%d)): symbol=%s: %s", file, function, line, symbol, err);
    }

  return ret;
}

int 
wrap_lt_dlclose(const char *file, const char *function, int line, void *handle)
{
  int ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!handle)
    err_exit("lt_dlclose(%s(%s:%d)): null handle pointer");

  if ((ret = lt_dlclose(handle)) != 0)
    err_exit("lt_dlclose(%s(%s:%d)): %s", lt_dlerror());

  return ret;
}
#endif /* !WITH_STATIC_MODULES */

List 
wrap_list_create(const char *file, const char *function, int line, ListDelF f)
{
  List ret;

  assert(file != NULL);
  assert(function != NULL);

  /* f can be null */

  if (!(ret = list_create(f)))
    err_exit("list_create(%s(%s:%d)): %s", file, function, line, strerror(errno));

  return ret;
}

void
wrap_list_destroy(const char *file, const char *function, int line, List l)
{
  assert(file != NULL);
  assert(function != NULL);

  if (!l)
    err_exit("list_destroy(%s(%s:%d)): null l pointer", file, function, line);

  list_destroy(l);

  return;
}

int
wrap_list_count(const char *file, const char *function, int line, List l)
{
  assert(file != NULL);
  assert(function != NULL);

  if (!l)
    err_exit("list_count(%s(%s:%d)): null l pointer", file, function, line);

  return list_count(l);
}

void *
wrap_list_append (const char *file, const char *function, int line, List l, void *x)
{
  void *ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!l)
    err_exit("list_append(%s(%s:%d)): null l pointer", file, function, line);

  if (!x)
    err_exit("list_append(%s(%s:%d)): null x pointer", file, function, line);

  if (!(ret = list_append(l, x)))
    err_exit("list_append(%s(%s:%d)): %s", file, function, line, strerror(errno));
  
  return ret;
}

int 
wrap_list_delete_all(const char *file, const char *function, int line, List l, ListFindF f, void *key)
{
  int ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!l)
    err_exit("list_delete_all(%s(%s:%d)): null l pointer", file, function, line);

  if (!f)
    err_exit("list_delete_all(%s(%s:%d)): null f pointer", file, function, line);

  if (!key)
    err_exit("list_delete_all(%s(%s:%d)): null key pointer", file, function, line);

  if ((ret = list_delete_all(l, f, key)) < 0)
    err_exit("list_delete_all(%s(%s:%d)): %s", file, function, line, strerror(errno));
  
  return ret;
}

int
wrap_list_for_each(const char *file, const char *function, int line, List l, ListForF f, void *arg)
{
  int ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!l)
    err_exit("list_for_each(%s(%s:%d)): null l pointer", file, function, line);

  if (!f)
    err_exit("list_for_each(%s(%s:%d)): null f pointer", file, function, line);

  /* arg can be null */

  if ((ret = list_for_each(l, f, arg)) < 0)
    err_exit("list_for_each(%s(%s:%d)): %s", file, function, line, strerror(errno));
  
  return ret;
}

ListIterator
wrap_list_iterator_create(const char *file, const char *function, int line, List l)
{
  ListIterator ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!l)
    err_exit("list_iterator_create(%s(%s:%d)): null l pointer", file, function, line);

  if (!(ret = list_iterator_create(l)))
    err_exit("list_iterator_create(%s(%s:%d)): %s", file, function, line, strerror(errno));

  return ret;
}

void
wrap_list_iterator_destroy(const char *file, const char *function, int line, ListIterator i)
{
  assert(file != NULL);
  assert(function != NULL);

  if (!i)
    err_exit("list_iterator_destroy(%s(%s:%d)): null i pointer", file, function, line);

  list_iterator_destroy(i);

  return;
}

hash_t 
wrap_hash_create(const char *file, const char *function, int line, int size, hash_key_f key_f, hash_cmp_f cmp_f, hash_del_f del_f)
{
  hash_t ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!(size > 0 || size <= INT_MAX))
    err_exit("hash_create(%s(%s:%d)): invalid size: %d", file, function, line, size);

  if (!key_f)
    err_exit("hash_create(%s(%s:%d)): null key_f pointer", file, function, line);

  if (!cmp_f)
    err_exit("hash_create(%s(%s:%d)): null cmp_f pointer", file, function, line);

  /* del_f can be null */

  if (!(ret = hash_create(size, key_f, cmp_f, del_f)))
    err_exit("hash_create(%s(%s:%d)): size=%d: %s", file, function, line, size, strerror(errno));

  return ret;
}

int 
wrap_hash_count(const char *file, const char *function, int line, hash_t h)
{
  int ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!h)
    err_exit("hash_count(%s(%s:%d)): null h pointer", file, function, line);

  if (!(ret = hash_count(h)))
    {
      if (errno != 0)
        err_exit("hash_count(%s(%s:%d)): %s", file, function, line, strerror(errno));
    }

  return ret;
}

void *
wrap_hash_find(const char *file, const char *function, int line, hash_t h, const void *key)
{
  void *ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!h)
    err_exit("hash_find(%s(%s:%d)): null h pointer", file, function, line);

  if (!key)
    err_exit("hash_find(%s(%s:%d)): null key pointer", file, function, line);

  ret = hash_find(h, key);
  if (!ret && errno != 0)
    err_exit("hash_find(%s(%s:%d)): key=%s: %s", file, function, line, key, strerror(errno));

  return ret;
}

void *
wrap_hash_insert(const char *file, const char *function, int line, hash_t h, const void *key, void *data)
{
  void *ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!h)
    err_exit("hash_insert(%s(%s:%d)): null h pointer", file, function, line);

  if (!key)
    err_exit("hash_insert(%s(%s:%d)): null key pointer", file, function, line);

  if (!data)
    err_exit("hash_insert(%s(%s:%d)): null data pointer", file, function, line);

  if (!(ret = hash_insert(h, key, data)))
    err_exit("hash_insert(%s(%s:%d)): key=%s: %s", file, function, line, key, strerror(errno));
  if (ret != data)
    err_exit("hash_insert(%s(%s:%d)): key=%s: invalid insert", file, function, line, key);

  return ret;
}

void *
wrap_hash_remove (const char *file, const char *function, int line, hash_t h, const void *key)
{
  void *ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!h)
    err_exit("hash_remove(%s(%s:%d)): null h pointer", file, function, line);

  if (!key)
    err_exit("hash_remove(%s(%s:%d)): null key pointer", file, function, line);

  if (!(ret = hash_remove(h, key)))
    err_exit("hash_remove(%s(%s:%d)): key=%s: %s", file, function, line, key, strerror(errno));

  return ret;
}

int 
wrap_hash_delete_if(const char *file, const char *function, int line, hash_t h, hash_arg_f argf, void *arg)
{
  int ret;

  assert(file != NULL);
  assert(function != NULL);
    
  if (!h)
    err_exit("hash_delete_if(%s(%s:%d)): null h pointer", file, function, line);

  if (!argf)
    err_exit("hash_delete_if(%s(%s:%d)): null argf pointer", file, function, line);

  /* arg can be null */

  if ((ret = hash_delete_if(h, argf, arg)) < 0)
    err_exit("hash_delete_if(%s(%s:%d)): %s", file, function, line, strerror(errno));

  return ret;
}

int 
wrap_hash_for_each(const char *file, const char *function, int line, hash_t h, hash_arg_f argf, void *arg)
{
  int ret;

  assert(file != NULL);
  assert(function != NULL);

  if (!h)
    err_exit("hash_for_each(%s(%s:%d)): null h pointer", file, function, line);

  if (!argf)
    err_exit("hash_for_each(%s(%s:%d)): null argf pointer", file, function, line);

  /* arg can be null */

  if ((ret = hash_for_each(h, argf, arg)) < 0)
    err_exit("hash_for_each(%s(%s:%d)): %s", file, function, line, strerror(errno));

  return ret;
}

void
wrap_hash_destroy(const char *file, const char *function, int line, hash_t h)
{
  assert(file != NULL);
  assert(function != NULL);

  if (!h)
    err_exit("hash_destroy(%s(%s:%d)): null h pointer", file, function, line);

  hash_destroy(h);

  return;
}
