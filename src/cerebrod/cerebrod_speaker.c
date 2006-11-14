/*****************************************************************************\
 *  $Id: cerebrod_speaker.c,v 1.91 2006-11-14 18:58:26 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2005 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <http://www.llnl.gov/linux/cerebro/>.
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
 *  with Genders; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
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

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"

#include "cerebro/cerebrod_heartbeat_protocol.h"

#include "cerebrod_config.h"
#include "cerebrod_heartbeat.h"
#include "cerebrod_speaker.h"
#include "cerebrod_speaker_data.h"
#include "cerebrod_util.h"

#include "debug.h"
#include "metric_module.h"
#include "data_util.h"
#include "wrappers.h"

extern struct cerebrod_config conf;
#if CEREBRO_DEBUG
#if !WITH_CEREBROD_NO_THREADS
extern pthread_mutex_t debug_output_mutex;
#endif /* !WITH_CEREBROD_NO_THREADS */
#endif /* CEREBRO_DEBUG */

/*
 * cerebrod_nodename
 *
 * cached system nodename
 */
static char cerebrod_nodename[CEREBRO_MAX_NODENAME_LEN+1];

/* 
 * next_send_times
 *
 * Stores information on the next time to send information
 */
List next_send_times = NULL;

/*
 * Speaker Data
 */
extern List metric_list;
extern int metric_list_size;
extern pthread_mutex_t metric_list_lock;

/* 
 * _speaker_socket_create
 *
 * Create and setup a speaker socket.  Do not use wrappers in this
 * function.  We want to give the daemon additional chances to
 * "survive" an error condition.
 *
 * In this socket setup function, 'num' is used as the port to setup
 * the socket with.
 * 
 * Returns file descriptor on success, -1 on error
 */
static int
_speaker_socket_create(int num)
{
  struct sockaddr_in addr;
  int fd, optval = 1;

  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      CEREBRO_DBG(("socket: %s", strerror(errno)));
      goto cleanup;
    }

  if (conf.destination_ip_is_multicast)
    {
      /* XXX: Probably lots of portability problems here */
      struct ip_mreqn imr;
      unsigned int optlen;
      int optval;

      memset(&imr, '\0', sizeof(struct ip_mreqn));
      memcpy(&imr.imr_multiaddr, 
	     &conf.heartbeat_destination_ip_in_addr,
	     sizeof(struct in_addr));
      memcpy(&imr.imr_address, 
	     &conf.heartbeat_source_network_interface_in_addr,
	     sizeof(struct in_addr));
      imr.imr_ifindex = conf.heartbeat_source_network_interface_index;
      
      optlen = sizeof(struct ip_mreqn);
      if (setsockopt(fd, SOL_IP, IP_MULTICAST_IF, &imr, optlen) < 0)
	{
	  CEREBRO_DBG(("setsockopt: %s", strerror(errno)));
          goto cleanup;
	}

      optval = 1;
      optlen = sizeof(optval);
      if (setsockopt(fd, SOL_IP, IP_MULTICAST_LOOP, &optval, optlen) < 0)
	{
	  CEREBRO_DBG(("setsockopt: %s", strerror(errno)));
          goto cleanup;
	}

      optval = conf.heartbeat_ttl;
      optlen = sizeof(optval);
      if (setsockopt(fd, SOL_IP, IP_MULTICAST_TTL, &optval, optlen) < 0)
	{
	  CEREBRO_DBG(("setsockopt: %s", strerror(errno)));
          goto cleanup;
	}
    }

  /* For quick start/restart */
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) < 0)
    {
      CEREBRO_DBG(("setsockopt: %s", strerror(errno)));
      goto cleanup;
    }

  memset(&addr, '\0', sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(num);
  memcpy(&addr.sin_addr,
	 &conf.heartbeat_source_network_interface_in_addr,
	 sizeof(struct in_addr));
  if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0) 
    {
      CEREBRO_DBG(("bind: %s", strerror(errno)));
      goto cleanup;
    }

  return fd;

 cleanup:
  close(fd);
  return -1;
}

/* 
 * _next_send_time_compare
 *
 * Compare function for cerebrod_next_send_time structures.
 */
static int
_next_send_time_compare(void *x, void *y)
{
  struct cerebrod_next_send_time *a;
  struct cerebrod_next_send_time *b;
  
  assert(x);
  assert(y);

  a = (struct cerebrod_next_send_time *)x;
  b = (struct cerebrod_next_send_time *)y;

  if (a->next_send_time < b->next_send_time)
    return -1;
  if (b->next_send_time > a->next_send_time)
    return 1;
  return 0;
}

