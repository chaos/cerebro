/*****************************************************************************\
 *  $Id: cerebrod_wrappers.c,v 1.3 2005-06-16 21:35:34 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <limits.h>
#include <assert.h>
#include <errno.h>

#include "cerebro/cerebro_error.h"

#include "cerebrod_wrappers.h"

#include "fd.h"

extern int h_errno;

#define MALLOC_MAGIC      0xf0e0d0c0
#define MALLOC_PAD_DATA   0xab
#define MALLOC_PAD_LEN    16

void *
wrap_malloc(const char *file, const char *function, int line, size_t size)
{
  void *ptr;

  assert(file);
  assert(function);

  if (!(size > 0 || size <= INT_MAX))
    cerebro_err_exit("malloc(%s(%s:%d)): invalid size: %d", file, function, line, size);

  if (!(ptr = malloc(2*sizeof(int) + size + MALLOC_PAD_LEN))) 
    cerebro_err_exit("malloc(%s(%s:%d)): %s", file, function, line, strerror(errno));

  *((int *)(ptr)) = MALLOC_MAGIC;
  *((int *)(ptr+sizeof(int))) = size;
  memset(ptr+2*sizeof(int), '\0', size);
  memset(ptr+2*sizeof(int) + size, MALLOC_PAD_DATA, MALLOC_PAD_LEN);
  return (ptr + 2*sizeof(int));
}

void
wrap_free(const char *file, const char *function, int line, void *ptr)
{
  assert(file);
  assert(function);

  if (ptr)
    {
      void *p = ptr - 2*sizeof(int);
      int i, size;
      char *c;

      /* assert(*((int *)p) == MALLOC_MAGIC); */
      if (!(*((int *)p) == MALLOC_MAGIC))
        cerebro_err_exit("free(%s(%s:%d)): memory corruption", file, function, line);

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

  assert(file);
  assert(function);

  if (!s)
    cerebro_err_exit("strdup(%s(%s:%d)): null s pointer", file, function, line);

  ptr = wrap_malloc(file, function, line, strlen(s) + 1);
  strcpy(ptr, s);
  return ptr;
}

char *
wrap_strncpy(const char *file, const char *function, int line, char *dest, const char *src, size_t n)
{
  char *rv;

  assert(file);
  assert(function);

  if (!dest)
    cerebro_err_exit("strncpy(%s(%s:%d)): null dest pointer", file, function, line);

  if (!src)
    cerebro_err_exit("strncpy(%s(%s:%d)): null src pointer", file, function, line);

  rv = strncpy(dest, src, n);
  dest[n-1] = '\0';
  
  return rv;
}

int 
wrap_open(const char *file, const char *function, int line, const char *pathname, int flags, int mode)
{
  int fd;

  assert(file);
  assert(function);
  
  if (!pathname)
    cerebro_err_exit("open(%s(%s:%d)): null pathname pointer");

  if ((fd = open(pathname, flags, mode)) < 0)
    cerebro_err_exit("open(%s(%s:%d)): pathname=%s flags=%x mode=%o: %s", 
             file, function, line, pathname, flags, mode, strerror(errno));

  return fd;
}

int 
wrap_close(const char *file, const char *function, int line, int fd)
{
  int rv;
                                 
  assert(file);
  assert(function);
                                                   
  if ((rv = close(fd)) < 0)
    cerebro_err_exit("close(%s(%s:%d)): %s", file, function, line, strerror(errno));
  return rv;
}

ssize_t
wrap_read(const char *file, const char *function, int line, int fd, void *buf, size_t count)
{
  ssize_t rv;

  assert(file);
  assert(function);

  if (!buf)
    cerebro_err_exit("read(%s(%s:%d)): null buf pointer", file, function, line);

  if (!(count > 0 || count <= INT_MAX))
    cerebro_err_exit("read(%s(%s:%d)): invalid count: %d", file, function, line, count);

  if ((rv = fd_read_n(fd, buf, count)) < 0)
    cerebro_err_exit("read(%s(%s:%d)): count=%d: %s", 
             file, function, line, count, strerror(errno));
  
  return rv;
}

ssize_t
wrap_write(const char *file, const char *function, int line, int fd, const void *buf, size_t count)
{
  ssize_t rv;

  assert(file);
  assert(function);

  if (!buf)
    cerebro_err_exit("write(%s(%s:%d)): null buf pointer", file, function, line);

  if (!(count > 0 || count <= INT_MAX))
    cerebro_err_exit("write(%s(%s:%d)): invalid count: %d", file, function, line, count);

  if ((rv = fd_write_n(fd, buf, count)) < 0)
    cerebro_err_exit("write(%s(%s:%d)): count=%d: %s", 
             file, function, line, count, strerror(errno));

  return rv;
}

int
wrap_chdir(const char *file, const char *function, int line, const char *path)
{
  int rv;

  assert(file);
  assert(function);

  if (!path)
    cerebro_err_exit("chdir(%s(%s:%d)): null path pointer", file, function, line);

  if ((rv = chdir(path)) < 0)
    cerebro_err_exit("chdir(%s(%s:%d)): path=%s: %s", file, function, line, path, strerror(errno));

  return rv;
}

