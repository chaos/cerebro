/*****************************************************************************\
 *  $Id: cerebrod_util.c,v 1.5 2005-03-20 21:24:58 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <netinet/in.h>
#include <assert.h>
#include <errno.h>

#include <sys/param.h>

#include "cerebrod_util.h"
#include "cerebrod.h"
#include "error.h"
#include "wrappers.h"

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
  int rv;

  assert(old_hash);
  assert(hash_size);
  assert(hash_size_increment > 0);
  assert(hash_num > 0);
  assert(hash_mutex);

#ifndef NDEBUG
  /* Should be called with lock already set */
  if (hash_mutex)
    {
      rv = Pthread_mutex_trylock(hash_mutex);
      if (rv != EBUSY)
	err_exit("cerebrod_heartbeat_dump: hash_mutex not locked");
    }
#endif /* NDEBUG */

  *hash_size += hash_size_increment;
  
  new_hash = Hash_create(*hash_size,
			 (hash_key_f)hash_key_string,
			 (hash_cmp_f)strcmp,
			 (hash_del_f)_Free);
  
  rv = Hash_for_each(*old_hash, _hash_reinsert, &new_hash);
  if (rv != hash_num)
    err_exit("_rehash: invalid reinsert count: rv=%d num=%d",
             rv, hash_num);

  rv = Hash_delete_if(*old_hash, _hash_removeall, NULL);
  if (rv != hash_num)
    err_exit("_rehash: invalid removeall count: rv=%d num=%d",
             rv, hash_num);

  Hash_destroy(*old_hash);

  *old_hash = new_hash;
}

int
cerebrod_search_dir_for_module(char *search_dir,
                               char **modules_list,
			       int modules_list_len,
                               Cerebrod_load_module load_module)
{
  DIR *dir;
  int i = 0, found = 0;

  assert(search_dir);
  assert(modules_list);
  assert(modules_list_len > 0);
  assert(load_module);

  if (!(dir = opendir(search_dir)))
    return 0;

  for (i = 0; i < modules_list_len; i++)
    {
      struct dirent *dirent;

      while ((dirent = readdir(dir)))
        {
          if (!strcmp(dirent->d_name, modules_list[i]))
            {
              char filebuf[MAXPATHLEN+1];
              int ret;

              memset(filebuf, '\0', MAXPATHLEN+1);
              snprintf(filebuf, MAXPATHLEN, "%s/%s",
                       search_dir, modules_list[i]);

              if ((ret = load_module(filebuf)) < 0)
                err_exit("cerebrod_search_dir_for_module: load_module: %s",
                         strerror(errno));

              if (ret)
                {
                  found++;
                  goto done;
                }
            }
        }

      rewinddir(dir);
    }

 done:
  Closedir(dir);

  return (found) ? 1 : 0;
}