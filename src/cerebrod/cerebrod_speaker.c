/*****************************************************************************\
 *  $Id: cerebrod_speaker.c,v 1.8 2005-02-02 01:02:24 achu Exp $
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

#include "cerebrod_speaker.h"
#include "cerebrod_config.h"
#include "cerebrod_heartbeat.h"
#include "error.h"
#include "fd.h"
#include "wrappers.h"

extern struct cerebrod_config conf;
#ifndef NDEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* NDEBUG */

static void
_cerebrod_speaker_initialize(void)
{
  srand(Time(NULL));
}

/* Do not use wrappers in this function, give the daemon additional
 * chances to "survive" an error condition.
 */
static int
_cerebrod_speaker_create_and_setup_socket(void)
{
  struct sockaddr_in heartbeat_destination_addr, speak_from_addr;
  int temp_fd;

  if ((temp_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      err_debug("_cerebrod_speaker_create_and_setup_socket: socket: %s", 
		strerror(errno));
      return -1;
    }

  if (conf.multicast)
    {
      /* XXX: Probably lots of portability problems here */
      struct ip_mreqn imr;
      int optval;

      memcpy(&imr.imr_multiaddr, 
	     &conf.heartbeat_destination_in_addr,
	     sizeof(struct in_addr));
      memcpy(&imr.imr_address, 
	     &conf.speak_from_in_addr,
	     sizeof(struct in_addr));
      imr.imr_ifindex = conf.speak_from_interface_index;
      
      /* Sort of like a multicast-bind */
      if (setsockopt(temp_fd,
		     SOL_IP,
		     IP_MULTICAST_IF,
		     &imr,
		     sizeof(struct ip_mreqn)) < 0)
	{
	  err_debug("_cerebrod_speaker_create_and_setup_socket: setsockopt: %s", 
		    strerror(errno));
	  return -1;
	}

      optval = 1;
      if (setsockopt(temp_fd, 
		     SOL_IP,
		     IP_MULTICAST_LOOP,
		     &optval, 
		     sizeof(optval)) < 0)
	{
	  err_debug("_cerebrod_speaker_create_and_setup_socket: setsockopt: %s", 
		    strerror(errno));
	  return -1;
	}

      optval = conf.speak_ttl;
      if (setsockopt(temp_fd,
		     SOL_IP,
		     IP_MULTICAST_TTL,
		     &optval,
		     sizeof(optval)) < 0)
	{
	  err_debug("_cerebrod_speaker_create_and_setup_socket: setsockopt: %s", 
		    strerror(errno));
	  return -1;
	}
    }

  /* Even if we're multicasting, the port still needs to be bound */
  speak_from_addr.sin_family = AF_INET;
  speak_from_addr.sin_port = htons(conf.heartbeat_source_port);
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
  heartbeat_destination_addr.sin_family = AF_INET;
  heartbeat_destination_addr.sin_port = htons(conf.heartbeat_destination_port);
  memcpy(&heartbeat_destination_addr.sin_addr,
	 &conf.heartbeat_destination_in_addr,
	 sizeof(struct in_addr));
  if (connect(temp_fd, (struct sockaddr *)&heartbeat_destination_addr, sizeof(struct sockaddr_in)) < 0)
    {
      err_debug("_cerebrod_speaker_create_and_setup_socket: connect: %s", 
		strerror(errno));
      return -1;
    }

  return temp_fd;
}

static void
_cerebrod_speaker_dump_heartbeat(struct cerebrod_heartbeat *hb)
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
      fprintf(stderr, "* Sending Heartbeat: %s\n", strbuf);     
      fprintf(stderr, "* -----------------------\n");
      cerebrod_heartbeat_dump(hb);
      fprintf(stderr, "**************************************\n");
      Pthread_mutex_unlock(&debug_output_mutex);
    }
#endif /* NDEBUG */
}

void *
cerebrod_speaker(void *arg)
{
  int fd;

  _cerebrod_speaker_initialize();
  if ((fd = _cerebrod_speaker_create_and_setup_socket()) < 0)
    err_exit("cerebrod_speaker: fd setup failed");

  while (1)
    {
      struct cerebrod_heartbeat hb;
      char hbbuf[CEREBROD_PACKET_BUFLEN];
      int hblen, sleep_time;

      /* Algorithm from srand(3) manpage */
      if (conf.heartbeat_frequency_ranged)
	sleep_time = conf.heartbeat_frequency_min + ((((double)(conf.heartbeat_frequency_max - conf.heartbeat_frequency_min))*rand())/(RAND_MAX+1.0));
      else
	sleep_time = conf.heartbeat_frequency_min;

      /* XXX: Cache rather than re-construct each time? */
      cerebrod_heartbeat_construct(&hb);
  
      hblen = cerebrod_heartbeat_marshall(&hb, hbbuf, CEREBROD_PACKET_BUFLEN);

      _cerebrod_speaker_dump_heartbeat(&hb);
      
      if (fd_write_n(fd, hbbuf, hblen) < 0)
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
	      || fd < 0)
	    {
	      if (!(fd < 0))
		close(fd);	/* no-wrapper, make best effort */

	      /* XXX: Should we re-calc in-addrs? Its even more unlikely
	       * that a system administrator will change IPs, subnets, etc.
	       * on us.
	       */
	      if ((fd = _cerebrod_speaker_create_and_setup_socket()) < 0)
		err_debug("cerebrod_speaker: error re-initializing socket");
	      else
		err_debug("cerebrod_speaker: success re-initializing socket");
	    }
	  else
	    err_exit("cerebrod_speaker: fd_write_n: %s", strerror(errno));
	}

      sleep(sleep_time);
    }

  return NULL;			/* NOT REACHED */
}
