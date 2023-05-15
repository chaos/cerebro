/*****************************************************************************\
 *  $Id: cerebrod_speaker.c,v 1.115 2010-02-02 01:01:20 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2018 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2005-2007 The Regents of the University of California.
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
 *  with Cerebro.  If not, see <http://www.gnu.org/licenses/>.
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

#include "cerebro/cerebrod_message_protocol.h"

#include "cerebrod_config.h"
#include "cerebrod_debug.h"
#include "cerebrod_message.h"
#include "cerebrod_speaker.h"
#include "cerebrod_speaker_data.h"
#include "cerebrod_util.h"

#include "debug.h"
#include "metric_module.h"
#include "data_util.h"
#include "wrappers.h"

extern struct cerebrod_config conf;
#if !WITH_CEREBROD_NO_THREADS
extern pthread_mutex_t debug_output_mutex;
#endif /* !WITH_CEREBROD_NO_THREADS */

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
#if !WITH_CEREBROD_NO_THREADS
extern pthread_mutex_t metric_list_lock;
#endif /* !WITH_CEREBROD_NO_THREADS */

/*
 * speaker_fds
 * speaker_fds_lock
 *
 * speaker file descriptor and lock to protect concurrent access
 */
int speaker_fds[CEREBRO_CONFIG_SPEAK_MESSAGE_CONFIG_MAX];
unsigned int speaker_fds_len = 0;
pthread_mutex_t speaker_fds_lock = PTHREAD_MUTEX_INITIALIZER;

/* 
 * _speaker_setup_socket
 *
 * Create and setup a speaker socket.  Do not use wrappers in this
 * function.  We want to give the daemon additional chances to
 * "survive" an error condition.
 *
 * In this socket setup function, 'num' is used as the message config
 * and file descriptor index.
 * 
 * Returns file descriptor on success, -1 on error
 */
static int
_speaker_setup_socket(int num)
{
  struct sockaddr_in addr;
  unsigned int optlen;
  int fd, optval = 1;

  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      CEREBROD_ERR(("socket: %s", strerror(errno)));
      goto cleanup;
    }

  if (conf.speak_message_config[num].ip_is_multicast)
    {
      /* XXX: Probably lots of portability problems here */
      struct ip_mreqn imr;

      memset(&imr, '\0', sizeof(struct ip_mreqn));
      memcpy(&imr.imr_multiaddr, 
	     &conf.speak_message_config[num].ip_in_addr,
	     sizeof(struct in_addr));
      memcpy(&imr.imr_address, 
	     &conf.speak_message_config[num].network_interface_in_addr,
	     sizeof(struct in_addr));
      imr.imr_ifindex = conf.speak_message_config[num].network_interface_index;
      
      optlen = sizeof(struct ip_mreqn);
      if (setsockopt(fd, SOL_IP, IP_MULTICAST_IF, &imr, optlen) < 0)
	{
	  CEREBROD_ERR(("setsockopt: %s", strerror(errno)));
          goto cleanup;
	}

      optval = 1;
      optlen = sizeof(optval);
      if (setsockopt(fd, SOL_IP, IP_MULTICAST_LOOP, &optval, optlen) < 0)
	{
	  CEREBROD_ERR(("setsockopt: %s", strerror(errno)));
          goto cleanup;
	}

      optval = conf.speak_message_ttl;
      optlen = sizeof(optval);
      if (setsockopt(fd, SOL_IP, IP_MULTICAST_TTL, &optval, optlen) < 0)
	{
	  CEREBROD_ERR(("setsockopt: %s", strerror(errno)));
          goto cleanup;
	}
    }

  /* For quick start/restart */
  optval = 1;
  optlen = sizeof(optval);
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, optlen) < 0)
    {
      CEREBROD_ERR(("setsockopt: %s", strerror(errno)));
      goto cleanup;
    }

  memset(&addr, '\0', sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(conf.speak_message_config[num].source_port);
  memcpy(&addr.sin_addr,
	 &conf.speak_message_config[num].network_interface_in_addr,
	 sizeof(struct in_addr));
  if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0) 
    {
      CEREBROD_ERR(("bind: %s", strerror(errno)));
      goto cleanup;
    }

  return fd;

 cleanup:
  /* ignore potential error, we're in the error path already */
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
#if CEREBRO_DEBUG
  if (!conf.alternate_hostname)
    Gethostname(cerebrod_nodename, CEREBRO_MAX_NODENAME_LEN);
  else
    Strncpy(cerebrod_nodename, conf.alternate_hostname, CEREBRO_MAX_NODENAME_LEN+1);
