/*****************************************************************************\
 *  $Id: wrappers.c,v 1.12 2005-03-14 17:05:14 achu Exp $
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
    {
      if (pathname)
	err_exit("open(%s:%d): pathname=%s: %s", file, line, pathname, strerror(errno));
      else
	err_exit("open(%s:%d): %s", file, line, strerror(errno));
    }
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
    {
      if (path)
	err_exit("chdir(%s:%d): path=%s: %s", file, line, path, strerror(errno));
      else
	err_exit("chdir(%s:%d): %s", file, line, strerror(errno));
    }

  return ret;
}

int 
wrap_stat(const char *file, int line, const char *path, struct stat *buf)
{
  int ret;

  assert(file != NULL);

  if ((ret = stat(path, buf)) < 0)
    err_exit("stat(%s:%d): %s", file, line, strerror(errno));

  return ret;
}

mode_t
wrap_umask(const char *file, int line, mode_t mask)
{
  assert(file != NULL);

  /* achu: never supposed to fail.  Go fig. */
  return umask(mask);
}

DIR *
wrap_opendir(const char *file, int line, const char *name)
{
  DIR *ret;
  
  assert(file != NULL);

  if (!(ret = opendir(name)))
    {
      if (name)
	err_exit("opendir(%s:%d): name=%s: %s", file, line, name, strerror(errno));
      else
	err_exit("opendir(%s:%d): %s", file, line, strerror(errno));
    }

  return ret;
}

int
wrap_closedir(const char *file, int line, DIR *dir)
{
  int ret;

  assert(file != NULL);

  if ((ret = closedir(dir)) < 0)
    err_exit("closedir(%s:%d): %s", file, line, strerror(errno));

  return ret;
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

int
wrap_pthread_mutex_init(const char *file, int line, pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr)
{
  int ret;
  
  assert(file != NULL);

  if ((ret = pthread_mutex_init(mutex, mutexattr)) != 0)
    err_exit("pthread_mutex_init(%s:%d): %s", file, line, strerror(ret));

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

int 
wrap_gethostname(const char *file, int line, char *name, size_t len)
{
  int ret;

  assert(file != NULL);

  if ((ret = gethostname(name, len)) < 0)
    err_exit("gethostname(%s:%d): %s", file, line, strerror(errno));

  return ret;
}

lt_dlhandle
wrap_lt_dlopen(const char *file, int line, const char *filename)
{
  lt_dlhandle ret;

  assert(file != NULL);

  if (!(ret = lt_dlopen(filename)))
    {
      if (filename)
	err_exit("lt_dlopen(%s:%d): filename=%s: %s", file, line, filename, lt_dlerror());
      else
	err_exit("lt_dlopen(%s:%d): %s", file, line, lt_dlerror());
    }

  return ret;
}

lt_ptr
wrap_lt_dlsym(const char *file, int line, void *handle, char *symbol)
{
  lt_ptr *ret;
  const char *err;

  assert(file != NULL);

  /* "clear" lt_dlerror() */
  lt_dlerror();

  if (!(ret = lt_dlsym(handle, symbol)))
    {
      err = lt_dlerror();
      if (err != NULL)
	{
	  if (symbol)
	    err_exit("lt_dlsym(%s:%d): symbol=%s: %s", file, line, symbol, err);
	  else
	    err_exit("lt_dlsym(%s:%d): %s", file, line, err);
	}
    }

  return ret;
}

int 
wrap_lt_dlclose(const char *file, int line, void *handle)
{
  int ret;

  assert(file != NULL);

  if ((ret = lt_dlclose(handle)) != 0)
    err_exit("lt_dlclose(%s:%d): %s", lt_dlerror());

  return ret;
}

List 
wrap_list_create(const char *file, int line, ListDelF f)
{
  List ret;

  assert(file != NULL);

  if (!(ret = list_create(f)))
    err_exit("list_create(%s:%d): %s", file, line, strerror(errno));

  return ret;
}

void *
wrap_list_append (const char *file, int line, List l, void *x)
{
  void *ret;

  assert(file != NULL);

  if (!(ret = list_append(l, x)))
    err_exit("list_append(%s:%d): %s", file, line, strerror(errno));
  
  return ret;
}

void
wrap_list_destroy(const char *file, int line, List l)
{
  assert(file != NULL);

  if (!l)
    err_exit("list_destroy(%s:%d): NULL list pointer", file, line);

  list_destroy(l);

  return;
}

ListIterator
wrap_list_iterator_create(const char *file, int line, List l)
{
  ListIterator ret;

  assert(file != NULL);

  if (!(ret = list_iterator_create(l)))
    err_exit("list_iterator_create(%s:%d): %s", file, line, strerror(errno));

  return ret;
}

void
wrap_list_iterator_destroy(const char *file, int line, ListIterator i)
{
  assert(file != NULL);

  if (!i)
    err_exit("list_iterator_destroy(%s:%d): NULL list iterator pointer", file, line);

  list_iterator_destroy(i);

  return;
}

hash_t 
wrap_hash_create(const char *file, int line, int size, hash_key_f key_f, hash_cmp_f cmp_f, hash_del_f del_f)
{
  hash_t ret;

  assert(file != NULL);

  if (!(ret = hash_create(size, key_f, cmp_f, del_f)))
    err_exit("hash_create(%s:%d): %s", file, line, strerror(errno));

  return ret;
}

int 
wrap_hash_count(const char *file, int line, hash_t h)
{
  int ret;

  assert(file != NULL);

  if (!(ret = hash_count(h)))
    {
      if (errno != 0)
        err_exit("hash_count(%s:%d): %s", file, line, strerror(errno));
    }

  return ret;
}

void *
wrap_hash_find(const char *file, int line, hash_t h, const void *key)
{
  void *ret;

  assert(file != NULL);

  ret = hash_find(h, key);
  if (!ret && errno != 0)
    err_exit("hash_find(%s:%d): %s", file, line, strerror(errno));

  return ret;
}

void *
wrap_hash_insert(const char *file, int line, hash_t h, const void *key, void *data)
{
  void *ret;

  assert(file != NULL);

  if (!(ret = hash_insert(h, key, data)))
    err_exit("hash_insert(%s:%d): %s", file, line, strerror(errno));
  if (ret != data)
    err_exit("hash_insert(%s:%d): invalid insert", file, line);

  return ret;
}

void *
wrap_hash_remove (const char *file, int line, hash_t h, const void *key)
{
  void *ret;

  assert(file != NULL);

  if (!(ret = hash_remove(h, key)))
    err_exit("hash_remove(%s:%d): %s", file, line, strerror(errno));

  return ret;
}

int 
wrap_hash_delete_if(const char *file, int line, hash_t h, hash_arg_f argf, void *arg)
{
  int ret;

  assert(file != NULL);
  
  if ((ret = hash_delete_if(h, argf, arg)) < 0)
    err_exit("hash_delete_if(%s:%d): %s", file, line, strerror(errno));

  return ret;
}

int 
wrap_hash_for_each(const char *file, int line, hash_t h, hash_arg_f argf, void *arg)
{
  int ret;

  assert(file != NULL);

  if ((ret = hash_for_each(h, argf, arg)) < 0)
    err_exit("hash_for_each(%s:%d): %s", file, line, strerror(errno));

  return ret;
}

void
wrap_hash_destroy(const char *file, int line, hash_t h)
{
  assert(file != NULL);

  if (!h)
    err_exit("hash_destroy(%s:%d): NULL hash pointer", file, line);

  hash_destroy(h);

  return;
}
