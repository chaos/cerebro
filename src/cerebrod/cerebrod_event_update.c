/*****************************************************************************\
 *  $Id: cerebrod_event_update.c,v 1.15 2010-04-06 22:10:11 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2018 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2005-2007 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <https://github.com/chaos/cerebro>.
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
#include <limits.h>
#include <errno.h>
#include <assert.h>

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"

#include "cerebrod_config.h"
#include "cerebrod_debug.h"
#include "cerebrod_listener_data.h"
#include "cerebrod_event_update.h"
#include "cerebrod_event_server.h"
#include "cerebrod_util.h"

#include "debug.h"
#include "event_module.h"
#include "hash.h"
#include "list.h"
#include "wrappers.h"

#define EVENT_NODE_TIMEOUT_SIZE_DEFAULT         1024
#define EVENT_NODE_TIMEOUT_SIZE_INCREMENT       1024

extern struct cerebrod_config conf;
extern pthread_mutex_t debug_output_mutex;

extern pthread_mutex_t listener_data_init_lock;

extern hash_t listener_data;
extern int listener_data_numnodes;
extern int listener_data_size;
extern pthread_mutex_t listener_data_lock;

/*
 * event_handle
 *
 * Handle for event modules;
 */
event_modules_t event_handle = NULL;

/*
 * event_index
 *
 * hash indexes to quickly determine what metrics are needed
 * by which modules.
 */
hash_t event_index = NULL;

/*
 * event_module_timeout_index
 *
 * Map of timeouts to modules that need to be called
 */
hash_t event_module_timeout_index = NULL;

/*
 * event_names
 *
 * List of event names supported by the modules
 */
List event_names = NULL;

/*
 * event_module_timeouts
 *
 * List of module timeout lengths
 */
List event_module_timeouts = NULL;
unsigned int event_module_timeout_min = INT_MAX;

/*
 * event_node_timeout_data
 * event_node_timeout_data_index
 *
 * data for calculating when nodes timeout and informing the
 * appropriate modules.
 */
List event_node_timeout_data = NULL;
hash_t event_node_timeout_data_index = NULL;
int event_node_timeout_data_index_numnodes;
int event_node_timeout_data_index_size;

/*
 * event_node_timeout_data_lock
 *
 * lock to protect pthread access to both the event_node_timeout_data
 * and event_node_timeout_data_index
 */
pthread_mutex_t event_node_timeout_data_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * event_queue
 * event_queue_cond
 * event_queue_lock
 *
 * queue of events to send out and the conditional variable and mutex
 * for exclusive access and signaling.
 */
List event_queue = NULL;
pthread_cond_t event_queue_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t event_queue_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * _cerebrod_event_module_list_destroy
 *
 * Destroy a event_module_list struct.
 */
static void
_cerebrod_event_module_list_destroy(void *data)
{
  struct cerebrod_event_module_list *el;

  assert(data);

  el = (struct cerebrod_event_module_list *)data;
  List_destroy(el->event_list);
  Free(el);
}

/*
 * _cerebrod_event_module_info_destroy
 *
 * Destroy a event_module_info struct.
 */
static void
_cerebrod_event_module_info_destroy(void *data)
{
  struct cerebrod_event_module_info *event_module;

  assert(data);

  event_module = (struct cerebrod_event_module_info *)data;
  Free(event_module->metric_names);
  Free(event_module);
}

/*
 * _cerebrod_event_module_timeout_data_destroy
 *
 * Destroy a monitor_module struct.
 */
static void
_cerebrod_event_module_timeout_data_destroy(void *data)
{
  struct cerebrod_event_module_timeout_data *mtd;

  assert(data);

  mtd = (struct cerebrod_event_module_timeout_data *)data;
  Free(mtd->timeout_str);
  Free(mtd);
}

/*
 * _event_node_timeout_data_add
 *
 * Create entries for the event_node_timeout_data list and index
 *
 * Returns 0 on success, -1 on error
 */
