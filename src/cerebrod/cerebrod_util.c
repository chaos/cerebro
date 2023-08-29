/*****************************************************************************\
 *  $Id: cerebrod_util.c,v 1.42 2010-02-02 01:01:20 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2018 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2005-2007 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <https://github.com/chaos/cerebro>.
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
 *  with Cerebro.  If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <assert.h>
#include <errno.h>

#include "cerebrod_config.h"
#include "cerebrod_debug.h"
#include "cerebrod_util.h"

#include "debug.h"
#include "wrappers.h"

extern struct cerebrod_config conf;

#define CEREBROD_REINITIALIZE_TIME 2

#if !WITH_CEREBROD_SPEAKER_ONLY

/*
 * _hash_removeall
 *
 * callback for hash_for_each that inserts entries into the new hash.
 *
 * Returns 1 for success, 0 on failure, -1 on fatal error
 */
static int
_hash_reinsert(void *data, const void *key, void *arg)
{
  hash_t newhash;

  assert(data && key && arg);

  newhash = *((hash_t *)arg);
  Hash_insert(newhash, key, data);
  return 1;
}

/*
 * _hash_removeall
 *
 * callback for hash_remove_if that returns 1, signifying the removal
 * of all hashed entries
 */
static int
_hash_removeall(void *data, const void *key, void *arg)
{
  return 1;
}

void
cerebrod_rehash(hash_t *old_hash,
		int *hash_size,
		int hash_size_increment,
		int hash_num,
		pthread_mutex_t *hash_mutex)
{
  hash_t new_hash;

  assert(old_hash && hash_size && hash_size_increment && hash_num);

#if CEREBRO_DEBUG
  /* Should be called with lock already set */
  if (hash_mutex)
    {
      int rv = Pthread_mutex_trylock(hash_mutex);
      if (rv != EBUSY)
        CEREBROD_EXIT(("mutex not locked: rv=%d", rv));
    }
#endif /* CEREBRO_DEBUG */

  *hash_size += hash_size_increment;

  new_hash = Hash_create(*hash_size,
			 (hash_key_f)hash_key_string,
			 (hash_cmp_f)strcmp,
			 (hash_del_f)_Free);

  if (Hash_for_each(*old_hash, _hash_reinsert, &new_hash) != hash_num)
    CEREBROD_EXIT(("invalid reinsert: hash_num=%d", hash_num));

  if (Hash_remove_if(*old_hash, _hash_removeall, NULL) != hash_num)
    CEREBROD_EXIT(("invalid removeall: hash_num=%d", hash_num));

  Hash_destroy(*old_hash);

  *old_hash = new_hash;
}

#endif /* !WITH_CEREBROD_SPEAKER_ONLY */

int
cerebrod_reinit_socket(int old_fd,
                       int num,
                       Cerebrod_socket_setup socket_setup,
                       char *msg)
{
  int fd = old_fd;

  assert(socket_setup && msg);

  if (errno == EINVAL
      || errno == EBADF
      || errno == ENODEV
      || errno == ENETDOWN
      || errno == ENETUNREACH
      || old_fd < 0)
    {
      if (!(old_fd < 0))
        close(old_fd);       /* no-wrapper, make best effort */

      if ((fd = socket_setup(num)) < 0)
        {
          CEREBROD_DBG(("%s: error re-initializing socket", msg));

          /* Wait a bit, so we don't spin */
          sleep(CEREBROD_REINITIALIZE_TIME);
        }
      else
        CEREBROD_DBG(("success re-initializing socket"));
    }
  else if (errno == EINTR)
    CEREBROD_DBG(("%s: %s", msg, strerror(errno)));
  else
    CEREBROD_EXIT(("%s: %s", msg, strerror(errno)));

  return fd;
}
