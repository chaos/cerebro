/*****************************************************************************\
 *  $Id: cerebrod_listener.c,v 1.3 2005-02-01 01:00:13 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <errno.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "cerebrod_listener.h"
#include "cerebrod_config.h"
#include "cerebrod_heartbeat.h"
#include "error.h"
#include "wrappers.h"

#define CEREBROD_LISTENER_HASH_SIZE_DEFAULT   1024
#define CEREBROD_LISTENER_HASH_SIZE_INCREMENT 1024

extern struct cerebrod_config conf;
#ifndef NDEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* NDEBUG */

int initialization_complete = 0;
pthread_mutex_t initialization_complete_lock = PTHREAD_MUTEX_INITIALIZER;

int listener_fd;
pthread_mutex_t listener_fd_lock = PTHREAD_MUTEX_INITIALIZER;
hash_t heartbeat_hash;
int heartbeat_hash_size;
int heartbeat_hash_numnodes;
pthread_mutex_t heartbeat_hash_lock = PTHREAD_MUTEX_INITIALIZER;

static void
_cerebrod_listener_create_and_setup_socket(void)
{
  listener_fd = Socket(AF_INET, SOCK_DGRAM, 0);

#if 0
  if (conf.multicast)
    {
      /* XXX: Probably lots of portability problems here */
      struct ip_mreqn imr;
      int optval;

      memcpy(&imr.imr_multiaddr,
             &conf.speak_to_in_addr,
             sizeof(struct in_addr));
      memcpy(&imr.imr_address,
             &conf.speak_from_in_addr,
             sizeof(struct in_addr));
      imr.imr_ifindex = conf.speak_from_interface_index;

      /* Sort of like a multicast-bind */
      Setsockopt(temp_fd,
		 SOL_IP,
		 IP_MULTICAST_IF,
		 &imr,
		 sizeof(struct ip_mreqn));

      memcpy(&imr.imr_multiaddr,
             &conf.speak_to_in_addr,
             sizeof(struct in_addr));
      memcpy(&imr.imr_address,
             &conf.speak_from_in_addr,
             sizeof(struct in_addr));
      imr.imr_ifindex = conf.speak_from_interface_index;

      /* Sort of like a multicast-bind */
      Setsockopt(temp_fd,
		 SOL_IP,
		 IP_ADD_MEMBERSHIP,
		 &imr,
		 sizeof(struct ip_mreqn));
    }

  /* Even if we're multicasting, the port still needs to be bound */
  speak_from_addr.sin_family = AF_INET;
  speak_from_addr.sin_port = htons(conf.speak_from_port);
  memcpy(&speak_from_addr.sin_addr,
         &conf.speak_from_in_addr,
         sizeof(struct in_addr));
  if (bind(temp_fd, (struct sockaddr *)&speak_from_addr, sizeof(struct sockaddr_in)))
    {
      err_debug("_cerebrod_speaker_create_and_setup_socket: bind: %s",
                strerror(errno));
      return -1;
    }

  /* Connect to the speak to address */
  speak_to_addr.sin_family = AF_INET;
  speak_from_addr.sin_port = htons(conf.speak_to_port);
  memcpy(&speak_to_addr.sin_addr,
         &conf.speak_to_in_addr,
         sizeof(struct in_addr));
  if (connect(temp_fd, (struct sockaddr *)&speak_to_addr, sizeof(struct sockaddr_in)) < 0)
    {
      err_debug("_cerebrod_speaker_create_and_setup_socket: connect: %s",
                strerror(errno));
      return -1;
    }
#endif
}

static void
_cerebrod_listener_initialize(void)
{
  Pthread_mutex_lock(&initialization_complete_lock);
  if (initialization_complete)
    return;

  _cerebrod_listener_create_and_setup_socket();

  heartbeat_hash_size = CEREBROD_LISTENER_HASH_SIZE_DEFAULT;
  heartbeat_hash_numnodes = 0;
  heartbeat_hash = Hash_create(heartbeat_hash_size,
			       (hash_key_f)hash_key_string,
			       (hash_cmp_f)strcmp,
			       (hash_del_f)list_destroy);
  initialization_complete++;
  Pthread_mutex_unlock(&initialization_complete_lock);
}

static void
_cerebrod_listener_dump_heartbeat(struct cerebrod_heartbeat *hb)
{
#ifndef NDEBUG
  assert(hb);

  if (conf.debug)
    {
      time_t t;
      struct tm tm;
      char strbuf[CEREBROD_STRING_BUFLEN];

      t = Time(NULL);
      Localtime_r(&t, &tm);
      strftime(strbuf, CEREBROD_STRING_BUFLEN, "%H:%M:%S", &tm);

      Pthread_mutex_lock(&debug_output_mutex);
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Received Heartbeat: %s\n", strbuf);     
      fprintf(stderr, "* -----------------------\n");
      cerebrod_heartbeat_dump(hb);
      fprintf(stderr, "**************************************\n");
      Pthread_mutex_unlock(&debug_output_mutex);
    }
#endif /* NDEBUG */
}

void *
cerebrod_listener(void *arg)
{
  _cerebrod_listener_initialize();

  return NULL;			/* NOT REACHED */
}
