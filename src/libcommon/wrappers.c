/*****************************************************************************\
 *  $Id: wrappers.c,v 1.2 2004-07-03 00:34:15 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif

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
wrap_malloc(char *file, int line, size_t size)
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
wrap_free(char *file, int line, void *ptr)
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
wrap_strdup(char *file, int line, const char *s)
{
  char *ptr;

  assert(file != NULL);
  assert(s != NULL);

  ptr = wrap_malloc(file, line, strlen(s) + 1);
  strcpy(ptr, s);
  return ptr;
}

int 
wrap_open(char *file, int line, const char *pathname, int flags, int mode)
{
  int fd;

  assert(file != NULL);

  if ((fd = open(pathname, flags, mode)) < 0)
    err_exit("open(%s:%d): %s", file, line, strerror(errno));
  return fd;
}

int 
wrap_close(char *file, int line, int fd)
{
  int ret;
                                 
  assert(file != NULL);
                                                   
  if ((ret = close(fd)) < 0)
    err_exit("close(%s:%d): %s", file, line, strerror(errno));
  return ret;
}

ssize_t
wrap_read(char *file, int line, int fd, void *buf, size_t count)
{
  ssize_t ret;

  assert(file != NULL);

  if ((ret = fd_read_n(fd, buf, count)) < 0)
    err_exit("read(%s:%d): %s", file, line, strerror(errno));
    
  return ret;
}

ssize_t
wrap_write(char *file, int line, int fd, const void *buf, size_t count)
{
  ssize_t ret;

  assert(file != NULL);

  if ((ret = fd_write_n(fd, buf, count)) < 0)
    err_exit("write(%s:%d): %s", file, line, strerror(errno));

  return ret;
}

int
wrap_socket(char *file, int line, int domain, int type, int protocol)
{
  int fd;

  assert(file != NULL);

  if ((fd = socket(domain, type, protocol)) < 0)
    err_exit("socket(%s:%d): %s", file, line, strerror(errno));

  return fd;
}

int
wrap_getsockopt(char *file, int line, int s, int level, int optname, void *optval, socklen_t *optlen)
{
  int ret;

  assert(file != NULL);

  if ((ret = getsockopt(s, level, optname, optval, optlen)) < 0)
    err_exit("socket(%s:%d): %s", file, line, strerror(errno));

  return ret;
}

int
wrap_setsockopt(char *file, int line, int s, int level, int optname, const void *optval, socklen_t optlen)
{
  int ret;

  assert(file != NULL);

  if ((ret = setsockopt(s, level, optname, optval, optlen)) < 0)
    err_exit("socket(%s:%d): %s", file, line, strerror(errno));

  return ret;
}