/* 
 * _speaker_initialize
 *
 * perform speaker initialization
 */
static void
_speaker_initialize(void)
{
  struct cerebrod_next_send_time *nst;
  unsigned int seed;
  struct timeval tv;
  int i, len;

  /* Cache Nodename */
  memset(cerebrod_nodename, '\0', CEREBRO_MAX_NODENAME_LEN+1);
  Gethostname(cerebrod_nodename, CEREBRO_MAX_NODENAME_LEN);

  /* Seed random number generator */
  Gettimeofday(&tv, NULL);
  seed = tv.tv_sec;

  /* If each cluster node is re-booted at the same time, each cluster
   * node could be seeded with the same random seed.  In order to
   * avoid this, we'll add the cluster nodename to the seed to give
   * every cluster node atleast a constant different offset.
   */
  len = strlen(cerebrod_nodename);
  for (i = 0; i < len; i++)
    seed += (int)cerebrod_nodename[i];

  srand(seed);

  cerebrod_speaker_data_initialize();

  next_send_times = List_create((ListDelF)_Free);
  /*  
   * We will always atleast send a heartbeat, so initialize
   * the next_send_times list with this information.
   * 
   * Initialize the next_send_time to send a heartbeat immediately.
   */
  nst = (struct cerebrod_next_send_time *)Malloc(sizeof(struct cerebrod_next_send_time));
  nst->next_send_type = CEREBROD_SPEAKER_NEXT_SEND_TYPE_HEARTBEAT;
  nst->next_send_time = 0;
  List_append(next_send_times, nst);
  nst = NULL;

  if (metric_list)
    {
      struct cerebrod_speaker_metric_info *metric_info;

      ListIterator itr;
      Pthread_mutex_lock(&metric_list_lock);
      itr = List_iterator_create(metric_list);
      while ((metric_info = list_next(itr)))
        {
          /* Must check for origin, there is a theoretically possible
           * time slice in which a non-module metric could be added
           * via the metric controller.
           */
          if (metric_info->metric_origin == CEREBROD_METRIC_SPEAKER_ORIGIN_MODULE)
            {
              if (metric_info->metric_flags & CEREBRO_METRIC_MODULE_FLAGS_SEND_ON_PERIOD)
                {
                  nst = (struct cerebrod_next_send_time *)Malloc(sizeof(struct cerebrod_next_send_time));
                  nst->next_send_type = CEREBROD_SPEAKER_NEXT_SEND_TYPE_MODULE;
                  nst->next_send_time = tv.tv_sec + metric_info->metric_period;
                  nst->metric_period = metric_info->metric_period;
                  nst->index = metric_info->index;
                  List_append(next_send_times, nst);
                  nst = NULL;
                }
            }
        }
      Pthread_mutex_unlock(&metric_list_lock);
    }

  List_sort(next_send_times, (ListCmpF)_next_send_time_compare);
}

/*
 * _cerebrod_heartbeat_create
 *
 * construct a heartbeat packet
 */
static struct cerebrod_heartbeat *
_cerebrod_heartbeat_create(struct cerebrod_next_send_time *nst,
                           unsigned int *heartbeat_len)
{
  struct cerebrod_heartbeat *hb = NULL;

  assert(nst && heartbeat_len);

  *heartbeat_len = 0;

  hb = Malloc(sizeof(struct cerebrod_heartbeat));

  hb->version = CEREBROD_HEARTBEAT_PROTOCOL_VERSION;
  memcpy(hb->nodename, cerebrod_nodename, CEREBRO_MAX_NODENAME_LEN);
  *heartbeat_len += CEREBROD_HEARTBEAT_HEADER_LEN;

  if (nst->next_send_type & CEREBROD_SPEAKER_NEXT_SEND_TYPE_HEARTBEAT)
    cerebrod_speaker_data_get_heartbeat_metric_data(hb, heartbeat_len);
  else if (nst->next_send_type & CEREBROD_SPEAKER_NEXT_SEND_TYPE_MODULE)
    cerebrod_speaker_data_get_module_metric_data(hb, 
                                                 heartbeat_len, 
                                                 nst->index);

  return hb;
}