static int
_event_node_timeout_data_add(const char *nodename, u_int32_t time_now)
{
  struct cerebrod_event_node_timeout_data *ntd;
#if CEREBRO_DEBUG
  int rv;

  /* Should be called with lock already set */
  rv = Pthread_mutex_trylock(&event_node_timeout_data_lock);
  if (rv != EBUSY)
    CEREBROD_EXIT(("mutex not locked: rv=%d", rv));
#endif /* CEREBRO_DEBUG */

  ntd = (struct cerebrod_event_node_timeout_data *)Malloc(sizeof(struct cerebrod_event_node_timeout_data));
  ntd->nodename = (char *)nodename;
  ntd->last_received_time = time_now;
  ntd->timeout_occurred = 0;

  List_append(event_node_timeout_data, ntd);
  Hash_insert(event_node_timeout_data_index, ntd->nodename, ntd);
  event_node_timeout_data_index_numnodes++;
  return 0;
}

/*
 * _event_node_timeout_data_callback
 *
 * Callback from Hash_for_each on listener_data to create entries for
 * the event_node_timeout_data list and index for the first time.
 *
 * Returns 1 on success, 0 if not, -1 on error
 */
static int
_event_node_timeout_data_callback(void *data, const void *key, void *arg)
{
  struct cerebrod_node_data *nd;
  u_int32_t *time_now = (u_int32_t *)arg;

  nd = (struct cerebrod_node_data *)data;

  /* Note: Lock not really necessary b/c this function is called
   * during initialization, but it'll make the debug stuff happy
   * when you call _event_node_timeout_data_add().
   */
#if CEREBRO_DEBUG
  Pthread_mutex_lock(&event_node_timeout_data_lock);
#endif /* CEREBRO_DEBUG */
  _event_node_timeout_data_add(nd->nodename, *time_now);
#if CEREBRO_DEBUG
  Pthread_mutex_unlock(&event_node_timeout_data_lock);
#endif /* CEREBRO_DEBUG */
  return 1;
}

/*
 * _event_module_timeout_data_find_callback
 *
 * Callback for list_find to find matching timeout
 */
static int
_event_module_timeout_data_find_callback(void *x, void *key)
{
  struct cerebrod_event_module_timeout_data *mtd;
  int timeout;

  assert(x);
  assert(key);

  timeout = *((int *)key);
  mtd = (struct cerebrod_event_module_timeout_data *)x;

  if (mtd->timeout == timeout)
    return 1;
  return 0;
}

/*
 * _event_module_timeout_data_compare
 *
 * Callback from list_sort for comparison of data in
 * cerebrod_event_module_timeout_data.
 */
static int
_event_module_timeout_data_compare(void *x, void *y)
{
  struct cerebrod_event_module_timeout_data *a;
  struct cerebrod_event_module_timeout_data *b;

  a = (struct cerebrod_event_module_timeout_data *)x;
  b = (struct cerebrod_event_module_timeout_data *)y;

  if (a->timeout < b->timeout)
    return -1;
  if (a->timeout > b->timeout)
    return 1;
  return 0;
}

/*
 * _setup_event_node_timeout_data
 *
 * Setup event timeout calculation data structures.
 *
 * Return 0 on success, -1 on error
 */
static int
_setup_event_node_timeout_data(void)
{
  struct timeval tv;
  int num;

  assert(conf.event_server);
  assert(listener_data);

  event_node_timeout_data = List_create(NULL);
  event_node_timeout_data_index = Hash_create(EVENT_NODE_TIMEOUT_SIZE_DEFAULT,
                                              (hash_key_f)hash_key_string,
                                              (hash_cmp_f)strcmp,
                                              (hash_del_f)NULL);
  event_node_timeout_data_index_size = EVENT_NODE_TIMEOUT_SIZE_DEFAULT;

  Gettimeofday(&tv, NULL);

  num = Hash_for_each(listener_data,
                      _event_node_timeout_data_callback,
                      &(tv.tv_sec));
  if (num != listener_data_numnodes)
    {
      fprintf(stderr, "* invalid create count: num=%d numnodes=%d\n",
              num, listener_data_numnodes);
      goto cleanup;
    }

  return 0;

 cleanup:
  if (event_node_timeout_data)
    {
      List_destroy(event_node_timeout_data);
      event_node_timeout_data = NULL;
    }
  if (event_node_timeout_data_index)
    {
      Hash_destroy(event_node_timeout_data_index);
      event_node_timeout_data_index =  NULL;
    }
  return -1;
}

void
cerebrod_event_to_send_destroy(void *x)
{
  struct cerebrod_event_to_send *ets;

  assert(x);
  assert(event_handle);

  ets = (struct cerebrod_event_to_send *)x;
  event_module_destroy(event_handle, ets->index, ets->event);
  Free(ets);
}

/*
 * _cerebrod_name_strcmp
 */