int 
wrap_stat(const char *file, const char *function, int line, const char *path, struct stat *buf)
{
  int rv;

  assert(file);
  assert(function);

  if (!path)
    cerebro_err_exit("stat(%s(%s:%d)): null path pointer", file, function, line);

  if (!buf)
    cerebro_err_exit("stat(%s(%s:%d)): null buf pointer", file, function, line);

  if ((rv = stat(path, buf)) < 0)
    cerebro_err_exit("stat(%s(%s:%d)): path=%s: %s", file, function, line, path, strerror(errno));

  return rv;
}

mode_t
wrap_umask(const char *file, const char *function, int line, mode_t mask)
{
  assert(file);
  assert(function);

  /* achu: never supposed to fail.  Go fig. */
  return umask(mask);
}

DIR *
wrap_opendir(const char *file, const char *function, int line, const char *name)
{
  DIR *rv;
  
  assert(file);
  assert(function);

  if (!name)
    cerebro_err_exit("opendir(%s(%s:%d)): null name pointer", file, function, line);

  if (!(rv = opendir(name)))
    cerebro_err_exit("opendir(%s(%s:%d)): name=%s: %s", file, function, line, name, strerror(errno));

  return rv;
}

int
wrap_closedir(const char *file, const char *function, int line, DIR *dir)
{
  int rv;

  assert(file);
  assert(function);

  if (!dir)
    cerebro_err_exit("closedir(%s(%s:%d)): null dir pointer", file, function, line);

  if ((rv = closedir(dir)) < 0)
    cerebro_err_exit("closedir(%s(%s:%d)): %s", file, function, line, strerror(errno));

  return rv;
}

int
wrap_socket(const char *file, const char *function, int line, int domain, int type, int protocol)
{
  int fd;

  assert(file);
  assert(function);

  if ((fd = socket(domain, type, protocol)) < 0)
    cerebro_err_exit("socket(%s(%s:%d)): domain=%x type=%x protocol=%x: %s", 
             file, function, line, domain, type, protocol, strerror(errno));

  return fd;
}

int 
wrap_bind(const char *file, const char *function, int line, int sockfd, struct sockaddr *my_addr, socklen_t addrlen)
{
  int rv;
  
  assert(file);
  assert(function);

  if (!my_addr)
    cerebro_err_exit("bind(%s(%s:%d)): null my_addr pointer", file, function, line);

  if (!(addrlen > 0 || addrlen <= INT_MAX))
    cerebro_err_exit("bind(%s(%s:%d)): invalid addrlen: %d", file, function, line, addrlen);

  if ((rv = bind(sockfd, my_addr, addrlen)) < 0)
    cerebro_err_exit("bind(%s(%s:%d)): addrlen=%d: %s", 
             file, function, line, addrlen, strerror(errno));

  return rv;
}

int 
wrap_connect(const char *file, const char *function, int line, int sockfd, struct sockaddr *serv_addr, socklen_t addrlen)
{
  int rv;

  assert(file);
  assert(function);

  if (!serv_addr)
    cerebro_err_exit("connect(%s(%s:%d)): null serv_addr pointer", file, function, line);

  if (!(addrlen > 0 || addrlen <= INT_MAX))
    cerebro_err_exit("connect(%s(%s:%d)): invalid addrlen: %d", file, function, line, addrlen);

  if ((rv = connect(sockfd, serv_addr, addrlen)) < 0)
    cerebro_err_exit("connect(%s(%s:%d)): addrlen=%d: %s", 
             file, function, line, addrlen, strerror(errno));

  return rv;
}

int 
wrap_listen(const char *file, const char *function, int line, int s, int backlog)
{
  int rv;

  assert(file);
  assert(function);

  if (!(backlog > 0 || backlog <= INT_MAX))
    cerebro_err_exit("listen(%s(%s:%d)): invalid backlog: %d", file, function, line, backlog);

  if ((rv = listen(s, backlog)) < 0)
    cerebro_err_exit("listen(%s(%s:%d)): backlog=%d: %s", 
             file, function, line, backlog, strerror(errno));

  return rv;
}

int 
wrap_accept(const char *file, const char *function, int line, int s, struct sockaddr *addr, socklen_t *addrlen)
{
  int rv;

  assert(file);
  assert(function);

  if (!addr)
    cerebro_err_exit("accept(%s(%s:%d)): null addr pointer", file, function, line);

  if (!addrlen)
    cerebro_err_exit("accept(%s(%s:%d)): null addrlen pointer", file, function, line);

  if (!(*addrlen > 0 || *addrlen <= INT_MAX))
    cerebro_err_exit("accept(%s(%s:%d)): invalid addrlen: %d", file, function, line, *addrlen);

  if ((rv = accept(s, addr, addrlen)) < 0)
    cerebro_err_exit("accept(%s(%s:%d)): %s", file, function, line, strerror(errno));

  return rv;
}

int 
wrap_select(const char *file, const char *function, int line, int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
  int rv;
  struct timeval timeout_orig, timeout_current;
  struct timeval start, end, delta;
  
  assert(file);
  assert(function);

  /* readfds, writefds, exceptfds, and timeout could each be null, but
   * not all can be null at the same time
   */
  if (!readfds && !writefds && !exceptfds && !timeout)
    cerebro_err_exit("select(%s(%s:%d)): all null pointers", file, function, line);

  if (timeout) 
    {
      timeout_orig = *timeout;
      timeout_current = *timeout;
      Gettimeofday(&start, NULL);
    }

  do 
    {
      rv = select(n, readfds, writefds, exceptfds, &timeout_current);
      if (rv < 0 && errno != EINTR)
	cerebro_err_exit("select(%s(%s:%d)): %s", strerror(errno));
      if (rv < 0 && timeout) 
	{
	  Gettimeofday(&end, NULL);
	  /* delta = end-start */
	  timersub(&end, &start, &delta);     
	  /* timeout_current = timeout_orig-delta */
	  timersub(&timeout_orig, &delta, &timeout_current);     
	}
    } 
  while (rv < 0);

  return rv;
}

