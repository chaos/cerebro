/*****************************************************************************\
 *  $Id: cerebrod_speaker.c,v 1.2 2005-01-10 16:41:14 achu Exp $
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
#include <syslog.h>
#include <assert.h>

#include "cerebrod_speaker.h"
#include "cerebrod_config.h"
#include "cerebrod_heartbeat.h"
#include "error.h"
#include "wrappers.h"

extern struct cerebrod_config conf;

static void
_cerebrod_speaker_initialize(void)
{
  srand(Time(NULL));
}

static int
_cerebrod_speaker_create_and_setup_socket(void)
{
  struct sockaddr_in speak_to_addr, speak_from_addr;
  int temp_fd;

  temp_fd = Socket(AF_INET, SOCK_DGRAM, 0);

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

      optval = 1;
      Setsockopt(temp_fd, 
		 SOL_IP,
		 IP_MULTICAST_LOOP,
		 &optval, 
		 sizeof(optval));
    }

  /* Even if we're multicasting, the port still needs to be bound */
  speak_from_addr.sin_family = AF_INET;
  speak_from_addr.sin_port = htons(conf.speak_from_port);
  memcpy(&speak_from_addr.sin_addr,
	 &conf.speak_from_in_addr,
	 sizeof(struct in_addr));
  Bind(temp_fd, (struct sockaddr *)&speak_from_addr, sizeof(struct sockaddr_in));

  /* Connect to the speak to address */
  speak_to_addr.sin_family = AF_INET;
  speak_from_addr.sin_port = htons(conf.listen_port);
  memcpy(&speak_to_addr.sin_addr,
	 &conf.speak_to_in_addr,
	 sizeof(struct in_addr));
  Connect(temp_fd, (struct sockaddr *)&speak_to_addr, sizeof(struct sockaddr_in));

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

      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Sending Heartbeat: %s\n", strbuf);     
      fprintf(stderr, "* -----------------------\n");
      cerebrod_heartbeat_dump(hb);
      fprintf(stderr, "**************************************\n");
    }
#endif /* NDEBUG */
}

static void
_cerebrod_speaker_send_heartbeat(int fd)
{
  struct cerebrod_heartbeat hb;
  char hbbuf[CEREBROD_PACKET_BUFLEN];
  int hblen;

  /* XXX: Cache rather than re-construct each time? */
  cerebrod_heartbeat_construct(&hb);
  
  hblen = cerebrod_heartbeat_marshall(&hb, hbbuf, CEREBROD_PACKET_BUFLEN);

  _cerebrod_speaker_dump_heartbeat(&hb);
  Write(fd, hbbuf, hblen);
}

void
cerebrod_speaker(void *arg)
{
  int fd;

  _cerebrod_speaker_initialize();
  fd = _cerebrod_speaker_create_and_setup_socket();

  while (1)
    {
      int sleep_time;

      /* Algorithm from srand(3) manpage */
      sleep_time = conf.heartbeat_frequency_min + ((((double)(conf.heartbeat_frequency_max - conf.heartbeat_frequency_min))*rand())/(RAND_MAX+1.0));

      _cerebrod_speaker_send_heartbeat(fd);
      sleep(sleep_time);

    }
}
