/*****************************************************************************\
 *  $Id: cerebrod_util.h,v 1.1 2005-03-16 15:55:28 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_UTIL_H
#define _CEREBROD_UTIL_H

#include <pthread.h>
#include "cerebrod.h"
#include "hash.h"

void cerebrod_rehash(hash_t *old_hash, int *hash_size, int hash_size_increment,
		     int hash_numnodes, pthread_mutex_t *hash_mutex);

#endif /* _CEREBROD_UTIL_H */