int 
wrap_poll(const char *file, const char *function, int line, struct pollfd *ufds, unsigned int nfds, int timeout)
{
  int rv;
  struct timeval timeout_orig, timeout_current;
  struct timeval start, end, delta;
                                                                         
  if (!ufds)  
    cerebro_err_exit("poll(%s(%s:%d)): null ufds pointer", file, function, line);

  /* timeout can be <= 0 */

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
      cerebro_err_exit("poll(%s(%s:%d)): %s", strerror(errno));
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
wrap_getsockopt(const char *file, const char *function, int line, int s, int level, int optname, void *optval, socklen_t *optlen)
{
  int rv;

  assert(file);
  assert(function);

  if (!optval)
    cerebro_err_exit("getsockopt(%s(%s:%d)): null optval pointer", file, function, line);

  if (!optlen)  
    cerebro_err_exit("getsockopt(%s(%s:%d)): null optlen pointer", file, function, line);

  if (!(*optlen > 0 || *optlen <= INT_MAX))
    cerebro_err_exit("getsockopt(%s(%s:%d)): invalid *optlen: %d", file, function, line, *optlen);

  if ((rv = getsockopt(s, level, optname, optval, optlen)) < 0)
    cerebro_err_exit("getsockopt(%s(%s:%d)): optname=%x: %s", 
             file, function, line, optname, strerror(errno));

  return rv;
}

int
wrap_setsockopt(const char *file, const char *function, int line, int s, int level, int optname, const void *optval, socklen_t optlen)
{
  int rv;

  assert(file);
  assert(function);

  if (!optval)
    cerebro_err_exit("setsockopt(%s(%s:%d)): null optval pointer", file, function, line);

  if (!(optlen > 0 || optlen <= INT_MAX))
    cerebro_err_exit("setsockopt(%s(%s:%d)): invalid optlen: %d", file, function, line, optlen);

  if ((rv = setsockopt(s, level, optname, optval, optlen)) < 0)
    cerebro_err_exit("setsockopt(%s(%s:%d)): %s", file, function, line, strerror(errno));

  return rv;
}

struct hostent *
wrap_gethostbyname(const char *file, const char *function, int line, const char *name)
{
  struct hostent *rv;

  assert(file);
  assert(function);

  if (!name)
    cerebro_err_exit("gethostbyname(%s(%s:%d)): null name pointer", file, function, line);

  if (!(rv = gethostbyname(name)))
    cerebro_err_exit("gethostbyname(%s(%s:%d)): name=%s: %s", file, function, line, name, hstrerror(h_errno));

  return rv;
}

const char *
wrap_inet_ntop(const char *file, const char *function, int line, int af, const void *src, char *dst, socklen_t cnt)
{
  const char *rv;

  assert(file);
  assert(function);

  if (!src)
    cerebro_err_exit("inet_ntop(%s(%s:%d)): null src pointer", file, function, line);

  if (!dst)
    cerebro_err_exit("inet_ntop(%s(%s:%d)): null dst pointer", file, function, line);

  if ((rv = inet_ntop(af, src, dst, cnt)) < 0)
    cerebro_err_exit("inet_ntop(%s(%s:%d)): af=%x: %s", file, function, line, af, strerror(errno));
  
  return rv;
}

int 
wrap_inet_pton(const char *file, const char *function, int line, int af, const char *src, void *dst)
{
  int rv;
  
  assert(file);
  assert(function);

  if (!src)
    cerebro_err_exit("inet_pton(%s(%s:%d)): null src pointer", file, function, line);

  if (!dst)
    cerebro_err_exit("inet_pton(%s(%s:%d)): null dst pointer", file, function, line);

  if ((rv = inet_pton(af, src, dst)) < 0)
    cerebro_err_exit("inet_pton(%s(%s:%d)): af=%x: %s", file, function, line, af, strerror(errno));

  return rv;
}

int
wrap_gettimeofday(const char *file, const char *function, int line, struct timeval *tv, struct timezone *tz)
{
  int rv;

  assert(file);
  assert(function);

  if (!tv)
    cerebro_err_exit("gettimeofday(%s(%s:%d)): null tv pointer", file, function, line);

  /* tz can be null */

  if ((rv = gettimeofday(tv, tz)) < 0)
    cerebro_err_exit("gettimeofday(%s(%s:%d)): %s", file, function, line, strerror(errno));

  return rv;
}

time_t 
wrap_time(const char *file, const char *function, int line, time_t *t)
{
  time_t rv;

  assert(file);
  assert(function);

  /* t can be null */

  if ((rv = time(t)) == ((time_t)-1))
    cerebro_err_exit("time(%s(%s:%d)): %s", file, function, line, strerror(errno));

  return rv;
}

struct tm *
wrap_localtime(const char *file, const char *function, int line, const time_t *timep)
{
  struct tm *tmptr;
  
  assert(file);
  assert(function);

  if (!timep)
    cerebro_err_exit("localtime(%s(%s:%d)): null timep pointer", file, function, line);

  if (!(tmptr = localtime(timep)))
    cerebro_err_exit("localtime(%s(%s:%d))", file, function, line);

  return tmptr;
}

struct tm *
wrap_localtime_r(const char *file, const char *function, int line, const time_t *timep, struct tm *result)
{
  struct tm *tmptr;
  
  assert(file);
  assert(function);

  if (!timep)
    cerebro_err_exit("localtime_r(%s(%s:%d)): null timep pointer", file, function, line);

  if (!result)
    cerebro_err_exit("localtime_r(%s(%s:%d)): null result pointer", file, function, line);

  if (!(tmptr = localtime_r(timep, result)))
    cerebro_err_exit("localtime(%s(%s:%d))", file, function, line);

  return tmptr;
}

int 
wrap_pthread_create(const char *file, const char *function, int line, pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *), void *arg)
{
  int rv;
  
  assert(file);
  assert(function);

  if (!thread)
    cerebro_err_exit("pthread_create(%s(%s:%d)): null thread pointer", file, function, line);

  /* attr can be null */

  if (!start_routine)
    cerebro_err_exit("pthread_create(%s(%s:%d)): null start_routine pointer", file, function, line);

  if ((rv = pthread_create(thread, attr, start_routine, arg)) != 0)
    cerebro_err_exit("pthread_create(%s(%s:%d)): %s", file, function, line, strerror(rv));

  return rv;
}

