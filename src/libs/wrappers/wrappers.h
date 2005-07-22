/*****************************************************************************\
 *  $Id: wrappers.h,v 1.9 2005-07-22 17:21:07 achu Exp $
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
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/poll.h>

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else  /* !TIME_WITH_SYS_TIME */
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else /* !HAVE_SYS_TIME_H */
#  include <time.h>
# endif /* !HAVE_SYS_TIME_H */
#endif /* !TIME_WITH_SYS_TIME */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif /* _GNU_SOURCE */
#if HAVE_PTHREAD_H
#include <pthread.h>
#endif /* HAVE_PTHREAD_H */
#include <signal.h>
#include <dirent.h>

#include "error.h"
#include "hash.h"
#include "hostlist.h"
#include "list.h"
#include "ltdl.h"
#include "marshall.h"

#define WRAPPERS_DEBUG_ARGS \
        __FILE__, __FUNCTION__, __LINE__

#define WRAPPERS_ARGS \
        const char *file, const char *function, unsigned int line

/* 
 * Memory/String Wrappers 
 */
#define Malloc(size) \
        wrap_malloc(WRAPPERS_DEBUG_ARGS, size)
#define Free(ptr) \
        wrap_free(WRAPPERS_DEBUG_ARGS, ptr)
#define Strdup(s) \
        wrap_strdup(WRAPPERS_DEBUG_ARGS, s)
#define Strncpy(dest, src, n) \
        wrap_strncpy(WRAPPERS_DEBUG_ARGS, dest, src, n)

void * wrap_malloc(WRAPPERS_ARGS, size_t size);
void wrap_free(WRAPPERS_ARGS, void *ptr);
char * wrap_strdup(WRAPPERS_ARGS, const char *s);
char * wrap_strncpy(WRAPPERS_ARGS, char *dest, const char *src, size_t n);

/* Special wrapper for List/Hash libraries */
void _Free(void *ptr);

/* 
 * File System Wrappers 
 */
#define Open(pathname, flags, mode) \
        wrap_open(WRAPPERS_DEBUG_ARGS, pathname, flags, mode)
#define Close(fd) \
        wrap_close(WRAPPERS_DEBUG_ARGS, fd)
#define Read(fd, buf, count) \
        wrap_read(WRAPPERS_DEBUG_ARGS, fd, buf, count) 
#define Write(fd, buf, count) \
        wrap_write(WRAPPERS_DEBUG_ARGS, fd, buf, count) 
#define Chdir(path) \
        wrap_chdir(WRAPPERS_DEBUG_ARGS, path)
#define Stat(path, buf) \
        wrap_stat(WRAPPERS_DEBUG_ARGS, path, buf)
#define Umask(mask) \
        wrap_umask(WRAPPERS_DEBUG_ARGS, mask)
#define Opendir(name) \
        wrap_opendir(WRAPPERS_DEBUG_ARGS, name)
#define Closedir(dir) \
        wrap_closedir(WRAPPERS_DEBUG_ARGS, dir)

int wrap_open(WRAPPERS_ARGS, const char *pathname, int flags, int mode);
int wrap_close(WRAPPERS_ARGS, int fd);
ssize_t wrap_read(WRAPPERS_ARGS, int fd, void *buf, size_t count);
ssize_t wrap_write(WRAPPERS_ARGS, int fd, const void *buf, size_t count);
int wrap_chdir(WRAPPERS_ARGS, const char *path);
int wrap_stat(WRAPPERS_ARGS, const char *path, struct stat *buf);
mode_t wrap_umask(WRAPPERS_ARGS, mode_t mask);
DIR *wrap_opendir(WRAPPERS_ARGS, const char *name);
int wrap_closedir(WRAPPERS_ARGS, DIR *dir);

/* 
 * Networking Wrappers 
 */
#define Socket(domain, type, protocol) \
        wrap_socket(WRAPPERS_DEBUG_ARGS, domain, type, protocol)
#define Bind(sockfd, my_addr, addrlen) \
        wrap_bind(WRAPPERS_DEBUG_ARGS, sockfd, my_addr, addrlen)
#define Connect(sockfd, serv_addr, addrlen) \
        wrap_connect(WRAPPERS_DEBUG_ARGS, sockfd, serv_addr, addrlen)
#define Listen(s, backlog) \
        wrap_listen(WRAPPERS_DEBUG_ARGS, s, backlog)
#define Accept(s, addr, addrlen) \
        wrap_accept(WRAPPERS_DEBUG_ARGS, s, addr, addrlen)
