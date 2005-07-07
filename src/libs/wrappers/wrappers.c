/*****************************************************************************\
 *  $Id: wrappers.c,v 1.9 2005-07-07 16:10:15 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <limits.h>
#include <assert.h>
#include <errno.h>

#include "cerebro/cerebro_error.h"

#include "wrappers.h"

#include "fd.h"

extern int h_errno;

#define MALLOC_MAGIC      0xf0e0d0c0
#define MALLOC_PAD_DATA   0xab
#define MALLOC_PAD_LEN    16

#define WRAPPERS_ERR_INVALID_PARAMETERS(func) \
   do { \
     cerebro_err_exit(func "(%s, %s, %d): invalid parameters", \
                      file, function, line); \
   } while(0);

#define WRAPPERS_ERR_ERRNO(func) \
   do { \
     cerebro_err_exit(func "(%s, %s, %d): %s", \
                      file, function, line, strerror(errno)); \
   } while(0);

#define WRAPPERS_ERR_MSG(func, msg) \
   do { \
     cerebro_err_exit(func "(%s, %s, %d): %s", \
                      file, function, line, msg); \
   } while(0);

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

  if ((rv = fd_read_n(fd, buf, count)) < 0)
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

  if ((rv = fd_write_n(fd, buf, count)) < 0)
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
  struct timeval timeout_orig, timeout_current;
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
      timeout_current = *timeout;
      Gettimeofday(&start, NULL);
    }

  do 
    {
      rv = select(n, readfds, writefds, exceptfds, &timeout_current);
      if (rv < 0 && errno != EINTR)
        WRAPPERS_ERR_ERRNO("select");
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

int 
wrap_pthread_create(WRAPPERS_ARGS, pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *), void *arg)
{
  int rv;
  
  assert(file && function);

  if (!thread || !start_routine)
    WRAPPERS_ERR_INVALID_PARAMETERS("pthread_create");

  if ((rv = pthread_create(thread, attr, start_routine, arg)) != 0)
    WRAPPERS_ERR_MSG("pthread_create", strerror(rv));

  return rv;
}

int 
wrap_pthread_attr_init(WRAPPERS_ARGS, pthread_attr_t *attr)
{
  int rv;
  
  assert(file && function);

  if (!attr)
    WRAPPERS_ERR_INVALID_PARAMETERS("pthread_attr_init");

  if ((rv = pthread_attr_init(attr)) != 0)
    WRAPPERS_ERR_MSG("pthread_attr_init", strerror(rv));

  return rv;
}

int 
wrap_pthread_attr_destroy(WRAPPERS_ARGS, pthread_attr_t *attr)
{
  int rv;
  
  assert(file && function);

  if (!attr)
    WRAPPERS_ERR_INVALID_PARAMETERS("pthread_attr_destroy");

  if ((rv = pthread_attr_destroy(attr)) != 0)
    WRAPPERS_ERR_MSG("pthread_attr_destroy", strerror(rv));

  return rv;
}

int 
wrap_pthread_attr_setdetachstate(WRAPPERS_ARGS, pthread_attr_t *attr, int detachstate)
{
  int rv;
  
  assert(file && function);

  if (!attr)
    WRAPPERS_ERR_INVALID_PARAMETERS("pthread_attr_setdetachstate");

  if ((rv = pthread_attr_setdetachstate(attr, detachstate)) != 0)
    WRAPPERS_ERR_MSG("pthread_attr_setdetachstate", strerror(rv));

  return rv;
}

int 
wrap_pthread_attr_setstacksize(WRAPPERS_ARGS, pthread_attr_t *attr, size_t stacksize)
{
  int rv;
  
  assert(file && function);

  if (!attr || stacksize < PTHREAD_STACK_MIN)
    WRAPPERS_ERR_INVALID_PARAMETERS("pthread_attr_setstacksize");

  if ((rv = pthread_attr_setstacksize(attr, stacksize)) != 0)
    WRAPPERS_ERR_MSG("pthread_attr_setstacksize", strerror(rv));

  return rv;
}

int
wrap_pthread_mutex_lock(WRAPPERS_ARGS, pthread_mutex_t *mutex)
{
  int rv;
  
  assert(file && function);

  if (!mutex)
    WRAPPERS_ERR_INVALID_PARAMETERS("pthread_mutex_lock");

  if ((rv = pthread_mutex_lock(mutex)) != 0)
    WRAPPERS_ERR_MSG("pthread_mutex_lock", strerror(rv));

  return rv;
}

int 
wrap_pthread_mutex_trylock(WRAPPERS_ARGS, pthread_mutex_t *mutex)
{
  int rv;
  
  assert(file && function);

  if (!mutex)
    WRAPPERS_ERR_INVALID_PARAMETERS("pthread_mutex_trylock");

  rv = pthread_mutex_trylock(mutex);
  if (rv != 0 && rv != EBUSY)
    WRAPPERS_ERR_MSG("pthread_mutex_trylock", strerror(rv));

  return rv;
}

int 
wrap_pthread_mutex_unlock(WRAPPERS_ARGS, pthread_mutex_t *mutex)
{
  int rv;
  
  assert(file && function);

  if (!mutex)
    WRAPPERS_ERR_INVALID_PARAMETERS("pthread_mutex_unlock");

  if ((rv = pthread_mutex_unlock(mutex)) != 0)
    WRAPPERS_ERR_MSG("pthread_mutex_unlock", strerror(rv));

  return rv;
}

int
wrap_pthread_mutex_init(WRAPPERS_ARGS, pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr)
{
  int rv;
  
  assert(file && function);

  if (!mutex)
    WRAPPERS_ERR_INVALID_PARAMETERS("pthread_mutex_init");

  if ((rv = pthread_mutex_init(mutex, mutexattr)) != 0)
    WRAPPERS_ERR_MSG("pthread_mutex_init", strerror(rv));

  return rv;
}

int 
wrap_pthread_cond_signal(WRAPPERS_ARGS, pthread_cond_t *cond)
{
  int rv;

  assert(file && function);

  if (!cond)
    WRAPPERS_ERR_INVALID_PARAMETERS("pthread_cond_signal");

  if ((rv = pthread_cond_signal(cond)) != 0)
    WRAPPERS_ERR_MSG("pthread_cond_signal", strerror(rv));

  return rv;
}

int 
wrap_pthread_cond_wait(WRAPPERS_ARGS, pthread_cond_t *cond, pthread_mutex_t *mutex)
{
  int rv;

  assert(file && function);

  if (!cond || !mutex)
    WRAPPERS_ERR_INVALID_PARAMETERS("pthread_cond_wait");

  if ((rv = pthread_cond_wait(cond, mutex)) != 0)
    WRAPPERS_ERR_MSG("pthread_cond_signal", strerror(rv));

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

int
wrap_lt_dlinit(WRAPPERS_ARGS)
{
  int rv;

  assert(file && function);

  if ((rv = lt_dlinit()) != 0)
    WRAPPERS_ERR_MSG("lt_dlinit", lt_dlerror());

  return rv;
}

int
wrap_lt_dlexit(WRAPPERS_ARGS)
{
  int rv;

  assert(file && function);

  if ((rv = lt_dlexit()) != 0)
    WRAPPERS_ERR_MSG("lt_dlexit", lt_dlerror());

  return rv;
}

lt_dlhandle
wrap_lt_dlopen(WRAPPERS_ARGS, const char *filename)
{
  lt_dlhandle rv;

  assert(file && function);

  if (!filename)
    WRAPPERS_ERR_INVALID_PARAMETERS("lt_dlopen");

  if (!(rv = lt_dlopen(filename)))
    WRAPPERS_ERR_MSG("lt_dlopen", lt_dlerror());

  return rv;
}

lt_ptr
wrap_lt_dlsym(WRAPPERS_ARGS, void *handle, char *symbol)
{
  lt_ptr *rv;

  assert(file && function);

  if (!handle || !symbol)
    WRAPPERS_ERR_INVALID_PARAMETERS("lt_dlsym");

  /* "clear" lt_dlerror() */
  lt_dlerror();

  if (!(rv = lt_dlsym(handle, symbol)))
    {
      const char *err = lt_dlerror();
      if (err)
        WRAPPERS_ERR_MSG("lt_dlopen", err);
    }

  return rv;
}

