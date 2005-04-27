/*****************************************************************************\
 *  $Id: cerebrod_util.h,v 1.8 2005-04-27 18:11:35 achu Exp $
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

#endif /* _CEREBROD_UTIL_H */
