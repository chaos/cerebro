/*****************************************************************************\
 *  $Id: wrappers.h,v 1.1.1.1 2004-07-02 22:31:29 achu Exp $
\*****************************************************************************/

#ifndef _WRAPPERS_H
#define _WRAPPERS_H

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

#define Malloc(size)                wrap_malloc(__FILE__, __LINE__, size)
#define Free(ptr)                   wrap_free(__FILE__, __LINE__, ptr)
#define Strdup(s)                   wrap_strdup(__FILE__, __LINE__, s)
#define Open(pathname, flags, mode) wrap_open(__FILE__, __LINE__, pathname, flags, mode)
#define Close(fd)                   wrap_close(__FILE__, __LINE__, fd)
#define Read(fd, buf, count)        wrap_read(__FILE__, __LINE__, fd, buf, count) 
#define Write(fd, buf, count)       wrap_write(__FILE__, __LINE__, fd, buf, count) 

void * wrap_malloc(char *file, int line, size_t size);
void wrap_free(char *file, int line, void *ptr);
char * wrap_strdup(char *file, int line, const char *s);
int wrap_open(char *file, int line, const char *pathname, int flags, int mode);
int wrap_close(char *file, int line, int fd);
ssize_t wrap_read(char *file, int line, int fd, void *buf, size_t count);
ssize_t wrap_write(char *file, int line, int fd, const void *buf, size_t count);

#endif /* _WRAPPERS_H */
