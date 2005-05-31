/*****************************************************************************\
 *  $Id: cerebrod_util.h,v 1.10 2005-05-31 22:06:03 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_UTIL_H
#define _CEREBROD_UTIL_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_PTHREAD_H
#include <pthread.h>
#endif /* HAVE_PTHREAD_H */

#include "hash.h"

/* 
 * list_find_first_string
 *
 * callback function for list_find_first to find a string
 */
int list_find_first_string(void *x, void *key);

/* 
 * cerebrod_rehash
 *
 * rehash the contents of old_hash into a new hash.  Caller is
 * responsible for locking any locks before calling cerebrod_rehash.
 *
 * - old_hash - pointer to old hash
 * - hash_size - pointer to integer with current hash size
 * - hash_size_increment - amount to increase hash size
 * - hash_num - current number of entries in hash
 * - hash_mutex - mutex to check for locking
 *
 * Returns new_hash in 'old_hash', and new hash size in 'hash_size'.
 */
void cerebrod_rehash(hash_t *old_hash, 
		     int *hash_size, 
		     int hash_size_increment,
		     int hash_num, 
		     pthread_mutex_t *hash_mutex);

/* 
 * cerebrod_service_connection
 *
 * function prototype for a function that will service a TCP
 * connection.  Function is passed a void * pointer to the
 * TCP connection's file descriptor.  Function is responsible
 * for closing the file descriptor.
 *
 * Function is executed in detached state, so any return is
 * ignored.
 */
typedef void *(*Cerebrod_service_connection)(void *arg);

/* 
 * cerebrod_tcp_data_server
 *
 * Will execute 'service_connection' thread after a TCP connection
 * received, passing it the connection's file descriptor.  The thread
 * is executed in the detached state, so the return value will be
 * ignored.
 */
void cerebrod_tcp_data_server(Cerebrod_service_connection service_connection,
                              unsigned int port,
                              unsigned int backlog,
                              unsigned int reinitialize_wait_time);

#endif /* _CEREBROD_UTIL_H */
