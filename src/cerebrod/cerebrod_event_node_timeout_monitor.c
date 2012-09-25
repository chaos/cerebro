/*****************************************************************************\
 *  $Id: cerebrod_event_node_timeout_monitor.c,v 1.11 2010-02-02 01:01:20 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2011 Lawrence Livermore National Security, LLC.
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

#if !WITH_CEREBROD_SPEAKER_ONLY

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <errno.h>
#include <assert.h>

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"

#include "cerebrod_config.h"
#include "cerebrod_event_node_timeout_monitor.h"
#include "cerebrod_event_server.h"
#include "cerebrod_event_update.h"

#include "debug.h"
#include "event_module.h"
#include "list.h"
#include "wrappers.h"

extern struct cerebrod_config conf;
extern pthread_mutex_t debug_output_mutex;

/* 
 * event_node_timeout_monitor_init
 * event_node_timeout_monitor_init_cond
 * event_node_timeout_monitor_init_lock
 *
 * variables for synchronizing initialization between different pthreads
 * and signaling when it is complete
 */
int event_node_timeout_monitor_init = 0;
pthread_cond_t event_node_timeout_monitor_init_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t event_node_timeout_monitor_init_lock = PTHREAD_MUTEX_INITIALIZER;

extern event_modules_t event_handle;
extern hash_t event_index;

extern List event_module_timeouts;
extern unsigned int event_module_timeout_min;
extern hash_t event_module_timeout_index;

extern List event_node_timeout_data;
extern hash_t event_node_timeout_data_index;
extern int event_node_timeout_data_index_numnodes;
extern int event_node_timeout_data_index_size;
extern pthread_mutex_t event_node_timeout_data_lock;

/*
 * _event_node_timeout_monitor_initialize
 *
 * perform metric server initialization
 */
static void
_event_node_timeout_monitor_initialize(void)
{
  Pthread_mutex_lock(&event_node_timeout_monitor_init_lock);
  if (event_node_timeout_monitor_init)
    goto out;

  /* I don't think there's anything to do? */

  event_node_timeout_monitor_init++;
  Pthread_cond_signal(&event_node_timeout_monitor_init_cond);
 out:
  Pthread_mutex_unlock(&event_node_timeout_monitor_init_lock);
}

/*
 * _event_node_timeout_data_compare
 *
 * Comparison for list sorting
 */
static int
_event_node_timeout_data_compare(void *x, void *y)
{
  struct cerebrod_event_node_timeout_data *a;
  struct cerebrod_event_node_timeout_data *b;

  a = (struct cerebrod_event_node_timeout_data *)x;
  b = (struct cerebrod_event_node_timeout_data *)y;

  /* if a timeout occurred, it's later in the sort */
  if (!a->timeout_occurred && b->timeout_occurred)
    return -1;
  if (a->timeout_occurred && !b->timeout_occurred)
    return 1;

  if (a->last_received_time < b->last_received_time)
    return -1;
  if (a->last_received_time > b->last_received_time)
    return 1;
  return 0;
}

void *
cerebrod_event_node_timeout_monitor(void *arg)
{
  ListIterator timesitr;

  _event_node_timeout_monitor_initialize();

  /* Don't bother if there aren't any event modules */
  if (!event_index)
    return NULL;

  /* Note: After initial setup, we are the only thread that uses this
   * list/iterator.  So no need for pthread locking.
   */
  timesitr = List_iterator_create(event_module_timeouts);

  while (1)
    {
      struct cerebrod_event_node_timeout_data *ntd;
      u_int32_t last_received_time = 0;
      ListIterator nodesitr;
      struct timeval tv;

      Gettimeofday(&tv, NULL);

      Pthread_mutex_lock(&event_node_timeout_data_lock);
      List_sort(event_node_timeout_data, _event_node_timeout_data_compare);

      nodesitr = List_iterator_create(event_node_timeout_data);
      
      while ((ntd = list_next(nodesitr)))
        {
          struct cerebrod_event_module_timeout_data *mtd;
          int timeout_hit = 0;

          list_iterator_reset(timesitr);
          while ((mtd = list_next(timesitr)))
            {
              if (tv.tv_sec >= (ntd->last_received_time + mtd->timeout))
                {
                  if (!ntd->timeout_occurred)
                    {
                      List modules_list;

                      if (conf.event_server_debug)
                        {
                          Pthread_mutex_lock(&debug_output_mutex);
                          fprintf(stderr, "**************************************\n");
                          fprintf(stderr, "* Event Node Timeout: %s\n", ntd->nodename);
                          fprintf(stderr, "**************************************\n");
                          Pthread_mutex_unlock(&debug_output_mutex);
                        }

                      if ((modules_list = Hash_find(event_module_timeout_index, 
                                                    mtd->timeout_str)))
                        {
                          struct cerebrod_event_module *event_module;
                          ListIterator modulesitr;

                          modulesitr = List_iterator_create(modules_list);
                          
                          while ((event_module = list_next(modulesitr)))
                            {
                              struct cerebro_event *event = NULL;
                              int rv;

                              Pthread_mutex_lock(&event_module->event_lock);
                              if ((rv = event_module_node_timeout(event_handle,
                                                                  event_module->index,
                                                                  ntd->nodename, 
                                                                  &event)) < 0)
                                {
                                  CEREBRO_DBG(("event_module_node_timeout"));
                                  goto loop_next;
                                }

                              if (rv && event)
                                cerebrod_queue_event(event, event_module->index);

                            loop_next:
                              Pthread_mutex_unlock(&event_module->event_lock);
                            }

                          List_iterator_destroy(modulesitr);
                        }

                      ntd->timeout_occurred++;
                      timeout_hit++;
                    }
                }

              /* If we didn't timeout with this timeout length, we
               * won't on any other timeout later in the list, so
               * break
               */
              if (!timeout_hit)
                break;
            }

          /* If we didn't timeout on this node, we won't on
           * any other node later in the list, so break
           */
          if (!timeout_hit)
            break;
        }

      List_iterator_destroy(nodesitr);

      List_sort(event_node_timeout_data, _event_node_timeout_data_compare);

      /* When might we timeout again? It is possible for the list to be
       * empty, see below.
       */
      if ((ntd = List_peek(event_node_timeout_data)))
        {
          if (!ntd->timeout_occurred)
            last_received_time = ntd->last_received_time;
        }

      Pthread_mutex_unlock(&event_node_timeout_data_lock);

      if ((last_received_time + event_module_timeout_min) > tv.tv_sec)
        sleep((last_received_time + event_module_timeout_min) - tv.tv_sec);
      else
        /* If we reach this point, it means all nodes have timed out
         * (this would mean the listening/event_server node wouldn't
         * be a speaker node or perhaps no nodes are known about
         * b/c no clusterlist module has been configured).
         *
         * So what should the timeout be? Well, the worst thing that
         * can happen is a message packet comes the exact microsecond
         * after we've determined all nodes are down.  So the packet
         * that just came in has the potential to timeout after
         * 'event_module_timeout_min' seconds.  So that's the time
         * we'll sleep for.
         */
        sleep(event_module_timeout_min);

    }

  return NULL;			/* NOT REACHED */
}

#endif /* !WITH_CEREBROD_SPEAKER_ONLY */
