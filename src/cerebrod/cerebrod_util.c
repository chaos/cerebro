/*****************************************************************************\
 *  $Id: cerebrod_util.c,v 1.24 2005-06-28 00:32:12 achu Exp $
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

int 
list_find_first_string(void *x, void *key)
{
  assert(x);
  assert(key);

  return (!strcmp((char *)x, (char *)key)) ? 1 : 0;
}

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

  assert(data);
  assert(key);
  assert(arg);

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
  int num;

  assert(old_hash);
  assert(hash_size);
  assert(hash_size_increment > 0);
  assert(hash_num > 0);
  assert(hash_mutex);

#if CEREBRO_DEBUG
  /* Should be called with lock already set */
  if (hash_mutex)
    {
      int rv = Pthread_mutex_trylock(hash_mutex);
      if (rv != EBUSY)
        CEREBRO_EXIT(("%s(%s:%d): mutex not locked: rv=%d",
                      __FILE__, __FUNCTION__, __LINE__, rv));
    }
#endif /* CEREBRO_DEBUG */

  *hash_size += hash_size_increment;
  
  new_hash = Hash_create(*hash_size,
			 (hash_key_f)hash_key_string,
			 (hash_cmp_f)strcmp,
			 (hash_del_f)_Free);
  
  num = Hash_for_each(*old_hash, _hash_reinsert, &new_hash);
  if (num != hash_num)
    CEREBRO_EXIT(("invalid reinsert: num=%d hash_num=%d", num, hash_num));

  num = Hash_delete_if(*old_hash, _hash_removeall, NULL);
  if (num != hash_num)
    CEREBRO_EXIT(("invalid removeall: num=%d hash_num=%d", num, hash_num));

  Hash_destroy(*old_hash);

  *old_hash = new_hash;
}

int
cerebrod_reinitialize_socket(int old_fd,
                             Cerebrod_socket_setup socket_setup,
                             char *debug_msg)
{
  int fd = old_fd;

  if (!socket_setup || !debug_msg)
    CEREBRO_EXIT(("invalid parameters"));

  if (errno == EINVAL || errno == EBADF || errno == ENODEV || old_fd < 0)
    {
      if (!(old_fd < 0))
        close(old_fd);       /* no-wrapper, make best effort */
                                                                                      
      if ((fd = socket_setup()) < 0)
        {
          CEREBRO_DBG(("%s: error re-initializing socket", debug_msg));
                                                                                      
          /* Wait a bit, so we don't spin */
          sleep(CEREBROD_REINITIALIZE_TIME);
        }
      else
        CEREBRO_DBG(("success re-initializing socket"));
    }
  else if (errno == EINTR)
    CEREBRO_DBG(("%s: %s", debug_msg, strerror(errno)));
  else
    CEREBRO_EXIT(("%s: %s", debug_msg, strerror(errno)));

  return fd;
}