int 
wrap_lt_dlclose(WRAPPERS_ARGS, void *handle)
{
  int rv;

  assert(file && function);

  if (!handle)
    WRAPPERS_ERR_INVALID_PARAMETERS("lt_dlclose");

  if ((rv = lt_dlclose(handle)) != 0)
    WRAPPERS_ERR_MSG("lt_dlclose", lt_dlerror());

  return rv;
}

List 
wrap_list_create(WRAPPERS_ARGS, ListDelF f)
{
  List rv;

  assert(file && function);

  if (!(rv = list_create(f)))
    WRAPPERS_ERR_ERRNO("list_create");

  return rv;
}

void
wrap_list_destroy(WRAPPERS_ARGS, List l)
{
  assert(file && function);

  if (!l)
    WRAPPERS_ERR_INVALID_PARAMETERS("list_destroy");

  list_destroy(l);

  return;
}

int
wrap_list_count(WRAPPERS_ARGS, List l)
{
  assert(file && function);

  if (!l)
    WRAPPERS_ERR_INVALID_PARAMETERS("list_count");

  return list_count(l);
}

void *
wrap_list_append (WRAPPERS_ARGS, List l, void *x)
{
  void *rv;

  assert(file && function);

  if (!l || !x)
    WRAPPERS_ERR_INVALID_PARAMETERS("list_append");

  if (!(rv = list_append(l, x)))
    WRAPPERS_ERR_ERRNO("list_append");
  
  return rv;
}