static int
_cerebrod_name_strcmp(void *x, void *key)
{
  assert(x);
  assert(key);

  if (!strcmp((char *)x, (char *)key))
    return (1);
  return (0);
}

/*
 * Under almost any circumstance, don't return a -1 error, cerebro can
 * go on without loading monitor modules. The listener_data_init_lock
 * should already be set.
 */
int
cerebrod_event_modules_setup(void)
{
  int i, event_module_count, event_index_len, event_index_count = 0;
  struct cerebrod_event_module_list *el = NULL;
#if CEREBRO_DEBUG
  int rv;
#endif /* CEREBRO_DEBUG */

  assert(listener_data);

#if CEREBRO_DEBUG
  /* Should be called with lock already set */
  rv = Pthread_mutex_trylock(&listener_data_init_lock);
  if (rv != EBUSY)
    CEREBROD_EXIT(("mutex not locked: rv=%d", rv));
#endif /* CEREBRO_DEBUG */

  if (!conf.event_server)
    return 0;

  if (!(event_handle = event_modules_load()))
    {
      CEREBROD_DBG(("event_modules_load"));
      goto cleanup;
    }

  if ((event_module_count = event_modules_count(event_handle)) < 0)
    {
      CEREBROD_DBG(("event_modules_count"));
      goto cleanup;
    }

  if (!event_module_count)
    {
      if (conf.debug && conf.event_server_debug)
        {
          Pthread_mutex_lock(&debug_output_mutex);
          fprintf(stderr, "**************************************\n");
          fprintf(stderr, "* No Event Modules Found\n");
          fprintf(stderr, "**************************************\n");
          Pthread_mutex_unlock(&debug_output_mutex);
        }
      goto cleanup;
    }

  /* Each event module may want multiple metrics and/or offer multiple
   * event names.  We'll assume there will never be more than 2 per
   * event module, and that will be enough to avoid all hash
   * collisions.
   */
  event_index_len = event_module_count * 2;

  event_index = Hash_create(event_index_len,
                            (hash_key_f)hash_key_string,
                            (hash_cmp_f)strcmp,
                            (hash_del_f)_cerebrod_event_module_list_destroy);

  event_names = List_create((ListDelF)_Free);

  event_module_timeouts = List_create((ListDelF)_cerebrod_event_module_timeout_data_destroy);
  event_module_timeout_index = Hash_create(event_module_count,
					   (hash_key_f)hash_key_string,
					   (hash_cmp_f)strcmp,
					   (hash_del_f)list_destroy);

  for (i = 0; i < event_module_count; i++)
    {
      struct cerebrod_event_module *event_module;
      char *module_name, *module_metric_names, *module_event_names;
      char *metricPtr, *metricbuf;
      char *eventnamePtr, *eventbuf;
      int timeout;

      module_name = event_module_name(event_handle, i);

      if (conf.event_module_exclude_len)
        {
          int found_exclude = 0;
          int j;

          for (j = 0; j < conf.event_module_exclude_len; j++)
            {
              if (!strcasecmp(conf.event_module_exclude[j], module_name))
                {
                  found_exclude++;
                  break;
                }
            }

          if (found_exclude)
            {
              if (conf.debug && conf.event_server_debug)
                {
                  Pthread_mutex_lock(&debug_output_mutex);
                  fprintf(stderr, "**************************************\n");
                  fprintf(stderr, "* Skip Event Module: %s\n", module_name);
                  fprintf(stderr, "**************************************\n");
                  Pthread_mutex_unlock(&debug_output_mutex);
                }
              CEREBROD_ERR(("Dropping event module: %s", module_name));
              continue;
            }
        }

      if (conf.debug && conf.event_server_debug)
        {
          Pthread_mutex_lock(&debug_output_mutex);
          fprintf(stderr, "**************************************\n");
          fprintf(stderr, "* Setup Event Module: %s\n", module_name);
          fprintf(stderr, "**************************************\n");
          Pthread_mutex_unlock(&debug_output_mutex);
        }

      if (event_module_setup(event_handle, i) < 0)
        {
          CEREBROD_DBG(("event_module_setup failed: %s", module_name));
          continue;
        }

      if (!(module_metric_names = event_module_metric_names(event_handle, i)))
        {
          CEREBROD_DBG(("event_module_metric_names failed: %s", module_name));
          event_module_cleanup(event_handle, i);
          continue;
        }

      if (!(module_event_names = event_module_event_names(event_handle, i)))
        {
          CEREBROD_DBG(("event_module_event_names failed: %s", module_name));
          event_module_cleanup(event_handle, i);
          continue;
        }

      if ((timeout = event_module_timeout_length(event_handle, i)) < 0)
        {
          CEREBROD_DBG(("event_module_timeout_length failed: %s", module_name));
          event_module_cleanup(event_handle, i);
          continue;
        }

      event_module = Malloc(sizeof(struct cerebrod_event_module_info));
      event_module->metric_names = Strdup(module_metric_names);
      event_module->event_names = Strdup(module_event_names);
      event_module->index = i;
      Pthread_mutex_init(&(event_module->event_lock), NULL);

      /* The monitoring module may support multiple metrics */

      metricPtr = strtok_r(event_module->metric_names, ",", &metricbuf);
      while (metricPtr)
        {
          if (!(el = Hash_find(event_index, metricPtr)))
            {
              el = (struct cerebrod_event_module_list *)Malloc(sizeof(struct cerebrod_event_module_list));
              el->event_list = List_create((ListDelF)_cerebrod_event_module_info_destroy);
              Pthread_mutex_init(&(el->event_list_lock), NULL);

              List_append(el->event_list, event_module);
              Hash_insert(event_index, metricPtr, el);
              event_index_count++;
            }
          else
            List_append(el->event_list, event_module);

          metricPtr = strtok_r(NULL, ",", &metricbuf);
        }

      /* The monitoring module may support multiple event names */

      eventnamePtr = strtok_r(event_module->event_names, ",", &eventbuf);
      while (eventnamePtr)
        {
          if (!list_find_first(event_names,
                               (ListFindF)_cerebrod_name_strcmp,
                               eventnamePtr))
            {
              List_append(event_names, eventnamePtr);
              if (conf.debug && conf.event_server_debug)
                {
                  Pthread_mutex_lock(&debug_output_mutex);
                  fprintf(stderr, "**************************************\n");
                  fprintf(stderr, "* Event Name: %s\n", eventnamePtr);
                  fprintf(stderr, "**************************************\n");
                  Pthread_mutex_unlock(&debug_output_mutex);
                }
            }
          eventnamePtr = strtok_r(NULL, ",", &eventbuf);
        }

      if (timeout)
        {
          struct cerebrod_event_module_timeout_data *mtd;
          List modules_list;

          if (!(mtd = List_find_first(event_module_timeouts,
                                      _event_module_timeout_data_find_callback,
                                      &timeout)))
            {
              char strbuf[64];

              mtd = (struct cerebrod_event_module_timeout_data *)Malloc(sizeof(struct cerebrod_event_module_timeout_data));
              mtd->timeout = timeout;
              snprintf(strbuf, 64, "%d", timeout);
              mtd->timeout_str = Strdup(strbuf);

              List_append(event_module_timeouts, mtd);

              if (timeout < event_module_timeout_min)
                event_module_timeout_min = timeout;
            }

          if (!(modules_list = Hash_find(event_module_timeout_index,
                                         mtd->timeout_str)))
            {
              modules_list = List_create((ListDelF)NULL);
              List_append(modules_list, event_module);
              Hash_insert(event_module_timeout_index,
                          mtd->timeout_str,
                          modules_list);
            }
          else
            List_append(modules_list, event_module);
        }
    }

  List_sort(event_module_timeouts, _event_module_timeout_data_compare);

  if (!event_index_count)
    goto cleanup;

  if (_setup_event_node_timeout_data() < 0)
    goto cleanup;

  /*
   * Since the cerebrod listener is started before any of the event
   * threads (node_timeout, queue_monitor, server), this must be
   * created in here (which is called by the listener) to avoid a
   * possible race of modules creating events before the event_queue
   * is created.
   */
  event_queue = List_create((ListDelF)cerebrod_event_to_send_destroy);

  return 1;

 cleanup:
  if (event_handle)
    {
      event_modules_unload(event_handle);
      event_handle = NULL;
    }
  if (event_index)
    {
      Hash_destroy(event_index);
      event_index = NULL;
    }
  if (event_names)
    {
      List_destroy(event_names);
      event_names = NULL;
    }
  return 0;
}

