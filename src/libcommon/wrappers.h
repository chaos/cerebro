/*****************************************************************************\
 *  $Id: wrappers.h,v 1.21 2005-03-22 01:34:54 achu Exp $
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
#include <pthread.h>
#include <signal.h>
#include <dirent.h>

#if !WITH_STATIC_MODULES
#include "ltdl.h"
#endif
#include "list.h"
#include "hash.h"

/* 
 * Memory/String Wrappers 
 */
#define Malloc(size) \
        wrap_malloc(__FILE__, __LINE__, size)
#define Free(ptr) \
        wrap_free(__FILE__, __LINE__, ptr)
#define Strdup(s) \
        wrap_strdup(__FILE__, __LINE__, s)
#define Strncpy(dest, src, n) \
        wrap_strncpy(__FILE__, __LINE__, dest, src, n)

void * wrap_malloc(const char *file, int line, size_t size);
void wrap_free(const char *file, int line, void *ptr);
char * wrap_strdup(const char *file, int line, const char *s);
char * wrap_strncpy(const char *file, int line, char *dest, const char *src, size_t n);

/* Special wrapper for List/Hash libraries */
void _Free(void *ptr);

/* 
 * File System Wrappers 
 */
#define Open(pathname, flags, mode) \
        wrap_open(__FILE__, __LINE__, pathname, flags, mode)
#define Close(fd) \
        wrap_close(__FILE__, __LINE__, fd)
#define Read(fd, buf, count) \
        wrap_read(__FILE__, __LINE__, fd, buf, count) 
#define Write(fd, buf, count) \
        wrap_write(__FILE__, __LINE__, fd, buf, count) 
#define Chdir(path) \
        wrap_chdir(__FILE__, __LINE__, path)
#define Stat(path, buf) \
        wrap_stat(__FILE__, __LINE__, path, buf)
#define Umask(mask) \
        wrap_umask(__FILE__, __LINE__, mask)
#define Opendir(name) \
        wrap_opendir(__FILE__, __LINE__, name)
#define Closedir(dir) \
        wrap_closedir(__FILE__, __LINE__, dir)

int wrap_open(const char *file, int line, const char *pathname, int flags, int mode);
int wrap_close(const char *file, int line, int fd);
ssize_t wrap_read(const char *file, int line, int fd, void *buf, size_t count);
ssize_t wrap_write(const char *file, int line, int fd, const void *buf, size_t count);
int wrap_chdir(const char *file, int line, const char *path);
int wrap_stat(const char *file, int line, const char *path, struct stat *buf);
mode_t wrap_umask(const char *file, int line, mode_t mask);
DIR *wrap_opendir(const char *file, int line, const char *name);
int wrap_closedir(const char *file, int line, DIR *dir);

/* 
 * Networking Wrappers 
 */
#define Socket(domain, type, protocol) \
        wrap_socket(__FILE__, __LINE__, domain, type, protocol)
#define Bind(sockfd, my_addr, addrlen) \
        wrap_bind(__FILE__, __LINE__, sockfd, my_addr, addrlen)
#define Connect(sockfd, serv_addr, addrlen) \
        wrap_connect(__FILE__, __LINE__, sockfd, serv_addr, addrlen)
#define Getsockopt(s, level, optname, optval, optlen) \
        wrap_getsockopt(__FILE__, __LINE__, s, level, optname, optval, optlen)
#define Setsockopt(s, level, optname, optval, optlen) \
        wrap_setsockopt(__FILE__, __LINE__, s, level, optname, optval, optlen)
#define Inet_pton(af, src, dst) \
        wrap_inet_pton(__FILE__, __LINE__, af, src, dst)

int wrap_socket(const char *file, int line, int domain, int type, int protocol);
int wrap_bind(const char *file, int line, int sockfd, struct sockaddr *my_addr, socklen_t addrlen);
int wrap_connect(const char *file, int line, int sockfd, struct sockaddr *serv_addr, socklen_t addrlen);
int wrap_getsockopt(const char *file, int line, int s, int level, int optname, void *optval, socklen_t *optlen);
int wrap_setsockopt(const char *file, int line, int s, int level, int optname, const void *optval, socklen_t optlen);
int wrap_inet_pton(const char *file, int line, int af, const char *src, void *dst);