/*
 * _heartbeat_marshall
 *
 * marshall contents of a heartbeat packet.
 *
 * Returns length written to buffer on success, -1 on error
 */
int
_heartbeat_marshall(struct cerebrod_heartbeat *hb, 
                    char *buf, 
                    unsigned int buflen)
{
  char *bufPtr;
  int i, bufPtrlen, c = 0;
 
  assert(hb && buf && buflen >= CEREBROD_HEARTBEAT_HEADER_LEN);
  
  bufPtr = hb->nodename;
  bufPtrlen = sizeof(hb->nodename);
  memset(buf, '\0', buflen);
  c += Marshall_int32(hb->version, buf + c, buflen - c);
  c += Marshall_buffer(bufPtr, bufPtrlen, buf + c, buflen - c);
  c += Marshall_u_int32(hb->metrics_len, buf + c, buflen - c);
  
  if (!hb->metrics_len)
    return c;

  for (i = 0; i < hb->metrics_len; i++)
    {
      char *mname;
      u_int32_t mtype, mlen;
      void *mvalue;
      int n, mnamelen;

      mname = hb->metrics[i]->metric_name;
      mnamelen = sizeof(hb->metrics[i]->metric_name);
      mtype = hb->metrics[i]->metric_value_type;
      mlen = hb->metrics[i]->metric_value_len;
      mvalue = hb->metrics[i]->metric_value;

      c += Marshall_buffer(mname, mnamelen, buf + c, buflen - c);

      if (mtype == CEREBRO_DATA_VALUE_TYPE_NONE && mlen)
        {
          CEREBRO_DBG(("adjusting metric len to 0"));
          mlen = 0;
        }

      if (mtype == CEREBRO_DATA_VALUE_TYPE_STRING && !mlen)
        {
          CEREBRO_DBG(("adjusting metric type to none"));
          mtype = CEREBRO_DATA_VALUE_TYPE_NONE;
        }

      if ((n = marshall_data(mtype, 
                             mlen,
                             mvalue,
                             buf + c,
                             buflen - c,
                             NULL)) < 0)
        goto cleanup;
      c += n;
    }
  
  return c;

 cleanup:
  return -1;
}

/* 
 * _cerebrod_heartbeat_dump
 *
 * Dump contents of heartbeat packet
 */
static void
_cerebrod_heartbeat_dump(struct cerebrod_heartbeat *hb)
{
#if CEREBRO_DEBUG
  assert(hb);

  if (!(conf.debug && conf.speak_debug))
    return;

#if !WITH_CEREBROD_NO_THREADS
  Pthread_mutex_lock(&debug_output_mutex);
#endif /* !WITH_CEREBROD_NO_THREADS */
  fprintf(stderr, "**************************************\n");
  fprintf(stderr, "* Sending Heartbeat\n");     
  fprintf(stderr, "* -----------------------\n");
  cerebrod_heartbeat_dump(hb);
  fprintf(stderr, "**************************************\n");
#if !WITH_CEREBROD_NO_THREADS
  Pthread_mutex_unlock(&debug_output_mutex);
#endif /* !WITH_CEREBROD_NO_THREADS */
#endif /* CEREBRO_DEBUG */
}