int 
wrap_pthread_attr_init(const char *file, const char *function, int line, pthread_attr_t *attr)
{
  int rv;
  
  assert(file);
  assert(function);

  if (!attr)
    cerebro_err_exit("pthread_attr_init(%s(%s:%d)): null attr pointer", file, function, line);

  if ((rv = pthread_attr_init(attr)) != 0)
    cerebro_err_exit("pthread_attr_init(%s(%s:%d)): %s", file, function, line, strerror(rv));

  return rv;
}

int 
wrap_pthread_attr_destroy(const char *file, const char *function, int line, pthread_attr_t *attr)
{
  int rv;
  
  assert(file);
  assert(function);

  if (!attr)
    cerebro_err_exit("pthread_attr_destroy(%s(%s:%d)): null attr pointer", file, function, line);

  if ((rv = pthread_attr_destroy(attr)) != 0)
    cerebro_err_exit("pthread_attr_destroy(%s(%s:%d)): %s", file, function, line, strerror(rv));

  return rv;
}

int 
wrap_pthread_attr_setdetachstate(const char *file, const char *function, int line, pthread_attr_t *attr, int detachstate)
{
  int rv;
  
  assert(file);
  assert(function);

  if (!attr)
    cerebro_err_exit("pthread_attr_setdetachstate(%s(%s:%d)): null attr pointer", file, function, line);

  if ((rv = pthread_attr_setdetachstate(attr, detachstate)) != 0)
    cerebro_err_exit("pthread_attr_setdetachstate(%s(%s:%d)): detachstate=%d: %s", 
             file, function, line, detachstate, strerror(rv));

  return rv;
}

int
wrap_pthread_mutex_lock(const char *file, const char *function, int line, pthread_mutex_t *mutex)
{
  int rv;
  
  assert(file);
  assert(function);

  if (!mutex)
    cerebro_err_exit("pthread_mutex_lock(%s(%s:%d)): null mutex pointer", file, function, line);

  if ((rv = pthread_mutex_lock(mutex)) != 0)
    cerebro_err_exit("pthread_mutex_lock(%s(%s:%d)): %s", file, function, line, strerror(rv));

  return rv;
}

int 
wrap_pthread_mutex_trylock(const char *file, const char *function, int line, pthread_mutex_t *mutex)
{
  int rv;
  
  assert(file);
  assert(function);

  if (!mutex)
    cerebro_err_exit("pthread_mutex_trylock(%s(%s:%d)): null mutex pointer", file, function, line);

  rv = pthread_mutex_trylock(mutex);
  if (rv != 0 && rv != EBUSY)
    cerebro_err_exit("pthread_mutex_trylock(%s(%s:%d)): %s", file, function, line, strerror(rv));

  return rv;
}

int 
wrap_pthread_mutex_unlock(const char *file, const char *function, int line, pthread_mutex_t *mutex)
{
  int rv;
  
  assert(file);
  assert(function);

  if (!mutex)
    cerebro_err_exit("pthread_mutex_unlock(%s(%s:%d)): null mutex pointer", file, function, line);

  if ((rv = pthread_mutex_unlock(mutex)) != 0)
    cerebro_err_exit("pthread_mutex_unlock(%s(%s:%d)): %s", file, function, line, strerror(rv));

  return rv;
}

int
wrap_pthread_mutex_init(const char *file, const char *function, int line, pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr)
{
  int rv;
  
  assert(file);
  assert(function);

  if (!mutex)
    cerebro_err_exit("pthread_mutex_init(%s(%s:%d)): null mutex pointer", file, function, line);

  /* mutexattr can be null */

  if ((rv = pthread_mutex_init(mutex, mutexattr)) != 0)
    cerebro_err_exit("pthread_mutex_init(%s(%s:%d)): %s", file, function, line, strerror(rv));

  return rv;
}

int 
wrap_pthread_cond_signal(const char *file, const char *function, int line, pthread_cond_t *cond)
{
  int rv;

  assert(file);
  assert(function);

  if (!cond)
    cerebro_err_exit("pthread_cond_signal(%s(%s:%d)): null cond pointer", file, function, line);

  if ((rv = pthread_cond_signal(cond)) != 0)
    cerebro_err_exit("pthread_cond_signal(%s(%s:%d)): %s", file, function, line, strerror(rv));

  return rv;
}

