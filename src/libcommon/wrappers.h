/*****************************************************************************\
 *  $Id: wrappers.h,v 1.3 2004-07-06 17:06:26 achu Exp $
\*****************************************************************************/

#ifndef _WRAPPERS_H
#define _WRAPPERS_H

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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif /* _GNU_SOURCE */
#include <signal.h>

#define Malloc(size) \
        wrap_malloc(__FILE__, __LINE__, size)
#define Free(ptr) \
        wrap_free(__FILE__, __LINE__, ptr)
#define Strdup(s) \
        wrap_strdup(__FILE__, __LINE__, s)
#define Strncpy(dest, src, n) \
        wrap_strncpy(__FILE__, __LINE__, dest, src, n)
#define Open(pathname, flags, mode) \
        wrap_open(__FILE__, __LINE__, pathname, flags, mode)
#define Close(fd) \
        wrap_close(__FILE__, __LINE__, fd)
#define Read(fd, buf, count) \
        wrap_read(__FILE__, __LINE__, fd, buf, count) 
#define Write(fd, buf, count) \
        wrap_write(__FILE__, __LINE__, fd, buf, count) 
#define Socket(domain, type, protocol) \
        wrap_socket(__FILE__, __LINE__, domain, type, protocol)
#define Getsockopt(s, level, optname, optval, optlen) \
        wrap_getsockopt(__FILE__, __LINE__, s, level, optname, optval, optlen)
#define Setsockopt(s, level, optname, optval, optlen) \
        wrap_setsockopt(__FILE__, __LINE__, s, level, optname, optval, optlen)
#define Fork() \
        wrap_fork(__FILE__, __LINE__);
#define Signal(signum, handler) \
        wrap_signal(__FILE__, __LINE__, signum, handler)
#define Chdir(path) \
        wrap_chdir(__FILE__, __LINE__, path)
#define Umask(mask) \
        wrap_umask(__FILE__, __LINE__, mask)

typedef void (*Sighandler_t)(int);

void * wrap_malloc(const char *file, int line, size_t size);
void wrap_free(const char *file, int line, void *ptr);
char * wrap_strdup(const char *file, int line, const char *s);
char * wrap_strncpy(const char *file, int line, char *dest, const char *src, size_t n);
int wrap_open(const char *file, int line, const char *pathname, int flags, int mode);
int wrap_close(const char *file, int line, int fd);
ssize_t wrap_read(const char *file, int line, int fd, void *buf, size_t count);
ssize_t wrap_write(const char *file, int line, int fd, const void *buf, size_t count);
int wrap_socket(const char *file, int line, int domain, int type, int protocol);
int wrap_getsockopt(const char *file, int line, int s, int level, int optname, void *optval, socklen_t *optlen);
int wrap_setsockopt(const char *file, int line, int s, int level, int optname, const void *optval, socklen_t optlen);
pid_t wrap_fork(const char *file, int line);
Sighandler_t wrap_signal(const char *file, int line, int signum, Sighandler_t handler);
int wrap_chdir(const char *file, int line, const char *path);
mode_t wrap_umask(const char *file, int line, mode_t mask);

#endif /* _WRAPPERS_H */