/* 
 * Time Wrappers 
 */
#define Time(t) \
        wrap_time(__FILE__, __LINE__, t)
#define Localtime(timep) \
        wrap_localtime(__FILE__, __LINE__, timep)
#define Localtime_r(timep, result) \
        wrap_localtime_r(__FILE__, __LINE__, timep, result)
#define Gettimeofday(tv, tz) \
        wrap_gettimeofday(__FILE__, __LINE__, tv, tz)

int wrap_gettimeofday(const char *file, int line, struct timeval *tv, struct timezone *tz);
time_t wrap_time(const char *file, int line, time_t *t);
struct tm *wrap_localtime(const char *file, int line, const time_t *timep);
struct tm *wrap_localtime_r(const char *file, int line, const time_t *timep, struct tm *result);

/* 
 * Pthread Wrappers 
 */
#define Pthread_create(thread, attr, start_routine, arg) \
        wrap_pthread_create(__FILE__, __LINE__, thread, attr, start_routine, arg)
#define Pthread_attr_init(attr) \
        wrap_pthread_attr_init(__FILE__, __LINE__, attr)
#define Pthread_attr_destroy(attr) \
        wrap_pthread_attr_destroy(__FILE__, __LINE__, attr)
#define Pthread_attr_setdetachstate(attr, detachstate) \
        wrap_pthread_attr_setdetachstate(__FILE__, __LINE__, attr, detachstate)
#define Pthread_mutex_lock(mutex) \
        wrap_pthread_mutex_lock(__FILE__, __LINE__, mutex)
#define Pthread_mutex_trylock(mutex) \
        wrap_pthread_mutex_trylock(__FILE__, __LINE__, mutex)
#define Pthread_mutex_unlock(mutex) \
        wrap_pthread_mutex_unlock(__FILE__, __LINE__, mutex)
#define Pthread_mutex_init(mutex, mutexattr) \
        wrap_pthread_mutex_init(__FILE__, __LINE__, mutex, mutexattr)
#define Pthread_cond_signal(cond) \
        wrap_pthread_cond_signal(__FILE__, __LINE__, cond)
#define Pthread_cond_wait(cond, mutex) \
        wrap_pthread_cond_wait(__FILE__, __LINE__, cond, mutex)

int wrap_pthread_create(const char *file, int line, pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);
int wrap_pthread_attr_init(const char *file, int line, pthread_attr_t *attr);
int wrap_pthread_attr_destroy(const char *file, int line, pthread_attr_t *attr);
int wrap_pthread_attr_setdetachstate(const char *file, int line, pthread_attr_t *attr, int detachstate);
int wrap_pthread_mutex_lock(const char *file, int line, pthread_mutex_t *mutex);
int wrap_pthread_mutex_trylock(const char *file, int line, pthread_mutex_t *mutex);
int wrap_pthread_mutex_unlock(const char *file, int line, pthread_mutex_t *mutex);
int wrap_pthread_mutex_init(const char *file, int line, pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);
int wrap_pthread_cond_signal(const char *file, int line, pthread_cond_t *cond);
int wrap_pthread_cond_wait(const char *file, int line, pthread_cond_t *cond, pthread_mutex_t *mutex);

/* 
 * Misc System Call Wrappers 
 */
#define Fork() \
        wrap_fork(__FILE__, __LINE__);
#define Signal(signum, handler) \
        wrap_signal(__FILE__, __LINE__, signum, handler)
#define Gethostname(name, len) \
        wrap_gethostname(__FILE__, __LINE__, name, len)

typedef void (*Sighandler_t)(int);

pid_t wrap_fork(const char *file, int line);
Sighandler_t wrap_signal(const char *file, int line, int signum, Sighandler_t handler);
int wrap_gethostname(const char *file, int line, char *name, size_t len);

#if !WITH_STATIC_MODULES
/*
 * ltdl wrappers
 */

#define Lt_dlinit() \
        wrap_lt_dlinit(__FILE__, __LINE__)