int 
wrap_pthread_cond_wait(const char *file, const char *function, int line, pthread_cond_t *cond, pthread_mutex_t *mutex)
{
  int rv;

  assert(file);
  assert(function);

  if (!cond)
    cerebro_err_exit("pthread_cond_wait(%s(%s:%d)): null cond pointer", file, function, line);

  if (!mutex)
    cerebro_err_exit("pthread_cond_wait(%s(%s:%d)): null mutex pointer", file, function, line);

  if ((rv = pthread_cond_wait(cond, mutex)) != 0)
    cerebro_err_exit("pthread_cond_signal(%s(%s:%d)): %s", file, function, line, strerror(rv));

  return rv;
}

pid_t 
wrap_fork(const char *file, const char *function, int line)
{
  pid_t pid;

  assert(file);
  assert(function);

  if ((pid = fork()) < 0)
    cerebro_err_exit("fork(%s(%s:%d)): %s", file, function, line, strerror(errno));

  return pid;
}

Sighandler_t
wrap_signal(const char *file, const char *function, int line, int signum, Sighandler_t handler)
{
  Sighandler_t rv;

  assert(file);
  assert(function);

  if (!handler)
    cerebro_err_exit("signal(%s(%s:%d)): null handler pointer", file, function, line);

  if ((rv = signal(signum, handler)) == SIG_ERR)
    cerebro_err_exit("signal(%s(%s:%d)): %s", file, function, line, strerror(errno));

  return rv;
}

int 
wrap_gethostname(const char *file, const char *function, int line, char *name, size_t len)
{
  int rv;

  assert(file);
  assert(function);

  if (!name)
    cerebro_err_exit("gethostname(%s(%s:%d)): null name pointer", file, function, line);

  if (!(len > 0 || len <= INT_MAX))
    cerebro_err_exit("gethostname(%s(%s:%d)): invalid len: %d", file, function, line, len);

  if ((rv = gethostname(name, len)) < 0)
    cerebro_err_exit("gethostname(%s(%s:%d)): len=%d: %s", 
             file, function, line, len, strerror(errno));

  return rv;
}

int
wrap_lt_dlinit(const char *file, const char *function, int line)
{
  int rv;

  assert(file);
  assert(function);

  if ((rv = lt_dlinit()) != 0)
    cerebro_err_exit("lt_dlinit(%s(%s:%d)): %s", file, function, line, lt_dlerror());

  return rv;
}

int
wrap_lt_dlexit(const char *file, const char *function, int line)
{
  int rv;

  assert(file);
  assert(function);

  if ((rv = lt_dlexit()) != 0)
    cerebro_err_exit("lt_dlexit(%s(%s:%d)): %s", file, function, line, lt_dlerror());

  return rv;
}

lt_dlhandle
wrap_lt_dlopen(const char *file, const char *function, int line, const char *filename)
{
  lt_dlhandle rv;

  assert(file);
  assert(function);

  if (!filename)
    cerebro_err_exit("lt_dlopen(%s(%s:%d)): null filename pointer");

  if (!(rv = lt_dlopen(filename)))
    cerebro_err_exit("lt_dlopen(%s(%s:%d)): filename=%s: %s", file, function, line, filename, lt_dlerror());

  return rv;
}

lt_ptr
wrap_lt_dlsym(const char *file, const char *function, int line, void *handle, char *symbol)
{
  lt_ptr *rv;
  const char *err;

  assert(file);
  assert(function);

  if (!handle)
    cerebro_err_exit("lt_dlsym(%s(%s:%d)): null handle pointer");

  if (!symbol)
    cerebro_err_exit("lt_dlsym(%s(%s:%d)): null symbol pointer");

  /* "clear" lt_dlerror() */
  lt_dlerror();

  if (!(rv = lt_dlsym(handle, symbol)))
    {
      err = lt_dlerror();
      if (err)
        cerebro_err_exit("lt_dlsym(%s(%s:%d)): symbol=%s: %s", file, function, line, symbol, err);
    }

  return rv;
}

int 
wrap_lt_dlclose(const char *file, const char *function, int line, void *handle)
{
  int rv;

  assert(file);
  assert(function);

  if (!handle)
    cerebro_err_exit("lt_dlclose(%s(%s:%d)): null handle pointer");

  if ((rv = lt_dlclose(handle)) != 0)
    cerebro_err_exit("lt_dlclose(%s(%s:%d)): %s", lt_dlerror());

  return rv;
}

List 
wrap_list_create(const char *file, const char *function, int line, ListDelF f)
{
  List rv;

  assert(file);
  assert(function);

  /* f can be null */

  if (!(rv = list_create(f)))
    cerebro_err_exit("list_create(%s(%s:%d)): %s", file, function, line, strerror(errno));

  return rv;
}

void
wrap_list_destroy(const char *file, const char *function, int line, List l)
{
  assert(file);
  assert(function);

  if (!l)
    cerebro_err_exit("list_destroy(%s(%s:%d)): null l pointer", file, function, line);

  list_destroy(l);

  return;
}

int
wrap_list_count(const char *file, const char *function, int line, List l)
{
  assert(file);
  assert(function);

  if (!l)
    cerebro_err_exit("list_count(%s(%s:%d)): null l pointer", file, function, line);

  return list_count(l);
}