#else /* !CEREBRO_DEBUG */
  Gethostname(cerebrod_nodename, CEREBRO_MAX_NODENAME_LEN);
#endif /* CEREBRO_DEBUG */

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

  /* achu: setup speaker fds first, since later initialization could
   * launch metric modules that want to send data, which require the
   * fds to be setup first.
   */
  Pthread_mutex_lock(&speaker_fds_lock);
  for (i = 0; i < conf.speak_message_config_len; i++)
    {
      if ((speaker_fds[i] = _speaker_setup_socket(i)) < 0)
        CEREBROD_EXIT(("speaker fd setup failed"));
    }
  speaker_fds_len = conf.speak_message_config_len;
  Pthread_mutex_unlock(&speaker_fds_lock);

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
#if !WITH_CEREBROD_NO_THREADS
      Pthread_mutex_lock(&metric_list_lock);
#endif /* !WITH_CEREBROD_NO_THREADS */
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
#if !WITH_CEREBROD_NO_THREADS
      Pthread_mutex_unlock(&metric_list_lock);
#endif /* !WITH_CEREBROD_NO_THREADS */
    }

  List_sort(next_send_times, (ListCmpF)_next_send_time_compare);
}

/*
 * _cerebrod_message_create
 *
 * construct a message packet
 */
static struct cerebrod_message *
_cerebrod_message_create(struct cerebrod_next_send_time *nst,
                         unsigned int *message_len,
                         int *more_data_to_send)
{
  struct cerebrod_message *msg = NULL;

  assert(nst && message_len && more_data_to_send);

  *message_len = 0;

  msg = Malloc(sizeof(struct cerebrod_message));

  msg->version = CEREBROD_MESSAGE_PROTOCOL_VERSION;
  memcpy(msg->nodename, cerebrod_nodename, CEREBRO_MAX_NODENAME_LEN);
  *message_len += CEREBROD_MESSAGE_HEADER_LEN;

  if (nst->next_send_type & CEREBROD_SPEAKER_NEXT_SEND_TYPE_HEARTBEAT)
    cerebrod_speaker_data_get_heartbeat_metric_data(msg, 
                                                    message_len, 
                                                    more_data_to_send);
  else if (nst->next_send_type & CEREBROD_SPEAKER_NEXT_SEND_TYPE_MODULE)
    cerebrod_speaker_data_get_module_metric_data(msg, 
                                                 message_len, 
                                                 nst->index,
                                                 more_data_to_send);

  return msg;
}

/*
 * _message_marshall
 *
 * marshall contents of a message packet.
 *
 * Returns length written to buffer on success, -1 on error
 */
