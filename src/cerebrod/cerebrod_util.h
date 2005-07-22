/*****************************************************************************\
 *  $Id: cerebrod_util.h,v 1.14 2005-07-22 17:21:07 achu Exp $
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
 * responsible for locking any locks before calling rehash.
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
 * Cerebrod_socket_setup
 *
 * function prototype for a function that will return a socket.
 */
typedef int (*Cerebrod_socket_setup)(void);

/* 
 * cerebrod_reinit_socket
 *
 * This function helps various looping network servers or clients
 * reinitialize their sockets appropriately.
 *
 * For networking errnos EINVAL, EBADF, ENODEV, we assume the network
 * device has been temporarily brought down then back up.  For
 * example, this can occur if the administrator runs
 * '/etc/init.d/network restart'.  
 *
 * If old_fd < 0, the network device just isn't back up yet from
 * the previous time we got an errno EINVAL, EBADF, or
 * ENODEV.
 *
 * Returns new (or possibley old) fd on success, -1 on error
 */
int
cerebrod_reinit_socket(int old_fd, 
                       Cerebrod_socket_setup socket_setup,
                       char *msg);

#endif /* _CEREBROD_UTIL_H */