void * 
wrap_list_find_first (WRAPPERS_ARGS, List l, ListFindF f, void *key)
{
  assert(file && function);

  if (!l || !f || !key)
    WRAPPERS_ERR_INVALID_PARAMETERS("list_find_first");

  return list_find_first(l, f, key);
}

int 
wrap_list_delete_all(WRAPPERS_ARGS, List l, ListFindF f, void *key)
{
  int rv;

  assert(file && function);

  if (!l || !f || !key)
    WRAPPERS_ERR_INVALID_PARAMETERS("list_delete_all");

  if ((rv = list_delete_all(l, f, key)) < 0)
    WRAPPERS_ERR_ERRNO("list_delete_all");
  
  return rv;
}

int
wrap_list_for_each(WRAPPERS_ARGS, List l, ListForF f, void *arg)
{
  int rv;

  assert(file && function);

  if (!l || !f)
    WRAPPERS_ERR_INVALID_PARAMETERS("list_for_each");

  if ((rv = list_for_each(l, f, arg)) < 0)
    WRAPPERS_ERR_ERRNO("list_for_each");
  
  return rv;
}

void
wrap_list_sort(WRAPPERS_ARGS, List l, ListCmpF f)
{
  assert(file && function);

  if (!l || !f)
    WRAPPERS_ERR_INVALID_PARAMETERS("list_sort");

  list_sort(l, f);
  return;
}

ListIterator
wrap_list_iterator_create(WRAPPERS_ARGS, List l)
{
  ListIterator rv;

  assert(file && function);

  if (!l)
    WRAPPERS_ERR_INVALID_PARAMETERS("list_iterator_create");

  if (!(rv = list_iterator_create(l)))
    WRAPPERS_ERR_ERRNO("list_iterator_create");

  return rv;
}

void
wrap_list_iterator_destroy(WRAPPERS_ARGS, ListIterator i)
{
  assert(file && function);

  if (!i)
    WRAPPERS_ERR_INVALID_PARAMETERS("list_iterator_destroy");

  list_iterator_destroy(i);
  return;
}

hash_t 
wrap_hash_create(WRAPPERS_ARGS, int size, hash_key_f key_f, hash_cmp_f cmp_f, hash_del_f del_f)
{
  hash_t rv;

  assert(file && function);

  if (!(size > 0 || size <= INT_MAX) || !key_f || !cmp_f)
    WRAPPERS_ERR_INVALID_PARAMETERS("hash_create");

  if (!(rv = hash_create(size, key_f, cmp_f, del_f)))
    WRAPPERS_ERR_ERRNO("hash_create");

  return rv;
}

int 
wrap_hash_count(WRAPPERS_ARGS, hash_t h)
{
  int rv;

  assert(file && function);

  if (!h)
    WRAPPERS_ERR_INVALID_PARAMETERS("hash_count");

  if (!(rv = hash_count(h)))
    {
      if (errno != 0)
        WRAPPERS_ERR_ERRNO("hash_count");
    }

  return rv;
}

void *
wrap_hash_find(WRAPPERS_ARGS, hash_t h, const void *key)
{
  void *rv;

  assert(file && function);

  if (!h || !key)
    WRAPPERS_ERR_INVALID_PARAMETERS("hash_find");

  rv = hash_find(h, key);
  if (!rv && errno != 0)
    WRAPPERS_ERR_ERRNO("hash_find");

  return rv;
}