void *
wrap_list_append (const char *file, const char *function, int line, List l, void *x)
{
  void *rv;

  assert(file);
  assert(function);

  if (!l)
    cerebro_err_exit("list_append(%s(%s:%d)): null l pointer", file, function, line);

  if (!x)
    cerebro_err_exit("list_append(%s(%s:%d)): null x pointer", file, function, line);

  if (!(rv = list_append(l, x)))
    cerebro_err_exit("list_append(%s(%s:%d)): %s", file, function, line, strerror(errno));
  
  return rv;
}

void * 
wrap_list_find_first (const char *file, const char *function, int line, List l, ListFindF f, void *key)
{
  void *rv;

  assert(file);
  assert(function);

  if (!l)
    cerebro_err_exit("list_append(%s(%s:%d)): null l pointer", file, function, line);

  if (!f)
    cerebro_err_exit("list_append(%s(%s:%d)): null f pointer", file, function, line);

  if (!key)
    cerebro_err_exit("list_append(%s(%s:%d)): null key pointer", file, function, line);

  rv = list_find_first(l, f, key);
  
  return rv;
}

int 
wrap_list_delete_all(const char *file, const char *function, int line, List l, ListFindF f, void *key)
{
  int rv;

  assert(file);
  assert(function);

  if (!l)
    cerebro_err_exit("list_delete_all(%s(%s:%d)): null l pointer", file, function, line);

  if (!f)
    cerebro_err_exit("list_delete_all(%s(%s:%d)): null f pointer", file, function, line);

  if (!key)
    cerebro_err_exit("list_delete_all(%s(%s:%d)): null key pointer", file, function, line);

  if ((rv = list_delete_all(l, f, key)) < 0)
    cerebro_err_exit("list_delete_all(%s(%s:%d)): %s", file, function, line, strerror(errno));
  
  return rv;
}

int
wrap_list_for_each(const char *file, const char *function, int line, List l, ListForF f, void *arg)
{
  int rv;

  assert(file);
  assert(function);

  if (!l)
    cerebro_err_exit("list_for_each(%s(%s:%d)): null l pointer", file, function, line);

  if (!f)
    cerebro_err_exit("list_for_each(%s(%s:%d)): null f pointer", file, function, line);

  /* arg can be null */

  if ((rv = list_for_each(l, f, arg)) < 0)
    cerebro_err_exit("list_for_each(%s(%s:%d)): %s", file, function, line, strerror(errno));
  
  return rv;
}

ListIterator
wrap_list_iterator_create(const char *file, const char *function, int line, List l)
{
  ListIterator rv;

  assert(file);
  assert(function);

  if (!l)
    cerebro_err_exit("list_iterator_create(%s(%s:%d)): null l pointer", file, function, line);

  if (!(rv = list_iterator_create(l)))
    cerebro_err_exit("list_iterator_create(%s(%s:%d)): %s", file, function, line, strerror(errno));

  return rv;
}

void
wrap_list_iterator_destroy(const char *file, const char *function, int line, ListIterator i)
{
  assert(file);
  assert(function);

  if (!i)
    cerebro_err_exit("list_iterator_destroy(%s(%s:%d)): null i pointer", file, function, line);

  list_iterator_destroy(i);

  return;
}

hash_t 
wrap_hash_create(const char *file, const char *function, int line, int size, hash_key_f key_f, hash_cmp_f cmp_f, hash_del_f del_f)
{
  hash_t rv;

  assert(file);
  assert(function);

  if (!(size > 0 || size <= INT_MAX))
    cerebro_err_exit("hash_create(%s(%s:%d)): invalid size: %d", file, function, line, size);

  if (!key_f)
    cerebro_err_exit("hash_create(%s(%s:%d)): null key_f pointer", file, function, line);

  if (!cmp_f)
    cerebro_err_exit("hash_create(%s(%s:%d)): null cmp_f pointer", file, function, line);

  /* del_f can be null */

  if (!(rv = hash_create(size, key_f, cmp_f, del_f)))
    cerebro_err_exit("hash_create(%s(%s:%d)): size=%d: %s", file, function, line, size, strerror(errno));

  return rv;
}

int 
wrap_hash_count(const char *file, const char *function, int line, hash_t h)
{
  int rv;

  assert(file);
  assert(function);

  if (!h)
    cerebro_err_exit("hash_count(%s(%s:%d)): null h pointer", file, function, line);

  if (!(rv = hash_count(h)))
    {
      if (errno != 0)
        cerebro_err_exit("hash_count(%s(%s:%d)): %s", file, function, line, strerror(errno));
    }

  return rv;
}

void *
wrap_hash_find(const char *file, const char *function, int line, hash_t h, const void *key)
{
  void *rv;

  assert(file);
  assert(function);

  if (!h)
    cerebro_err_exit("hash_find(%s(%s:%d)): null h pointer", file, function, line);

  if (!key)
    cerebro_err_exit("hash_find(%s(%s:%d)): null key pointer", file, function, line);

  rv = hash_find(h, key);
  if (!rv && errno != 0)
    cerebro_err_exit("hash_find(%s(%s:%d)): key=%s: %s", file, function, line, key, strerror(errno));

  return rv;
}