void
cerebrod_event_add_node_timeout_data(struct cerebrod_node_data *nd,
                                     u_int32_t received_time)
{
#if CEREBRO_DEBUG
  int rv;
#endif /* CEREBRO_DEBUG */

  assert(nd);
  assert(received_time);

  if (!event_index)
    return;

  assert(event_node_timeout_data_index);

#if CEREBRO_DEBUG
  /* Should be called with lock already set */
  rv = Pthread_mutex_trylock(&listener_data_lock);
  if (rv != EBUSY)
    CEREBROD_EXIT(("mutex not locked: rv=%d", rv));
#endif /* CEREBRO_DEBUG */

  Pthread_mutex_lock(&event_node_timeout_data_lock);
  /* Re-hash if our hash is getting too small */
  if ((event_node_timeout_data_index_numnodes + 1) > (event_node_timeout_data_index_size*2))
    cerebrod_rehash(&event_node_timeout_data_index,
                    &event_node_timeout_data_index_size,
                    EVENT_NODE_TIMEOUT_SIZE_INCREMENT,
                    event_node_timeout_data_index_numnodes,
                    &event_node_timeout_data_lock);

  _event_node_timeout_data_add(nd->nodename, received_time);
  event_node_timeout_data_index_numnodes++;
  Pthread_mutex_unlock(&event_node_timeout_data_lock);
}