void *
wrap_hash_insert(WRAPPERS_ARGS, hash_t h, const void *key, void *data)
{
  void *rv;

  assert(file && function);

  if (!h || !key || !data)
    WRAPPERS_ERR_INVALID_PARAMETERS("hash_insert");

  if (!(rv = hash_insert(h, key, data)))
    WRAPPERS_ERR_ERRNO("hash_insert");

  if (rv != data)
    WRAPPERS_ERR_MSG("hash_insert", "invalid insert");

  return rv;
}

void *
wrap_hash_remove(WRAPPERS_ARGS, hash_t h, const void *key)
{
  void *rv;

  assert(file && function);

  if (!h || !key)
    WRAPPERS_ERR_INVALID_PARAMETERS("hash_remove");

  if (!(rv = hash_remove(h, key)))
    WRAPPERS_ERR_ERRNO("hash_remove");

  return rv;
}

int 
wrap_hash_delete_if(WRAPPERS_ARGS, hash_t h, hash_arg_f argf, void *arg)
{
  int rv;

  assert(file && function);
    
  if (!h || !argf)
    WRAPPERS_ERR_INVALID_PARAMETERS("hash_delete_if");

  if ((rv = hash_delete_if(h, argf, arg)) < 0)
    WRAPPERS_ERR_ERRNO("hash_delete_if");

  return rv;
}

int 
wrap_hash_for_each(WRAPPERS_ARGS, hash_t h, hash_arg_f argf, void *arg)
{
  int rv;

  assert(file && function);

  if (!h || !argf)
    WRAPPERS_ERR_INVALID_PARAMETERS("hash_for_each");

  if ((rv = hash_for_each(h, argf, arg)) < 0)
    WRAPPERS_ERR_ERRNO("hash_for_each");

  return rv;
}

void
wrap_hash_destroy(WRAPPERS_ARGS, hash_t h)
{
  assert(file && function);

  if (!h)
    WRAPPERS_ERR_INVALID_PARAMETERS("hash_destroy");

  hash_destroy(h);
  return;
}

hostlist_t 
wrap_hostlist_create(WRAPPERS_ARGS, const char *hostlist)
{
  hostlist_t rv;

  assert(file && function);

  if (!(rv = hostlist_create(hostlist)))
    WRAPPERS_ERR_ERRNO("hostlist_create");

  return rv;
}

void
wrap_hostlist_destroy(WRAPPERS_ARGS, hostlist_t hl)
{
  assert(file && function);

  if (!hl)
    WRAPPERS_ERR_INVALID_PARAMETERS("hostlist_destroy");

  hostlist_destroy(hl);
  return;
}

void 
wrap_hostlist_sort(WRAPPERS_ARGS, hostlist_t hl)
{
  assert(file && function);

  if (!hl)
    WRAPPERS_ERR_INVALID_PARAMETERS("hostlist_sort");

  hostlist_sort(hl);
  return;
}

void 
wrap_hostlist_uniq(WRAPPERS_ARGS, hostlist_t hl)
{
 assert(file && function);

  if (!hl)
    WRAPPERS_ERR_INVALID_PARAMETERS("hostlist_uniq");

  hostlist_uniq(hl);
  return;
}

int 
wrap_hostlist_push(WRAPPERS_ARGS, hostlist_t hl, const char *host)
{
  int rv;

  assert(file && function);

  if (!hl || !host)
    WRAPPERS_ERR_INVALID_PARAMETERS("hostlist_push");

  if (!(rv = hostlist_push(hl, host)))
    WRAPPERS_ERR_ERRNO("hostlist_push");

  return rv;
}

size_t 
wrap_hostlist_ranged_string(WRAPPERS_ARGS, hostlist_t hl, size_t n, char *buf)
{
  size_t rv;

  assert(file && function);

  if (!hl || !buf || !(n > 0 || n <= INT_MAX))
    WRAPPERS_ERR_INVALID_PARAMETERS("hostlist_ranged_string");

  if ((rv = hostlist_ranged_string(hl, n, buf)) < 0)
    WRAPPERS_ERR_ERRNO("hostlist_ranged_string");

  return rv;
}

size_t 
wrap_hostlist_deranged_string(WRAPPERS_ARGS, hostlist_t hl, size_t n, char *buf)
{
  size_t rv;

  assert(file && function);

  if (!hl || !buf || !(n > 0 || n <= INT_MAX))
    WRAPPERS_ERR_INVALID_PARAMETERS("hostlist_deranged_string");

  if ((rv = hostlist_deranged_string(hl, n, buf)) < 0)
    WRAPPERS_ERR_ERRNO("hostlist_deranged_string");

  return rv;
}

