/*****************************************************************************\
 *  $Id: cerebrod_util.h,v 1.2 2005-03-18 23:27:05 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_UTIL_H
#define _CEREBROD_UTIL_H

#include <pthread.h>

#include "cerebrod.h"
#include "hash.h"

void cerebrod_rehash(hash_t *old_hash, int *hash_size, int hash_size_increment,
		     int hash_numnodes, pthread_mutex_t *hash_mutex);

typedef int (*Cerebrod_load_module)(char *);

int cerebrod_search_dir_for_module(char *search_dir, char **module_list,
                                   Cerebrod_load_module load_module);
                                   

#endif /* _CEREBROD_UTIL_H */
