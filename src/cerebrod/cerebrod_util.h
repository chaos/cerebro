/*****************************************************************************\
 *  $Id: cerebrod_util.h,v 1.5 2005-03-22 07:27:30 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_UTIL_H
#define _CEREBROD_UTIL_H

#include <pthread.h>

#include "cerebrod.h"
#include "hash.h"

/*
 * Cerebrod_load_module
 *
 * function prototype for loading a module. Passed a module
 * file to load.
 *
 * Returns 1 on loading success, 0 on loading failure, -1 on fatal error
 */
typedef int (*Cerebrod_load_module)(char *);

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

#if !WITH_STATIC_MODULES
/*
 * cerebrod_search_dir_for_module
 *
 * search a directory for a module
 *
 * - search_dir - directory to search
 * - modules_list - list of modules to search for
 * - modules_list_len - length of list
 * - load_module - function to call when module is found
 *
 * Returns 1 when a module is found, 0 when one is not.
 */
int cerebrod_search_dir_for_module(char *search_dir, 
				   char **modules_list,
				   int modules_list_len,
                                   Cerebrod_load_module load_module);
#endif /* !WITH_STATIC_MODULES */                                   

#endif /* _CEREBROD_UTIL_H */
