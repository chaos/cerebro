/*****************************************************************************\
 *  $Id: cerebrod_util.c,v 1.30 2005-07-01 16:22:31 achu Exp $
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
#include "cerebrod_util.h"

#include "debug.h"
#include "wrappers.h"

extern struct cerebrod_config conf;

#define CEREBROD_REINITIALIZE_TIME 2

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
 * callback for hash_delete_if that returns 1, signifying the removal
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

  assert(old_hash && hash_size && hash_size_increment && hash_num && hash_mutex);

#if CEREBRO_DEBUG
  /* Should be called with lock already set */
  if (hash_mutex)
    {
      int rv = Pthread_mutex_trylock(hash_mutex);
      if (rv != EBUSY)
        CEREBRO_EXIT(("mutex not locked: rv=%d", rv));
    }
#endif /* CEREBRO_DEBUG */

  *hash_size += hash_size_increment;
  
  new_hash = Hash_create(*hash_size,
			 (hash_key_f)hash_key_string,
			 (hash_cmp_f)strcmp,
			 (hash_del_f)_Free);
  
  if (Hash_for_each(*old_hash, _hash_reinsert, &new_hash) != hash_num)
    CEREBRO_EXIT(("invalid reinsert: hash_num=%d", hash_num));

  if (Hash_delete_if(*old_hash, _hash_removeall, NULL) != hash_num)
    CEREBRO_EXIT(("invalid removeall: hash_num=%d", hash_num));

  Hash_destroy(*old_hash);

  *old_hash = new_hash;
}

int
cerebrod_reinit_socket(int old_fd, Cerebrod_socket_setup socket_setup, char *msg)
{
  int fd = old_fd;

  assert(socket_setup && msg);

  if (errno == EINVAL || errno == EBADF || errno == ENODEV || old_fd < 0)
    {
      if (!(old_fd < 0))
        close(old_fd);       /* no-wrapper, make best effort */

      if ((fd = socket_setup()) < 0)
        {
          CEREBRO_DBG(("%s: error re-initializing socket", msg));

          /* Wait a bit, so we don't spin */
          sleep(CEREBROD_REINITIALIZE_TIME);
        }
      else
        CEREBRO_DBG(("success re-initializing socket"));
    }
  else if (errno == EINTR)
    CEREBRO_DBG(("%s: %s", msg, strerror(errno)));
  else
    CEREBRO_EXIT(("%s: %s", msg, strerror(errno)));

  return fd;
}