int 
wrap_marshall_int8(WRAPPERS_ARGS, int8_t val, char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("marshall_int8");

  if ((rv = marshall_int8(val, buf, buflen)) <= 0)
    WRAPPERS_ERR_ERRNO("marshall_int8");

  return rv;
}

int 
wrap_marshall_int32(WRAPPERS_ARGS, int32_t val, char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("marshall_int32");

  if ((rv = marshall_int32(val, buf, buflen)) <= 0)
    WRAPPERS_ERR_ERRNO("marshall_int32");

  return rv;
}

int 
wrap_marshall_u_int8(WRAPPERS_ARGS, u_int8_t val, char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("marshall_u_int8");

  if ((rv = marshall_u_int8(val, buf, buflen)) <= 0)
    WRAPPERS_ERR_ERRNO("mashall_u_int8");

  return rv;
}

int 
wrap_marshall_u_int32(WRAPPERS_ARGS, u_int32_t val, char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("marshall_u_int32");

  if ((rv = marshall_u_int32(val, buf, buflen)) <= 0)
    WRAPPERS_ERR_ERRNO("marshall_u_int32");

  return rv;
}

int 
wrap_marshall_float(WRAPPERS_ARGS, float val, char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("marshall_float");

  if ((rv = marshall_float(val, buf, buflen)) <= 0)
    WRAPPERS_ERR_ERRNO("marshall_float");

  return rv;
}

int 
wrap_marshall_double(WRAPPERS_ARGS, double val, char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("marshall_double");

  if ((rv = marshall_double(val, buf, buflen)) <= 0)
    WRAPPERS_ERR_ERRNO("marshall_double");

  return rv;
}

int 
wrap_marshall_buffer(WRAPPERS_ARGS, const char *val, unsigned int vallen, char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!val || !buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("marshall_buffer");

  if ((rv = marshall_buffer(val, vallen, buf, buflen)) <= 0)
    WRAPPERS_ERR_ERRNO("marshall_buffer");

  return rv;
}

int 
wrap_unmarshall_int8(WRAPPERS_ARGS, int8_t *val, const char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!val || !buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("unmarshall_int8");

  if ((rv = unmarshall_int8(val, buf, buflen)) < 0)
    WRAPPERS_ERR_ERRNO("unmarshall_int8");

  return rv;
}

int 
wrap_unmarshall_int32(WRAPPERS_ARGS, int32_t *val, const char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!val || !buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("unmarshall_int32");

  if ((rv = unmarshall_int32(val, buf, buflen)) < 0)
    WRAPPERS_ERR_ERRNO("unmarshall_int32");

  return rv;
}

int 
wrap_unmarshall_u_int8(WRAPPERS_ARGS, u_int8_t *val, const char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!val || !buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("unmarshall_u_int8");

  if ((rv = unmarshall_u_int8(val, buf, buflen)) < 0)
    WRAPPERS_ERR_ERRNO("unmarshall_u_int8");

  return rv;
}

int 
wrap_unmarshall_u_int32(WRAPPERS_ARGS, u_int32_t *val, const char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!val || !buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("unmarshall_u_int32");

  if ((rv = unmarshall_u_int32(val, buf, buflen)) < 0)
    WRAPPERS_ERR_ERRNO("unmarshall_u_int32");

  return rv;
}

int
wrap_unmarshall_float(WRAPPERS_ARGS, float *val, const char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!val || !buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("unmarshall_float");

  if ((rv = unmarshall_float(val, buf, buflen)) < 0)
    WRAPPERS_ERR_ERRNO("unmarshall_float");

  return rv;
}

int 
wrap_unmarshall_double(WRAPPERS_ARGS, double *val, const char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!val || !buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("unmarshall_double");

  if ((rv = unmarshall_double(val, buf, buflen)) < 0)
    WRAPPERS_ERR_ERRNO("unmarshall_double");

  return rv;
}

int 
wrap_unmarshall_buffer(WRAPPERS_ARGS, char *val, unsigned int vallen, const char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!val || !buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("unmarshall_buffer");

  if ((rv = unmarshall_buffer(val, vallen, buf, buflen)) < 0)
    WRAPPERS_ERR_ERRNO("unmarshall_float");

  return rv;
}