void *
wrap_hash_insert(const char *file, const char *function, int line, hash_t h, const void *key, void *data)
{
  void *rv;

  assert(file);
  assert(function);

  if (!h)
    cerebro_err_exit("hash_insert(%s(%s:%d)): null h pointer", file, function, line);

  if (!key)
    cerebro_err_exit("hash_insert(%s(%s:%d)): null key pointer", file, function, line);

  if (!data)
    cerebro_err_exit("hash_insert(%s(%s:%d)): null data pointer", file, function, line);

  if (!(rv = hash_insert(h, key, data)))
    cerebro_err_exit("hash_insert(%s(%s:%d)): key=%s: %s", file, function, line, key, strerror(errno));
  if (rv != data)
    cerebro_err_exit("hash_insert(%s(%s:%d)): key=%s: invalid insert", file, function, line, key);

  return rv;
}

void *
wrap_hash_remove (const char *file, const char *function, int line, hash_t h, const void *key)
{
  void *rv;

  assert(file);
  assert(function);

  if (!h)
    cerebro_err_exit("hash_remove(%s(%s:%d)): null h pointer", file, function, line);

  if (!key)
    cerebro_err_exit("hash_remove(%s(%s:%d)): null key pointer", file, function, line);

  if (!(rv = hash_remove(h, key)))
    cerebro_err_exit("hash_remove(%s(%s:%d)): key=%s: %s", file, function, line, key, strerror(errno));

  return rv;
}

int 
wrap_hash_delete_if(const char *file, const char *function, int line, hash_t h, hash_arg_f argf, void *arg)
{
  int rv;

  assert(file);
  assert(function);
    
  if (!h)
    cerebro_err_exit("hash_delete_if(%s(%s:%d)): null h pointer", file, function, line);

  if (!argf)
    cerebro_err_exit("hash_delete_if(%s(%s:%d)): null argf pointer", file, function, line);

  /* arg can be null */

  if ((rv = hash_delete_if(h, argf, arg)) < 0)
    cerebro_err_exit("hash_delete_if(%s(%s:%d)): %s", file, function, line, strerror(errno));

  return rv;
}

int 
wrap_hash_for_each(const char *file, const char *function, int line, hash_t h, hash_arg_f argf, void *arg)
{
  int rv;

  assert(file);
  assert(function);

  if (!h)
    cerebro_err_exit("hash_for_each(%s(%s:%d)): null h pointer", file, function, line);

  if (!argf)
    cerebro_err_exit("hash_for_each(%s(%s:%d)): null argf pointer", file, function, line);

  /* arg can be null */

  if ((rv = hash_for_each(h, argf, arg)) < 0)
    cerebro_err_exit("hash_for_each(%s(%s:%d)): %s", file, function, line, strerror(errno));

  return rv;
}

void
wrap_hash_destroy(const char *file, const char *function, int line, hash_t h)
{
  assert(file);
  assert(function);

  if (!h)
    cerebro_err_exit("hash_destroy(%s(%s:%d)): null h pointer", file, function, line);

  hash_destroy(h);

  return;
}

int 
wrap_marshall_int8(const char *file, const char *function, int line, int8_t val, char *buf, unsigned int buflen)
{
  int rv;

  assert(file);
  assert(function);

  if (!buf)
    cerebro_err_exit("marshall_int8(%s(%s:%d)): null buf pointer", file, function, line);

  if ((rv = marshall_int8(val, buf, buflen)) <= 0)
    cerebro_err_exit("marshall_int8(%s(%s:%d)): %s\n", strerror(errno));

  return rv;
}

int 
wrap_marshall_int32(const char *file, const char *function, int line, int32_t val, char *buf, unsigned int buflen)
{
  int rv;

  assert(file);
  assert(function);

  if (!buf)
    cerebro_err_exit("marshall_int32(%s(%s:%d)): null buf pointer", file, function, line);

  if ((rv = marshall_int32(val, buf, buflen)) <= 0)
    cerebro_err_exit("marshall_int32(%s(%s:%d)): %s\n", strerror(errno));

  return rv;
}

int 
wrap_marshall_u_int8(const char *file, const char *function, int line, u_int8_t val, char *buf, unsigned int buflen)
{
  int rv;

  assert(file);
  assert(function);

  if (!buf)
    cerebro_err_exit("marshall_u_int8(%s(%s:%d)): null buf pointer", file, function, line);

  if ((rv = marshall_u_int8(val, buf, buflen)) <= 0)
    cerebro_err_exit("marshall_u_int8(%s(%s:%d)): %s\n", strerror(errno));

  return rv;
}

int 
wrap_marshall_u_int32(const char *file, const char *function, int line, u_int32_t val, char *buf, unsigned int buflen)
{
  int rv;

  assert(file);
  assert(function);

  if (!buf)
    cerebro_err_exit("marshall_u_int32(%s(%s:%d)): null buf pointer", file, function, line);

  if ((rv = marshall_u_int32(val, buf, buflen)) <= 0)
    cerebro_err_exit("marshall_u_int32(%s(%s:%d)): %s\n", strerror(errno));

  return rv;
}

int 
wrap_marshall_float(const char *file, const char *function, int line, float val, char *buf, unsigned int buflen)
{
  int rv;

  assert(file);
  assert(function);

  if (!buf)
    cerebro_err_exit("marshall_float(%s(%s:%d)): null buf pointer", file, function, line);

  if ((rv = marshall_float(val, buf, buflen)) <= 0)
    cerebro_err_exit("marshall_float(%s(%s:%d)): %s\n", strerror(errno));

  return rv;
}

