/*****************************************************************************\
 *  $Id: cerebrod_util.c,v 1.23 2005-06-27 17:24:09 achu Exp $
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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "cerebrod_config.h"
#include "cerebrod_util.h"

#include "debug.h"
#include "wrappers.h"

extern struct cerebrod_config conf;

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

/*
 * _cerebrod_create_and_setup_socket
 *
 * Create and setup the server socket.  Do not use wrappers in this
 * function.  We want to give the server additional chances to
 * "survive" an error condition.
 *
 * Returns file descriptor on success, -1 on error
 */
static int
_cerebrod_create_and_setup_socket(unsigned int port,
                                  unsigned int backlog)
{
  struct sockaddr_in server_addr;
  int temp_fd, optval = 1;

  if ((temp_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      CEREBRO_DBG(("socket: %s", strerror(errno)));
      return -1;
    }

  /* Configuration checks ensure destination ip is on this machine if
   * it is a non-multicast address.
   */
  memset(&server_addr, '\0', sizeof(struct sockaddr_in));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(temp_fd,
           (struct sockaddr *)&server_addr,
           sizeof(struct sockaddr_in)) < 0)
    {
      CEREBRO_DBG(("bind: %s", strerror(errno)));
      return -1;
    }

  if (listen(temp_fd, backlog) < 0)
    {
      CEREBRO_DBG(("listen: %s", strerror(errno)));
      return -1;
    }


  /* For quick start/restart */
  if (setsockopt(temp_fd,
                 SOL_SOCKET,
                 SO_REUSEADDR,
                 &optval,
                 sizeof(int)) < 0)
    {
      CEREBRO_DBG(("setsockopt: %s", strerror(errno)));
      return -1;
    }

  return temp_fd;
}

void
cerebrod_tcp_data_server(Cerebrod_service_connection service_connection,
                         unsigned int port,
                         unsigned int backlog,
                         unsigned int reinitialize_wait_time)
{
  int server_fd;

  assert(service_connection);
  assert(port);
  assert(backlog);
  assert(reinitialize_wait_time);

  if ((server_fd = _cerebrod_create_and_setup_socket(port, backlog)) < 0)
    CEREBRO_EXIT(("server_fd setup failed"));
  
  for (;;)
    {
      pthread_t thread;
      pthread_attr_t attr;
      int client_fd, client_addr_len, *arg;
      struct sockaddr_in client_addr;

      client_addr_len = sizeof(struct sockaddr_in);
      if ((client_fd = accept(server_fd, 
                              (struct sockaddr *)&client_addr, 
                              &client_addr_len)) < 0)
	{
          /* For errnos EINVAL, EBADF, ENODEV, assume the device has
           * been temporarily brought down then back up.  For example,
           * this can occur if the administrator runs
           * '/etc/init.d/network restart'.  We just need to re-setup
           * the socket.
           *
           * If fd < 0, the network device just isn't back up yet from
           * the previous time we got an errno EINVAL, EBADF, or
           * ENODEV.
           */
          if (errno == EINVAL
	      || errno == EBADF
	      || errno == ENODEV
	      || server_fd < 0)
            {
              if (!(server_fd < 0))
		close(server_fd);	/* no-wrapper, make best effort */

              if ((server_fd = _cerebrod_create_and_setup_socket(port,
                                                                 backlog)) < 0)
		{
		  CEREBRO_DBG(("error re-initializing socket"));

		  /* Wait a bit, so we don't spin */
		  sleep(reinitialize_wait_time);
		}
              else
                CEREBRO_DBG(("success re-initializing socket"));
            }
          else if (errno == EINTR)
            CEREBRO_DBG(("accept: %s", strerror(errno)));
          else
            CEREBRO_EXIT(("accept: %s", strerror(errno)));
	}

      if (client_fd < 0)
	continue;

      /* Pass off connection to thread */
      Pthread_attr_init(&attr);
      Pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      arg = Malloc(sizeof(int));
      *arg = client_fd;
      Pthread_create(&thread, 
                     &attr, 
                     service_connection, 
                     (void *)arg);
      Pthread_attr_destroy(&attr);

    }

  return;			/* NOT REACHED */
}
