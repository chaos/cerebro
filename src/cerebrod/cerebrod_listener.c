/*****************************************************************************\
 *  $Id: cerebrod_listener.c,v 1.9 2005-02-04 00:09:01 achu Exp $
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
#include "fd.h"
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
hash_t cluster_data_hash;
int cluster_data_hash_size;
int cluster_data_hash_numnodes;
pthread_mutex_t cluster_data_hash_lock = PTHREAD_MUTEX_INITIALIZER;

static int
_cerebrod_listener_create_and_setup_socket(void)
{
  struct sockaddr_in heartbeat_addr;
  int temp_fd;

  if ((temp_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      err_debug("_cerebrod_listener_create_and_setup_socket: socket: %s",
		strerror(errno));
      return -1;
    }

  if (conf.multicast)
    {
      /* XXX: Probably lots of portability problems here */
      struct ip_mreqn imr;

      memcpy(&imr.imr_multiaddr,
             &conf.heartbeat_destination_in_addr,
             sizeof(struct in_addr));
      memcpy(&imr.imr_address,
             &conf.heartbeat_in_addr,
             sizeof(struct in_addr));
      imr.imr_ifindex = conf.heartbeat_interface_index;

      /* Sort of like a multicast-bind */
      if (setsockopt(temp_fd,
		     SOL_IP,
		     IP_MULTICAST_IF,
		     &imr,
		     sizeof(struct ip_mreqn)) < 0)
	{
	  err_debug("_cerebrod_listener_create_and_setup_socket: setsockopt: %s",
		    strerror(errno));
	  return -1;
	}

      memcpy(&imr.imr_multiaddr,
             &conf.heartbeat_destination_in_addr,
             sizeof(struct in_addr));
      memcpy(&imr.imr_address,
             &conf.heartbeat_in_addr,
             sizeof(struct in_addr));
      imr.imr_ifindex = conf.heartbeat_interface_index;

      /* Join the multicast group */
      if (setsockopt(temp_fd,
		     SOL_IP,
		     IP_ADD_MEMBERSHIP,
		     &imr,
		     sizeof(struct ip_mreqn)) < 0)
	{
	  err_debug("_cerebrod_listener_create_and_setup_socket: setsockopt: %s",
		    strerror(errno));
	  return -1;
	}
    }

  /* Even if we're multicasting, the port still needs to be bound */
  heartbeat_addr.sin_family = AF_INET;
  heartbeat_addr.sin_port = htons(conf.heartbeat_destination_port);
  memcpy(&heartbeat_addr.sin_addr,
         &conf.heartbeat_in_addr,
         sizeof(struct in_addr));
  if (bind(temp_fd, (struct sockaddr *)&heartbeat_addr, sizeof(struct sockaddr_in)) < 0)
    {
      err_debug("_cerebrod_listener_create_and_setup_socket: bind: %s",
		strerror(errno));
      return -1;
    }

  return temp_fd;
}

static void
_cerebrod_listener_initialize(void)
{
  Pthread_mutex_lock(&initialization_complete_lock);
  if (initialization_complete)
    goto done;

  Pthread_mutex_lock(&listener_fd_lock);
  if ((listener_fd = _cerebrod_listener_create_and_setup_socket()) < 0)
    err_exit("_cerebrod_listener_initialize: listener_fd setup failed");
  Pthread_mutex_unlock(&listener_fd_lock);

  cluster_data_hash_size = CEREBROD_LISTENER_HASH_SIZE_DEFAULT;
  cluster_data_hash_numnodes = 0;
  cluster_data_hash = Hash_create(cluster_data_hash_size,
				  (hash_key_f)hash_key_string,
				  (hash_cmp_f)strcmp,
				  (hash_del_f)free);
  initialization_complete++;
 done:
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

  for (;;)
    {
      struct cerebrod_heartbeat hb;
      char hbbuf[CEREBROD_PACKET_BUFLEN];
      int rv, hblen;

      Pthread_mutex_lock(&listener_fd_lock);
      if ((rv = fd_read_n(listener_fd, hbbuf, hblen)) < 0)
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
	      || listener_fd < 0)
            {
              if (!(listener_fd < 0))
		close(listener_fd);	/* no-wrapper, make best effort */

              /* XXX: Should we re-calc in-addrs? Its even more unlikely
               * that a system administrator will change IPs, subnets, etc.
               * on us.
               */
              if ((listener_fd = _cerebrod_listener_create_and_setup_socket()) < 0)
		{
		  err_debug("cerebrod_listener: error re-initializing socket");

		  /* Wait a bit, so we don't spin */
		  sleep(CEREBROD_REINITIALIZE_WAIT);
		}
              else
                err_debug("cerebrod_listener: success re-initializing socket");
            }
          else
            err_exit("cerebrod_listener: fd_read_n: %s", strerror(errno));
	}
      Pthread_mutex_unlock(&listener_fd_lock);
     
      /* No packet read */
      if (rv < 0)
	continue;

      if ((hblen = cerebrod_heartbeat_unmarshall(&hb, hbbuf, rv)) < 0)
	continue;

      _cerebrod_listener_dump_heartbeat(&hb);
      if (hb.version != CEREBROD_PROTOCOL_VERSION)
	{
	  err_debug("cerebrod_listener: invalid cerebrod packet version read");
	  continue;
	}
    }

  return NULL;			/* NOT REACHED */
}