int
_message_marshall(struct cerebrod_message *msg, 
                  char *buf, 
                  unsigned int buflen)
{
  char *bufPtr;
  int i, bufPtrlen, c = 0;
 
  assert(msg && buf && buflen >= CEREBROD_MESSAGE_HEADER_LEN);
  
  bufPtr = msg->nodename;
  bufPtrlen = sizeof(msg->nodename);
  memset(buf, '\0', buflen);
  c += Marshall_int32(msg->version, buf + c, buflen - c);
  c += Marshall_buffer(bufPtr, bufPtrlen, buf + c, buflen - c);
  c += Marshall_u_int32(msg->metrics_len, buf + c, buflen - c);
  
  if (!msg->metrics_len)
    return c;

  for (i = 0; i < msg->metrics_len; i++)
    {
      char *mname;
      u_int32_t mtype, mlen;
      void *mvalue;
      int n, mnamelen;

      mname = msg->metrics[i]->metric_name;
      mnamelen = sizeof(msg->metrics[i]->metric_name);
      mtype = msg->metrics[i]->metric_value_type;
      mlen = msg->metrics[i]->metric_value_len;
      mvalue = msg->metrics[i]->metric_value;

      c += Marshall_buffer(mname, mnamelen, buf + c, buflen - c);

      if (mtype == CEREBRO_DATA_VALUE_TYPE_NONE && mlen)
        {
          CEREBROD_DBG(("adjusting metric len to 0"));
          mlen = 0;
        }

      if (mtype == CEREBRO_DATA_VALUE_TYPE_STRING && !mlen)
        {
          CEREBROD_DBG(("adjusting metric type to none"));
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
 * _cerebrod_message_dump
 *
 * Dump contents of message packet
 */
static void
_cerebrod_message_dump(struct cerebrod_message *msg)
{
  assert(msg);

  if (!(conf.debug && conf.speak_debug))
    return;

#if !WITH_CEREBROD_NO_THREADS
  Pthread_mutex_lock(&debug_output_mutex);
#endif /* !WITH_CEREBROD_NO_THREADS */
  fprintf(stderr, "**************************************\n");
  fprintf(stderr, "* Sending Message\n");     
  fprintf(stderr, "* -----------------------\n");
  cerebrod_message_dump(msg);
  fprintf(stderr, "**************************************\n");
#if !WITH_CEREBROD_NO_THREADS
  Pthread_mutex_unlock(&debug_output_mutex);
#endif /* !WITH_CEREBROD_NO_THREADS */
}

static void
_cerebrod_message_send(struct cerebrod_message* msg, unsigned int msglen)
{
  int buflen;
  char *buf = NULL;
  int i, rv;

  assert(msg);
  assert(msglen);               /* atleast the header must be there */
  
  buf = Malloc(msglen + 1);
                  
  if ((buflen = _message_marshall(msg, buf, msglen)) < 0)
    {
      Free(buf);
      return;
    }
  
  Pthread_mutex_lock(&speaker_fds_lock);
  for (i = 0; i < speaker_fds_len; i++)
    {
      struct sockaddr *addr;
      struct sockaddr_in msgaddr;
      unsigned int addrlen;
      
      memset(&msgaddr, '\0', sizeof(struct sockaddr_in));
      msgaddr.sin_family = AF_INET;
      msgaddr.sin_port = htons(conf.speak_message_config[i].destination_port);
      memcpy(&msgaddr.sin_addr, 
             &conf.speak_message_config[i].ip_in_addr, 
             sizeof(struct in_addr));
      
      addr = (struct sockaddr *)&msgaddr;
      addrlen = sizeof(struct sockaddr_in);
      if ((rv = sendto(speaker_fds[i],
                       buf, 
                       buflen,
                       0,
                       addr, 
                       addrlen)) != msglen)
        {
          if (rv < 0)
            speaker_fds[i] = cerebrod_reinit_socket(speaker_fds[i], 
                                                    i,
                                                    _speaker_setup_socket, 
                                                    "speaker: sendto");
          else
            CEREBROD_ERR(("sendto: invalid bytes sent"));
        }
    }
  Pthread_mutex_unlock(&speaker_fds_lock);
  Free(buf);
}

void *
cerebrod_speaker(void *arg)
{
  _speaker_initialize();

  while (1)
    {
      struct cerebrod_next_send_time *nst;
      int sleep_time;
      ListIterator itr;
      struct timeval tv;
      time_t now;

      if (conf.gettimeofday_workaround)
	Gettimeofday_workaround(&tv, NULL);
      else
	Gettimeofday(&tv, NULL);
      now = tv.tv_sec;

      /* Note: After initial setup, we are the only thread that uses this
       * list/iterator.  So no need for pthread locking.
       */
      itr = List_iterator_create(next_send_times);
      while ((nst = list_next(itr)))
        {
          if (nst->next_send_time <= now)
            {
              int more_data_to_send = 1; /* initialize to 1 for first time through */

              while (more_data_to_send)
                {
                  struct cerebrod_message* msg;
                  unsigned int msglen;

                  more_data_to_send = 0;

                  /* There is a potential logic bug in this code.  If
                   * there are a lot of metrics that need/want to be
                   * sent on each heartbeat, this loop could loop
                   * forever.
                   *
                   * I'm ignoring the issue for now and leave it as a
                   * fault of the user of cerebrod.  It's technically
                   * possible to do this even without this loop.  For
                   * example, they could hammer the metric controller
                   * all day using cerebro-admin and the same problem
                   * would occur.
                   */

                  msg = _cerebrod_message_create(nst, &msglen, &more_data_to_send);

                  if (msglen >= CEREBRO_MAX_PACKET_LEN)
                    {
                      CEREBROD_DBG(("message exceeds maximum size: packet dropped"));
                      goto end_loop;
                    }

                  _cerebrod_message_dump(msg);

                  _cerebrod_message_send(msg, msglen);

                end_loop:
                  cerebrod_message_destroy(msg);
                  if (more_data_to_send)
                    CEREBROD_DBG(("extra heartbeat data to send"));
                }

              if (nst->next_send_type & CEREBROD_SPEAKER_NEXT_SEND_TYPE_HEARTBEAT)
                {
                  int t;

                  /* Algorithm from srand(3) manpage */
                  if (conf.heartbeat_frequency_ranged)
                    t = conf.heartbeat_frequency_min + ((((double)(conf.heartbeat_frequency_max - conf.heartbeat_frequency_min))*rand())/(RAND_MAX+1.0));
                  else
                    t = conf.heartbeat_frequency_min;

		  if (conf.gettimeofday_workaround)
		    {
		      /* On a number of Intel systems, it has been
		       * observed that gettimeofday can occasionally
		       * return erroneous values.  For example, values 40
		       * years into the future.  Then it goes back to
		       * returning valid values.
		       *
		       * Even the present workaround in Gettimeofday() was
		       * not found to be sufficient, so this additional
		       * workaround has been put in place.
		       *
		       * The following code should work around this
		       * situation.
		       */
		      if (conf.heartbeat_frequency_ranged
			  && nst->next_send_time
			  && (((now + t) - nst->next_send_time) > (2 * conf.heartbeat_frequency_max)))
			{
			  if (((now + t) - nst->next_send_time) < (10 * conf.heartbeat_frequency_max))
			    {
			      /* If the tv_sec is only moderately off, assume it's due to drift and it's ok */
			      nst->next_send_time = now + t;
			    }
			  else
			    {
			      CEREBROD_ERR (("Forcing maximum heartbeat frequency due to out of range time. "
                                             "tv_sec:%lu, t:%d, next_send_time:%u",
                                             now, t, (unsigned)nst->next_send_time));
			      nst->next_send_time = now + conf.heartbeat_frequency_max;
			    }
			}
		      else
			nst->next_send_time = now + t;
		    }
		  else
		    nst->next_send_time = now + t;
                }
              if (nst->next_send_type & CEREBROD_SPEAKER_NEXT_SEND_TYPE_MODULE)
                nst->next_send_time = now + nst->metric_period;
            }
          else
            break;
        }
      List_iterator_destroy(itr);

      List_sort(next_send_times, (ListCmpF)_next_send_time_compare);

      nst = List_peek(next_send_times);
      if (conf.gettimeofday_workaround)
	{
	  if ((nst->next_send_type & CEREBROD_SPEAKER_NEXT_SEND_TYPE_HEARTBEAT)
	      && (((nst->next_send_time - now) < 0)
		  || (conf.heartbeat_frequency_ranged
		      && ((nst->next_send_time - now) > conf.heartbeat_frequency_max))))
	    sleep_time = conf.heartbeat_frequency_max;
	  else
	    sleep_time = nst->next_send_time - now;
	}
      else
	sleep_time = nst->next_send_time - now;
      sleep(sleep_time);
    }

  return NULL;			/* NOT REACHED */
}

int
cerebrod_send_message(struct cerebrod_message *msg)
{
  int i, msglen = 0;

  if (!msg)
    {
      errno = EINVAL;
      return -1;
    }

  msglen = CEREBROD_MESSAGE_HEADER_LEN;
  for (i = 0; i < msg->metrics_len; i++)
    {
      if (!msg->metrics[i])
	{
	  CEREBROD_DBG(("null metrics pointer"));
	  errno = EINVAL;
	  goto cleanup;
	}
      msglen += CEREBROD_MESSAGE_METRIC_HEADER_LEN;
      msglen += msg->metrics[i]->metric_value_len;
    }

  if (msglen >= CEREBRO_MAX_PACKET_LEN)
    {
      CEREBROD_DBG(("message exceeds maximum size: packet dropped"));
      goto cleanup;
    }

  _cerebrod_message_dump(msg);
      
  _cerebrod_message_send(msg, msglen);

 cleanup:
  return 0;
}