int 
wrap_marshall_double(const char *file, const char *function, int line, double val, char *buf, unsigned int buflen)
{
  int rv;

  assert(file);
  assert(function);

  if (!buf)
    cerebro_err_exit("marshall_double(%s(%s:%d)): null buf pointer", file, function, line);

  if ((rv = marshall_double(val, buf, buflen)) <= 0)
    cerebro_err_exit("marshall_double(%s(%s:%d)): %s\n", strerror(errno));

  return rv;
}

int 
wrap_marshall_buffer(const char *file, const char *function, int line, const char *val, unsigned int vallen, char *buf, unsigned int buflen)
{
  int rv;

  assert(file);
  assert(function);

  if (!buf)
    cerebro_err_exit("marshall_buffer(%s(%s:%d)): null buf pointer", file, function, line);

  if ((rv = marshall_buffer(val, vallen, buf, buflen)) <= 0)
    cerebro_err_exit("marshall_buffer(%s(%s:%d)): %s\n", strerror(errno));

  return rv;
}

int 
wrap_unmarshall_int8(const char *file, const char *function, int line, int8_t *val, const char *buf, unsigned int buflen)
{
  int rv;

  assert(file);
  assert(function);

  if (!val)
    cerebro_err_exit("unmarshall_int8(%s(%s:%d)): null val pointer", file, function, line);

  if (!buf)
    cerebro_err_exit("unmarshall_int8(%s(%s:%d)): null buf pointer", file, function, line);

  if ((rv = unmarshall_int8(val, buf, buflen)) < 0)
    cerebro_err_exit("unmarshall_int8(%s(%s:%d)): %s\n", strerror(errno));

  return rv;
}

int 
wrap_unmarshall_int32(const char *file, const char *function, int line, int32_t *val, const char *buf, unsigned int buflen)
{
  int rv;

  assert(file);
  assert(function);

  if (!val)
    cerebro_err_exit("unmarshall_int32(%s(%s:%d)): null val pointer", file, function, line);

  if (!buf)
    cerebro_err_exit("unmarshall_int32(%s(%s:%d)): null buf pointer", file, function, line);

  if ((rv = unmarshall_int32(val, buf, buflen)) < 0)
    cerebro_err_exit("unmarshall_int32(%s(%s:%d)): %s\n", strerror(errno));

  return rv;
}

int 
wrap_unmarshall_u_int8(const char *file, const char *function, int line, u_int8_t *val, const char *buf, unsigned int buflen)
{
  int rv;

  assert(file);
  assert(function);

  if (!val)
    cerebro_err_exit("unmarshall_u_int8(%s(%s:%d)): null val pointer", file, function, line);

  if (!buf)
    cerebro_err_exit("unmarshall_u_int8(%s(%s:%d)): null buf pointer", file, function, line);

  if ((rv = unmarshall_u_int8(val, buf, buflen)) < 0)
    cerebro_err_exit("unmarshall_u_int8(%s(%s:%d)): %s\n", strerror(errno));

  return rv;
}

int 
wrap_unmarshall_u_int32(const char *file, const char *function, int line, u_int32_t *val, const char *buf, unsigned int buflen)
{
  int rv;

  assert(file);
  assert(function);

  if (!val)
    cerebro_err_exit("unmarshall_u_int32(%s(%s:%d)): null val pointer", file, function, line);

  if (!buf)
    cerebro_err_exit("unmarshall_u_int32(%s(%s:%d)): null buf pointer", file, function, line);

  if ((rv = unmarshall_u_int32(val, buf, buflen)) < 0)
    cerebro_err_exit("unmarshall_u_int32(%s(%s:%d)): %s\n", strerror(errno));

  return rv;
}

int
wrap_unmarshall_float(const char *file, const char *function, int line, float *val, const char *buf, unsigned int buflen)
{
  int rv;

  assert(file);
  assert(function);

  if (!val)
    cerebro_err_exit("unmarshall_float(%s(%s:%d)): null val pointer", file, function, line);

  if (!buf)
    cerebro_err_exit("unmarshall_float(%s(%s:%d)): null buf pointer", file, function, line);

  if ((rv = unmarshall_float(val, buf, buflen)) < 0)
    cerebro_err_exit("unmarshall_float(%s(%s:%d)): %s\n", strerror(errno));

  return rv;
}

int 
wrap_unmarshall_double(const char *file, const char *function, int line, double *val, const char *buf, unsigned int buflen)
{
  int rv;

  assert(file);
  assert(function);

  if (!val)
    cerebro_err_exit("unmarshall_double(%s(%s:%d)): null val pointer", file, function, line);

  if (!buf)
    cerebro_err_exit("unmarshall_double(%s(%s:%d)): null buf pointer", file, function, line);

  if ((rv = unmarshall_double(val, buf, buflen)) < 0)
    cerebro_err_exit("unmarshall_double(%s(%s:%d)): %s\n", strerror(errno));

  return rv;
}

int 
wrap_unmarshall_buffer(const char *file, const char *function, int line, char *val, unsigned int vallen, const char *buf, unsigned int buflen)
{
  int rv;

  assert(file);
  assert(function);

  if (!val)
    cerebro_err_exit("unmarshall_buffer(%s(%s:%d)): null val pointer", file, function, line);

  if (!buf)
    cerebro_err_exit("unmarshall_buffer(%s(%s:%d)): null buf pointer", file, function, line);

  if ((rv = unmarshall_buffer(val, vallen, buf, buflen)) < 0)
    cerebro_err_exit("unmarshall_buffer(%s(%s:%d)): %s\n", strerror(errno));

  return rv;
}