void *
cerebrod_speaker(void *arg)
{
  int speaker_fd;

  _speaker_initialize();

  if ((speaker_fd = _speaker_socket_create(conf.heartbeat_source_port)) < 0)
    CEREBRO_EXIT(("speaker fd setup failed"));

  while (1)
    {
      struct cerebrod_next_send_time *nst;
      int sleep_time;
      ListIterator itr;
      struct timeval tv;

      Gettimeofday(&tv, NULL);

      itr = List_iterator_create(next_send_times);
      while ((nst = list_next(itr)))
        {
          if (nst->next_send_time < tv.tv_sec)
            {
              struct cerebrod_heartbeat* hb;
              unsigned int buflen;
              char *buf = NULL;
              struct sockaddr *addr;
              struct sockaddr_in hbaddr;
              int rv, hblen;
              unsigned int addrlen;

              hb = _cerebrod_heartbeat_create(nst, &buflen);

              if (buflen >= CEREBRO_MAX_PACKET_LEN)
                {
                  CEREBRO_DBG(("heartbeat exceeds maximum size: packet dropped"));
                  goto end_loop;
                }
              
              buf = Malloc(buflen + 1);
              
              if ((hblen = _heartbeat_marshall(hb, buf, buflen)) < 0)
                goto end_loop;

              _cerebrod_heartbeat_dump(hb);
      
              memset(&hbaddr, '\0', sizeof(struct sockaddr_in));
              hbaddr.sin_family = AF_INET;
              hbaddr.sin_port = htons(conf.heartbeat_destination_port);
              memcpy(&hbaddr.sin_addr, 
                     &conf.heartbeat_destination_ip_in_addr, 
                     sizeof(struct in_addr));

              addr = (struct sockaddr *)&hbaddr;
              addrlen = sizeof(struct sockaddr_in);
              if ((rv = sendto(speaker_fd, buf, hblen, 0, addr, addrlen)) != hblen)
                {
                  if (rv < 0)
                    speaker_fd = cerebrod_reinit_socket(speaker_fd, 
                                                        conf.heartbeat_source_port,
                                                        _speaker_socket_create, 
                                                        "speaker: sendto");
                  else
                    CEREBRO_DBG(("sendto: invalid bytes sent"));
                }
              
            end_loop:
              cerebrod_heartbeat_destroy(hb);
              if (nst->next_send_type & CEREBROD_SPEAKER_NEXT_SEND_TYPE_HEARTBEAT)
                {
                  int t;

                  /* Algorithm from srand(3) manpage */
                  if (conf.heartbeat_frequency_ranged)
                    t = conf.heartbeat_frequency_min + ((((double)(conf.heartbeat_frequency_max - conf.heartbeat_frequency_min))*rand())/(RAND_MAX+1.0));
                  else
                    t = conf.heartbeat_frequency_min;

                  nst->next_send_time = tv.tv_sec + t;
                }
              if (nst->next_send_type & CEREBROD_SPEAKER_NEXT_SEND_TYPE_MODULE)
                nst->next_send_time = tv.tv_sec + nst->metric_period;
              if (buf)
                Free(buf);
            }
          else
            break;
        }
      
      List_sort(next_send_times, (ListCmpF)_next_send_time_compare);

      nst = List_peek(next_send_times);
      sleep_time = nst->next_send_time - tv.tv_sec;
      sleep(sleep_time);
    }

  return NULL;			/* NOT REACHED */
}

int
cerebrod_send_heartbeat(struct cerebrod_heartbeat *hb)
{
  int i, rv, hblen, buflen = 0;
  unsigned int addrlen;
  char *buf = NULL;
  struct sockaddr_in hbaddr;
  int fd = -1;

  if (!hb)
    {
      errno = EINVAL;
      return -1;
    }

  /* To avoid issues with the primary speaker "thread", we create
   * another fd
   */

  fd = _speaker_socket_create(0);

  buflen = CEREBROD_HEARTBEAT_HEADER_LEN;
  for (i = 0; i < hb->metrics_len; i++)
    {
      if (!hb->metrics[i])
	{
	  CEREBRO_DBG(("null metrics pointer"));
	  errno = EINVAL;
	  goto cleanup;
	}
      buflen += CEREBROD_HEARTBEAT_METRIC_HEADER_LEN;
      buflen += hb->metrics[i]->metric_value_len;
    }

  if (buflen >= CEREBRO_MAX_PACKET_LEN)
    {
      CEREBRO_DBG(("heartbeat exceeds maximum size: packet dropped"));
      goto cleanup;
    }

  buf = Malloc(buflen + 1);

  if ((hblen = _heartbeat_marshall(hb, buf, buflen)) < 0)
    {
      CEREBRO_DBG(("_heartbeat_marshall"));
      goto cleanup;
    }

  _cerebrod_heartbeat_dump(hb);
      
  memset(&hbaddr, '\0', sizeof(struct sockaddr_in));
  hbaddr.sin_family = AF_INET;
  hbaddr.sin_port = htons(conf.heartbeat_destination_port);
  memcpy(&hbaddr.sin_addr, 
	 &conf.heartbeat_destination_ip_in_addr, 
	 sizeof(struct in_addr));

  addrlen = sizeof(struct sockaddr_in);
  if ((rv = sendto(fd, 
                   buf, 
                   hblen, 
                   0, 
                   (struct sockaddr *)&hbaddr, 
                   addrlen)) != hblen)
    {
      CEREBRO_DBG(("sendto: invalid bytes sent"));
      goto cleanup;
    }

 cleanup:
  if (buf)
    Free(buf); 
  close(fd);
  return 0;
}
