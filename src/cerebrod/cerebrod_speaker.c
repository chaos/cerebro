/*****************************************************************************\
 *  $Id: cerebrod_speaker.c,v 1.19 2005-03-30 05:41:45 achu Exp $
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

#include "cerebro_defs.h"
#include "cerebrod_heartbeat_protocol.h"

#include "cerebrod_speaker.h"
#include "cerebrod_config.h"
#include "cerebrod_data.h"
#include "cerebrod_error.h"
#include "cerebrod_heartbeat.h"
#include "fd.h"
#include "wrappers.h"

extern struct cerebrod_config conf;
#ifndef NDEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* NDEBUG */

/* 
 * _cerebrod_speaker_create_and_setup_socket
 *
 * Create and setup the speaker socket.  Do not use wrappers in this
 * function.  We want to give the daemon additional chances to
 * "survive" an error condition.
 *
 * Returns file descriptor on success, -1 on error
 */
static int
_cerebrod_speaker_create_and_setup_socket(void)
{
  struct sockaddr_in heartbeat_addr;
  int temp_fd;

  if ((temp_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      cerebrod_err_debug("%s(%s:%d): socket: %s", 
			 __FILE__, __FUNCTION__, __LINE__,
                         strerror(errno));
      return -1;
    }

  if (conf.multicast)
    {
      /* XXX: Probably lots of portability problems here */
      struct ip_mreqn imr;
      int optval;

      memset(&imr, '\0', sizeof(struct ip_mreqn));
      memcpy(&imr.imr_multiaddr, 
	     &conf.heartbeat_destination_ip_in_addr,
	     sizeof(struct in_addr));
      memcpy(&imr.imr_address, 
	     &conf.heartbeat_network_interface_in_addr,
	     sizeof(struct in_addr));
      imr.imr_ifindex = conf.heartbeat_interface_index;
      
      if (setsockopt(temp_fd,
		     SOL_IP,
		     IP_MULTICAST_IF,
		     &imr,
		     sizeof(struct ip_mreqn)) < 0)
	{
	  cerebrod_err_debug("%s(%s:%d): setsockopt: %s", 
			     __FILE__, __FUNCTION__, __LINE__,
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
	  cerebrod_err_debug("%s(%s:%d): setsockopt: %s", 
			     __FILE__, __FUNCTION__, __LINE__,
                             strerror(errno));
	  return -1;
	}

      optval = conf.heartbeat_ttl;
      if (setsockopt(temp_fd,
		     SOL_IP,
		     IP_MULTICAST_TTL,
		     &optval,
		     sizeof(optval)) < 0)
	{
	  cerebrod_err_debug("%s(%s:%d): setsockopt: %s", 
			     __FILE__, __FUNCTION__, __LINE__,
                             strerror(errno));
	  return -1;
	}
    }

  memset(&heartbeat_addr, '\0', sizeof(struct sockaddr_in));
  heartbeat_addr.sin_family = AF_INET;
  heartbeat_addr.sin_port = htons(conf.heartbeat_source_port);
  memcpy(&heartbeat_addr.sin_addr,
	 &conf.heartbeat_network_interface_in_addr,
	 sizeof(struct in_addr));
  if (bind(temp_fd, 
	   (struct sockaddr *)&heartbeat_addr, 
	   sizeof(struct sockaddr_in)) < 0) 
    {
      cerebrod_err_debug("%s(%s:%d): bind: %s", 
			 __FILE__, __FUNCTION__, __LINE__,
                         strerror(errno));
      return -1;
    }

  return temp_fd;
}

/* 
 * _cerebrod_speaker_initialize
 *
 * perform speaker initialization
 */
static void
_cerebrod_speaker_initialize(void)
{
  char hostname[CEREBRO_MAXHOSTNAMELEN+1];
  unsigned int seed;;
  int i, len;

  seed = Time(NULL);

  /* If an entire cluster is re-booted at the same time, each cluster
   * node could potentially be seeded with the same time.  In order to
   * avoid this, we'll add the cluster hostname to the seed to give
   * every cluster node a constant different offset.
   */
  memset(hostname, '\0', CEREBRO_MAXHOSTNAMELEN+1);
  cerebrod_get_hostname(hostname, CEREBRO_MAXHOSTNAMELEN+1);
  
  len = strlen(hostname);
  for (i = 0; i < len; i++)
    seed += (int)hostname[i];

  srand(seed);
}

/* 
 * _cerebrod_speaker_dump_heartbeat
 *
 * Dump contents of heartbeat packet
 */
static void
_cerebrod_speaker_dump_heartbeat(struct cerebrod_heartbeat *hb)
{
#ifndef NDEBUG
  assert(hb);

  if (conf.debug && conf.speak_debug)
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
    cerebrod_err_exit("%s(%s:%d): fd setup failed",
		      __FILE__, __FUNCTION__, __LINE__);

  while (1)
    {
      struct sockaddr_in heartbeat_destination_addr;
      struct cerebrod_heartbeat hb;
      char hbbuf[CEREBROD_PACKET_BUFLEN];
      int rv, hblen, sleep_time;

      /* Algorithm from srand(3) manpage */
      if (conf.heartbeat_frequency_ranged)
	sleep_time = conf.heartbeat_frequency_min + ((((double)(conf.heartbeat_frequency_max - conf.heartbeat_frequency_min))*rand())/(RAND_MAX+1.0));
      else
	sleep_time = conf.heartbeat_frequency_min;

      cerebrod_heartbeat_construct(&hb);
  
      hblen = cerebrod_heartbeat_marshall(&hb, hbbuf, CEREBROD_PACKET_BUFLEN);

      _cerebrod_speaker_dump_heartbeat(&hb);
      
      memset(&heartbeat_destination_addr, '\0', sizeof(struct sockaddr_in));
      heartbeat_destination_addr.sin_family = AF_INET;
      heartbeat_destination_addr.sin_port = htons(conf.heartbeat_destination_port);
      memcpy(&heartbeat_destination_addr.sin_addr,
             &conf.heartbeat_destination_ip_in_addr,
             sizeof(struct in_addr));

      if ((rv = sendto(fd, 
                       hbbuf, 
                       hblen, 
                       0, 
                       (struct sockaddr *)&heartbeat_destination_addr,
                       sizeof(struct sockaddr_in))) != hblen)
        {
          if (rv < 0)
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
                  
                  if ((fd = _cerebrod_speaker_create_and_setup_socket()) < 0)
                    cerebrod_err_debug("%s(%s:%d): error re-initializing socket",
				       __FILE__, __FUNCTION__, __LINE__);
                  else
                    cerebrod_err_debug("%s(%s:%d): success re-initializing "
				       "socket",
				       __FILE__, __FUNCTION__, __LINE__);
                }
              else if (errno == EINTR)
                cerebrod_err_debug("%s(%s:%d): sendto: %s", 
				   __FILE__, __FUNCTION__, __LINE__,
				   strerror(errno));
              else
                cerebrod_err_exit("%s(%s:%d): sendto: %s", 
				  __FILE__, __FUNCTION__, __LINE__,
				  strerror(errno));
            }
          else
            cerebrod_err_debug("%s(%s:%d): sendto: invalid bytes sent: %d", 
			       __FILE__, __FUNCTION__, __LINE__, rv);
        }
      sleep(sleep_time);
    }

  return NULL;			/* NOT REACHED */
}