#define Lt_dlexit() \
        wrap_lt_dlexit(__FILE__, __LINE__)
#define Lt_dlopen(filename) \
        wrap_lt_dlopen(__FILE__, __LINE__, filename)
#define Lt_dlsym(handle, symbol) \
        wrap_lt_dlsym(__FILE__, __LINE__, handle, symbol)
#define Lt_dlclose(handle) \
        wrap_lt_dlclose(__FILE__, __LINE__, handle)

int wrap_lt_dlinit(const char *file, int line);
int wrap_lt_dlexit(const char *file, int line);
lt_dlhandle wrap_lt_dlopen(const char *file, int line, const char *filename);
lt_ptr wrap_lt_dlsym(const char *file, int line, void *handle, char *symbol);
int wrap_lt_dlclose(const char *file, int line, void *handle);
#endif /* !WITH_STATIC_MODULES */

/* 
 * List lib wrappers 
 */
#define List_create(f) \
        wrap_list_create(__FILE__, __LINE__, f)
#define List_destroy(l) \
        wrap_list_destroy(__FILE__, __LINE__, l)
#define List_count(l) \
        wrap_list_count(__FILE__, __LINE__, l)
#define List_append(l, x) \
        wrap_list_append(__FILE__, __LINE__, l, x)
#define List_delete_all(l, f, key) \
        wrap_list_delete_all(__FILE__, __LINE__, l, f, key)
#define List_for_each(l, f, arg) \
        wrap_list_for_each(__FILE__, __LINE__, l, f, arg)
#define List_iterator_create(l) \
        wrap_list_iterator_create(__FILE__, __LINE__, l)
#define List_iterator_destroy(i) \
        wrap_list_iterator_destroy(__FILE__, __LINE__, i)

List wrap_list_create(const char *file, int line, ListDelF f);
void wrap_list_destroy(const char *file, int line, List l);
int wrap_list_count(const char *file, int line, List l);
void *wrap_list_append (const char *file, int line, List l, void *x);
int wrap_list_delete_all(const char *file, int line, List l, ListFindF f, void *key);
int wrap_list_for_each(const char *file, int line, List l, ListForF f, void *arg);
ListIterator wrap_list_iterator_create(const char *file, int line, List l);
void wrap_list_iterator_destroy(const char *file, int line, ListIterator i);

/* 
 * Hash lib wrappers 
 */
#define Hash_create(size, key_f, cmp_f, del_f) \
        wrap_hash_create(__FILE__, __LINE__, size, key_f, cmp_f, del_f)
#define Hash_count(h) \
        wrap_hash_count(__FILE__, __LINE__, h)
#define Hash_find(h, key) \
        wrap_hash_find(__FILE__, __LINE__, h, key)
#define Hash_insert(h, key, data) \
        wrap_hash_insert(__FILE__, __LINE__, h, key, data)
#define Hash_remove(h, key) \
        wrap_hash_remove(__FILE__, __LINE__, h, key)
#define Hash_delete_if(h, argf, arg) \
        wrap_hash_delete_if(__FILE__, __LINE__, h, argf, arg)
#define Hash_for_each(h, argf, arg) \
        wrap_hash_for_each(__FILE__, __LINE__, h, argf, arg)
#define Hash_destroy(h) \
        wrap_hash_destroy(__FILE__, __LINE__, h)

hash_t wrap_hash_create (const char *file, int line, int size, hash_key_f key_f, hash_cmp_f cmp_f, hash_del_f del_f);
int wrap_hash_count(const char *file, int line, hash_t h);
void *wrap_hash_find(const char *file, int line, hash_t h, const void *key);
void *wrap_hash_insert(const char *file, int line, hash_t h, const void *key, void *data);
void *wrap_hash_remove (const char *file, int line, hash_t h, const void *key);
int wrap_hash_delete_if(const char *file, int line, hash_t h, hash_arg_f argf, void *arg);
int wrap_hash_for_each(const char *file, int line, hash_t h, hash_arg_f argf, void *arg);
void wrap_hash_destroy(const char *file, int line, hash_t h);

#endif /* _WRAPPERS_H */