void
cerebrod_event_update_node_received_time(struct cerebrod_node_data *nd,
                                         u_int32_t received_time)
{
  struct cerebrod_event_node_timeout *ntd;
#if CEREBRO_DEBUG
  int rv;
#endif /* CEREBRO_DEBUG */

  assert(nd);
  assert(received_time);

  if (!event_index)
    return;

#if CEREBRO_DEBUG
  /* Should be called with lock already set */
  rv = Pthread_mutex_trylock(&nd->node_data_lock);
  if (rv != EBUSY)
    CEREBROD_EXIT(("mutex not locked: rv=%d", rv));
#endif /* CEREBRO_DEBUG */

  Pthread_mutex_lock(&event_node_timeout_data_lock);
  if ((ntd = Hash_find(event_node_timeout_data_index, nd->nodename)))
    {
      ntd->last_received_time = received_time;
      ntd->timeout_occurred = 0;
    }
  Pthread_mutex_unlock(&event_node_timeout_data_lock);
}

void
cerebrod_event_modules_update(const char *nodename,
                              struct cerebrod_node_data *nd,
                              const char *metric_name,
                              struct cerebrod_message_metric *mm)
{
  struct cerebrod_event_module_info *event_module;
  struct cerebrod_event_module_list *el;
#if CEREBRO_DEBUG
  int rv;
#endif /* CEREBRO_DEBUG */

  assert(nodename && nd && metric_name && mm);

  if (!event_index)
    return;

#if CEREBRO_DEBUG
  /* Should be called with lock already set */
  rv = Pthread_mutex_trylock(&nd->node_data_lock);
  if (rv != EBUSY)
    CEREBROD_EXIT(("mutex not locked: rv=%d", rv));
#endif /* CEREBRO_DEBUG */

  /*
   * This function may be called by multiple threads by the listener.
   *
   * The event_index is setup at the beginning and is only read, not
   * written to.  However, the lists stored inside the event_index
   * need to called w/ thread safety (due to the nature of the list
   * API).
   *
   */

  if ((el = Hash_find(event_index, metric_name)))
    {
      struct cerebro_event *event = NULL;
      ListIterator itr = NULL;
      int rv;

      Pthread_mutex_lock(&(el->event_list_lock));
      itr = List_iterator_create(el->event_list);
      Pthread_mutex_unlock(&(el->event_list_lock));

      while ((event_module = list_next(itr)))
	{
	  Pthread_mutex_lock(&event_module->event_lock);
	  if ((rv = event_module_metric_update(event_handle,
                                               event_module->index,
                                               nodename,
                                               metric_name,
                                               mm->metric_value_type,
                                               mm->metric_value_len,
                                               mm->metric_value,
                                               &event)) < 0)
            {
              CEREBROD_DBG(("event_module_metric_update"));
              goto loop_next;
            }

          if (rv && event)
            cerebrod_queue_event(event, event_module->index);

        loop_next:
	  Pthread_mutex_unlock(&event_module->event_lock);
	}

      Pthread_mutex_lock(&(el->event_list_lock));
      List_iterator_destroy(itr);
      Pthread_mutex_unlock(&(el->event_list_lock));
    }
}

#endif /* !WITH_CEREBROD_SPEAKER_ONLY */

