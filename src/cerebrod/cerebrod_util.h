/*****************************************************************************\
 *  $Id: cerebrod_util.h,v 1.3 2005-03-19 19:06:24 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_UTIL_H
#define _CEREBROD_UTIL_H

#include <pthread.h>

#include "cerebrod.h"
#include "hash.h"

void cerebrod_rehash(hash_t *old_hash, int *hash_size, int hash_size_increment,
		     int hash_numnodes, pthread_mutex_t *hash_mutex);

typedef int (*Cerebrod_load_module)(char *);

int cerebrod_search_dir_for_module(char *search_dir, 
				   char **modules_list,
				   int modules_list_len,
                                   Cerebrod_load_module load_module);
                                   

#endif /* _CEREBROD_UTIL_H */