#define Select(n, readfds, writefds, exceptfds, timeout) \
        wrap_select(WRAPPERS_DEBUG_ARGS, n, readfds, writefds, exceptfds, timeout)
#define Poll(ufds, nfds, timeout) \
        wrap_poll(WRAPPERS_DEBUG_ARGS, ufds, nfds, timeout)
#define Getsockopt(s, level, optname, optval, optlen) \
        wrap_getsockopt(WRAPPERS_DEBUG_ARGS, s, level, optname, optval, optlen)
#define Setsockopt(s, level, optname, optval, optlen) \
        wrap_setsockopt(WRAPPERS_DEBUG_ARGS, s, level, optname, optval, optlen)
#define Gethostbyname(name) \
        wrap_gethostbyname(WRAPPERS_DEBUG_ARGS, name)
#define Inet_ntop(af, src, dst, cnt) \
        wrap_inet_ntop(WRAPPERS_DEBUG_ARGS, af, src, dst, cnt)
#define Inet_pton(af, src, dst) \
        wrap_inet_pton(WRAPPERS_DEBUG_ARGS, af, src, dst)

int wrap_socket(WRAPPERS_ARGS, int domain, int type, int protocol);
int wrap_bind(WRAPPERS_ARGS, int sockfd, struct sockaddr *my_addr, socklen_t addrlen);
int wrap_connect(WRAPPERS_ARGS, int sockfd, struct sockaddr *serv_addr, socklen_t addrlen);
int wrap_listen(WRAPPERS_ARGS, int s, int backlog);
int wrap_accept(WRAPPERS_ARGS, int s, struct sockaddr *addr, socklen_t *addrlen);
int wrap_select(WRAPPERS_ARGS, int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
int wrap_poll(WRAPPERS_ARGS, struct pollfd *ufds, unsigned int nfds, int timeout);
int wrap_getsockopt(WRAPPERS_ARGS, int s, int level, int optname, void *optval, socklen_t *optlen);
int wrap_setsockopt(WRAPPERS_ARGS, int s, int level, int optname, const void *optval, socklen_t optlen);
struct hostent *wrap_gethostbyname(WRAPPERS_ARGS, const char *name);
const char *wrap_inet_ntop(WRAPPERS_ARGS, int af, const void *src, char *dst, socklen_t cnt);
int wrap_inet_pton(WRAPPERS_ARGS, int af, const char *src, void *dst);

/* 
 * Time Wrappers 
 */
#define Gettimeofday(tv, tz) \
        wrap_gettimeofday(WRAPPERS_DEBUG_ARGS, tv, tz)

int wrap_gettimeofday(WRAPPERS_ARGS, struct timeval *tv, struct timezone *tz);

/* 
 * Pthread Wrappers 
 */
#define Pthread_create(thread, attr, start_routine, arg) \
        wrap_pthread_create(WRAPPERS_DEBUG_ARGS, thread, attr, start_routine, arg)
#define Pthread_attr_init(attr) \
        wrap_pthread_attr_init(WRAPPERS_DEBUG_ARGS, attr)
#define Pthread_attr_destroy(attr) \
        wrap_pthread_attr_destroy(WRAPPERS_DEBUG_ARGS, attr)
#define Pthread_attr_setdetachstate(attr, detachstate) \
        wrap_pthread_attr_setdetachstate(WRAPPERS_DEBUG_ARGS, attr, detachstate)
#define Pthread_attr_setstacksize(attr, stacksize) \
        wrap_pthread_attr_setstacksize(WRAPPERS_DEBUG_ARGS, attr, stacksize)
#define Pthread_mutex_lock(mutex) \
        wrap_pthread_mutex_lock(WRAPPERS_DEBUG_ARGS, mutex)
#define Pthread_mutex_trylock(mutex) \
        wrap_pthread_mutex_trylock(WRAPPERS_DEBUG_ARGS, mutex)
#define Pthread_mutex_unlock(mutex) \
        wrap_pthread_mutex_unlock(WRAPPERS_DEBUG_ARGS, mutex)
#define Pthread_mutex_init(mutex, mutexattr) \
        wrap_pthread_mutex_init(WRAPPERS_DEBUG_ARGS, mutex, mutexattr)
#define Pthread_cond_signal(cond) \
        wrap_pthread_cond_signal(WRAPPERS_DEBUG_ARGS, cond)
#define Pthread_cond_wait(cond, mutex) \
        wrap_pthread_cond_wait(WRAPPERS_DEBUG_ARGS, cond, mutex)

int wrap_pthread_create(WRAPPERS_ARGS, pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);
int wrap_pthread_attr_init(WRAPPERS_ARGS, pthread_attr_t *attr);
int wrap_pthread_attr_destroy(WRAPPERS_ARGS, pthread_attr_t *attr);
int wrap_pthread_attr_setdetachstate(WRAPPERS_ARGS, pthread_attr_t *attr, int detachstate);
int wrap_pthread_attr_setstacksize(WRAPPERS_ARGS, pthread_attr_t *attr, size_t stacksize);
int wrap_pthread_mutex_lock(WRAPPERS_ARGS, pthread_mutex_t *mutex);
int wrap_pthread_mutex_trylock(WRAPPERS_ARGS, pthread_mutex_t *mutex);
int wrap_pthread_mutex_unlock(WRAPPERS_ARGS, pthread_mutex_t *mutex);
int wrap_pthread_mutex_init(WRAPPERS_ARGS, pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);
int wrap_pthread_cond_signal(WRAPPERS_ARGS, pthread_cond_t *cond);
int wrap_pthread_cond_wait(WRAPPERS_ARGS, pthread_cond_t *cond, pthread_mutex_t *mutex);

/* 
 * Misc System Call Wrappers 
 */
#define Fork() \
        wrap_fork(WRAPPERS_DEBUG_ARGS);
#define Signal(signum, handler) \
        wrap_signal(WRAPPERS_DEBUG_ARGS, signum, handler)
#define Gethostname(name, len) \
        wrap_gethostname(WRAPPERS_DEBUG_ARGS, name, len)

typedef void (*Sighandler_t)(int);

pid_t wrap_fork(WRAPPERS_ARGS);
Sighandler_t wrap_signal(WRAPPERS_ARGS, int signum, Sighandler_t handler);
int wrap_gethostname(WRAPPERS_ARGS, char *name, size_t len);

/*
 * ltdl wrappers
 */

#define Lt_dlinit() \
        wrap_lt_dlinit(WRAPPERS_DEBUG_ARGS)
#define Lt_dlexit() \
        wrap_lt_dlexit(WRAPPERS_DEBUG_ARGS)
#define Lt_dlopen(filename) \
        wrap_lt_dlopen(WRAPPERS_DEBUG_ARGS, filename)
#define Lt_dlsym(handle, symbol) \
        wrap_lt_dlsym(WRAPPERS_DEBUG_ARGS, handle, symbol)
#define Lt_dlclose(handle) \
        wrap_lt_dlclose(WRAPPERS_DEBUG_ARGS, handle)

int wrap_lt_dlinit(WRAPPERS_ARGS);
int wrap_lt_dlexit(WRAPPERS_ARGS);
lt_dlhandle wrap_lt_dlopen(WRAPPERS_ARGS, const char *filename);
lt_ptr wrap_lt_dlsym(WRAPPERS_ARGS, void *handle, char *symbol);
int wrap_lt_dlclose(WRAPPERS_ARGS, void *handle);

/* 
 * List lib wrappers 
 */
#define List_create(f) \
        wrap_list_create(WRAPPERS_DEBUG_ARGS, f)
#define List_destroy(l) \
        wrap_list_destroy(WRAPPERS_DEBUG_ARGS, l)
#define List_count(l) \
        wrap_list_count(WRAPPERS_DEBUG_ARGS, l)
#define List_append(l, x) \
        wrap_list_append(WRAPPERS_DEBUG_ARGS, l, x)
#define List_find_first(l, f, key) \
        wrap_list_find_first(WRAPPERS_DEBUG_ARGS, l, f, key)
#define List_delete_all(l, f, key) \
        wrap_list_delete_all(WRAPPERS_DEBUG_ARGS, l, f, key)
#define List_for_each(l, f, arg) \
        wrap_list_for_each(WRAPPERS_DEBUG_ARGS, l, f, arg)
#define List_sort(l, f) \
        wrap_list_sort(WRAPPERS_DEBUG_ARGS, l, f)
#define List_iterator_create(l) \
        wrap_list_iterator_create(WRAPPERS_DEBUG_ARGS, l)
#define List_iterator_destroy(i) \
        wrap_list_iterator_destroy(WRAPPERS_DEBUG_ARGS, i)
#define List_delete(i) \
        wrap_list_delete(WRAPPERS_DEBUG_ARGS, i)

List wrap_list_create(WRAPPERS_ARGS, ListDelF f);
void wrap_list_destroy(WRAPPERS_ARGS, List l);
int wrap_list_count(WRAPPERS_ARGS, List l);
void *wrap_list_append (WRAPPERS_ARGS, List l, void *x);
void * wrap_list_find_first (WRAPPERS_ARGS, List l, ListFindF f, void *key);
int wrap_list_delete_all(WRAPPERS_ARGS, List l, ListFindF f, void *key);
int wrap_list_for_each(WRAPPERS_ARGS, List l, ListForF f, void *arg);
void wrap_list_sort(WRAPPERS_ARGS, List l, ListCmpF f);
ListIterator wrap_list_iterator_create(WRAPPERS_ARGS, List l);
void wrap_list_iterator_destroy(WRAPPERS_ARGS, ListIterator i);
int wrap_list_delete(WRAPPERS_ARGS, ListIterator i);

/* 
 * Hash lib wrappers 
 */
#define Hash_create(size, key_f, cmp_f, del_f) \
        wrap_hash_create(WRAPPERS_DEBUG_ARGS, size, key_f, cmp_f, del_f)
#define Hash_count(h) \
        wrap_hash_count(WRAPPERS_DEBUG_ARGS, h)
#define Hash_find(h, key) \
        wrap_hash_find(WRAPPERS_DEBUG_ARGS, h, key)
#define Hash_insert(h, key, data) \
        wrap_hash_insert(WRAPPERS_DEBUG_ARGS, h, key, data)
#define Hash_remove(h, key) \
        wrap_hash_remove(WRAPPERS_DEBUG_ARGS, h, key)
#define Hash_delete_if(h, argf, arg) \
        wrap_hash_delete_if(WRAPPERS_DEBUG_ARGS, h, argf, arg)
#define Hash_for_each(h, argf, arg) \
        wrap_hash_for_each(WRAPPERS_DEBUG_ARGS, h, argf, arg)
#define Hash_destroy(h) \
        wrap_hash_destroy(WRAPPERS_DEBUG_ARGS, h)

hash_t wrap_hash_create(WRAPPERS_ARGS, int size, hash_key_f key_f, hash_cmp_f cmp_f, hash_del_f del_f);
int wrap_hash_count(WRAPPERS_ARGS, hash_t h);
void *wrap_hash_find(WRAPPERS_ARGS, hash_t h, const void *key);
void *wrap_hash_insert(WRAPPERS_ARGS, hash_t h, const void *key, void *data);
void *wrap_hash_remove (WRAPPERS_ARGS, hash_t h, const void *key);
int wrap_hash_delete_if(WRAPPERS_ARGS, hash_t h, hash_arg_f argf, void *arg);
int wrap_hash_for_each(WRAPPERS_ARGS, hash_t h, hash_arg_f argf, void *arg);
void wrap_hash_destroy(WRAPPERS_ARGS, hash_t h);

/* 
 * Hostlist lib wrappers 
 */
#define Hostlist_create(hostlist) \
        wrap_hostlist_create(WRAPPERS_DEBUG_ARGS, hostlist)
#define Hostlist_destroy(hl) \
        wrap_hostlist_destroy(WRAPPERS_DEBUG_ARGS, hl)
#define Hostlist_sort(hl) \
        wrap_hostlist_sort(WRAPPERS_DEBUG_ARGS, hl)
#define Hostlist_uniq(hl) \
        wrap_hostlist_uniq(WRAPPERS_DEBUG_ARGS, hl)
#define Hostlist_push(hl, hosts) \
        wrap_hostlist_push(WRAPPERS_DEBUG_ARGS, hl, hosts)
#define Hostlist_ranged_string(hl, n, buf) \
        wrap_hostlist_ranged_string(WRAPPERS_DEBUG_ARGS, hl, n, buf)
#define Hostlist_deranged_string(hl, n, buf) \
        wrap_hostlist_deranged_string(WRAPPERS_DEBUG_ARGS, hl, n, buf)

hostlist_t wrap_hostlist_create(WRAPPERS_ARGS, const char *hostlist);
void wrap_hostlist_destroy(WRAPPERS_ARGS, hostlist_t hl);
void wrap_hostlist_sort(WRAPPERS_ARGS, hostlist_t hl);
void wrap_hostlist_uniq(WRAPPERS_ARGS, hostlist_t hl);
int wrap_hostlist_push(WRAPPERS_ARGS, hostlist_t hl, const char *host);
size_t wrap_hostlist_ranged_string(WRAPPERS_ARGS, hostlist_t hl, size_t n, char *buf);
size_t wrap_hostlist_deranged_string(WRAPPERS_ARGS, hostlist_t hl, size_t n, char *buf);

/* 
 * Marshall wrappers
 */

#define Marshall_int8(val, buf, buflen) \
        wrap_marshall_int8(WRAPPERS_DEBUG_ARGS, val, buf, buflen)
#define Marshall_int32(val, buf, buflen) \
        wrap_marshall_int32(WRAPPERS_DEBUG_ARGS, val, buf, buflen)
#define Marshall_u_int8(val, buf, buflen) \
        wrap_marshall_u_int8(WRAPPERS_DEBUG_ARGS, val, buf, buflen)
#define Marshall_u_int32(val, buf, buflen) \
        wrap_marshall_u_int32(WRAPPERS_DEBUG_ARGS, val, buf, buflen)
#define Marshall_float(val, buf, buflen) \
        wrap_marshall_float(WRAPPERS_DEBUG_ARGS, val, buf, buflen)
#define Marshall_double(val, buf, buflen) \
        wrap_marshall_double(WRAPPERS_DEBUG_ARGS, val, buf, buflen)
#define Marshall_buffer(val, vallen, buf, buflen) \
        wrap_marshall_buffer(WRAPPERS_DEBUG_ARGS, val, vallen, buf, buflen)
#define Unmarshall_int8(val, buf, buflen) \
        wrap_unmarshall_int8(WRAPPERS_DEBUG_ARGS, val, buf, buflen)
#define Unmarshall_int32(val, buf, buflen) \
        wrap_unmarshall_int32(WRAPPERS_DEBUG_ARGS, val, buf, buflen)
#define Unmarshall_u_int8(val, buf, buflen) \
        wrap_unmarshall_u_int8(WRAPPERS_DEBUG_ARGS, val, buf, buflen)
#define Unmarshall_u_int32(val, buf, buflen) \
        wrap_unmarshall_u_int32(WRAPPERS_DEBUG_ARGS, val, buf, buflen)
#define Unmarshall_float(val, buf, buflen) \
        wrap_unmarshall_float(WRAPPERS_DEBUG_ARGS, val, buf, buflen)
#define Unmarshall_double(val, buf, buflen) \
        wrap_unmarshall_double(WRAPPERS_DEBUG_ARGS, val, buf, buflen)
#define Unmarshall_buffer(val, vallen, buf, buflen) \
        wrap_unmarshall_buffer(WRAPPERS_DEBUG_ARGS, val, vallen, buf, buflen)

int wrap_marshall_int8(WRAPPERS_ARGS, int8_t val, char *buf, unsigned int buflen);
int wrap_marshall_int32(WRAPPERS_ARGS, int32_t val, char *buf, unsigned int buflen);
int wrap_marshall_u_int8(WRAPPERS_ARGS, u_int8_t val, char *buf, unsigned int buflen);
int wrap_marshall_u_int32(WRAPPERS_ARGS, u_int32_t val, char *buf, unsigned int buflen);
int wrap_marshall_float(WRAPPERS_ARGS, float val, char *buf, unsigned int buflen);
int wrap_marshall_double(WRAPPERS_ARGS, double val, char *buf, unsigned int buflen);
int wrap_marshall_buffer(WRAPPERS_ARGS, const char *val, unsigned int vallen, char *buf, unsigned int buflen);
int wrap_unmarshall_int8(WRAPPERS_ARGS, int8_t *val, const char *buf, unsigned int buflen);
int wrap_unmarshall_int32(WRAPPERS_ARGS, int32_t *val, const char *buf, unsigned int buflen);
int wrap_unmarshall_u_int8(WRAPPERS_ARGS, u_int8_t *val, const char *buf, unsigned int buflen);
int wrap_unmarshall_u_int32(WRAPPERS_ARGS, u_int32_t *val, const char *buf, unsigned int buflen);
int wrap_unmarshall_float(WRAPPERS_ARGS, float *val, const char *buf, unsigned int buflen);
int wrap_unmarshall_double(WRAPPERS_ARGS, double *val, const char *buf, unsigned int buflen);
int wrap_unmarshall_buffer(WRAPPERS_ARGS, char *val, unsigned int vallen, const char *buf, unsigned int buflen);

   
#endif /* _WRAPPERS_H */
