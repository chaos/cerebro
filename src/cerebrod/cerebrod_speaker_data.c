/*****************************************************************************\
 *  $Id: cerebrod_speaker_data.c,v 1.35 2005-08-22 15:50:08 achu Exp $
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

#include "protocol/cerebrod_heartbeat_protocol.h"

#include "cerebrod.h"
#include "cerebrod_config.h"
#include "cerebrod_speaker_data.h"

#include "debug.h"
#include "metric_module.h"
#include "metric_util.h"
#include "wrappers.h"

extern struct cerebrod_config conf;
#if CEREBRO_DEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* CEREBRO_DEBUG */

/*
 * speaker_data_init
 * speaker_data_init_lock
 *
 * variables for synchronizing initialization between different pthreads
 */
int speaker_data_init = 0;
pthread_mutex_t speaker_data_init_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * metric_handle
 *
 * Handle for metric modules;
 */
metric_modules_t metric_handle = NULL;

/*
 * metric_list
 *
 * Metrics to grab data from
 */
List metric_list = NULL;
int metric_list_size = 0;

/*
 * metric_list_lock
 *
 * lock to protect pthread access to the metric_list
 */
pthread_mutex_t metric_list_lock = PTHREAD_MUTEX_INITIALIZER;

/* 
 * _metric_is_updated
 *
 * May be called by a metric module to inform us to 
 * propogate the metric.
 *
 * Returns 0 on success, -1 on error
 */
static int 
_metric_is_updated(char *metric_name)
{
  struct cerebrod_speaker_metric_info *metric_info = NULL;
  ListIterator itr = NULL;
  int found = 0;

  assert(metric_list);

  if (!metric_name)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  Pthread_mutex_lock(&metric_list_lock);
  itr = List_iterator_create(metric_list);
  while ((metric_info = list_next(itr)))
    {
      if (!strcmp(metric_info->metric_name, metric_name))
        {
          metric_info->next_call_time = 0;
          found++;
          break;
        }
    }
  cerebrod_speaker_data_metric_list_sort();
  Pthread_mutex_unlock(&metric_list_lock);

#if CEREBRO_DEBUG  
  if (!found)
    CEREBRO_DBG(("metric '%s' not found", metric_name));
#endif /* CEREBRO_DEBUG */

  return (found) ? 0 : -1;
}

/* 
 * _setup_metric_modules
 *
 * Setup metric modules. Under almost any circumstance, don't return a
 * -1 error, cerebro can go on without loading metric modules.
 *
 * Returns 1 if modules are loaded, 0 if not, -1 on error
 */
static int
_setup_metric_modules(void)
{
  int i, metric_index_len;
#if CEREBRO_DEBUG
  int rv;
#endif /* CEREBRO_DEBUG */

  assert(metric_list);

#if CEREBRO_DEBUG
  /* Should be called with lock already set */
  rv = Pthread_mutex_trylock(&metric_list_lock);
  if (rv != EBUSY)
    CEREBRO_EXIT(("mutex not locked: rv=%d", rv));
#endif /* CEREBRO_DEBUG */

  if (!(metric_handle = metric_modules_load(conf.metric_max)))
    {
      CEREBRO_DBG(("metric_modules_load"));
      goto cleanup;
    }
  
  if ((metric_index_len = metric_modules_count(metric_handle)) < 0)
    {
      CEREBRO_DBG(("metric_module_count failed"));
      goto cleanup;
    }
  
  if (!metric_index_len)
    {
#if CEREBRO_DEBUG
      if (conf.debug && conf.speak_debug)
        {
          Pthread_mutex_lock(&debug_output_mutex);
          fprintf(stderr, "**************************************\n");
          fprintf(stderr, "* No Metric Modules Found\n");
          fprintf(stderr, "**************************************\n");
          Pthread_mutex_unlock(&debug_output_mutex);
        }
#endif /* CEREBRO_DEBUG */
      goto cleanup;
    }

  for (i = 0; i < metric_index_len; i++)
    {
      struct cerebrod_speaker_metric_info *metric_info;
      Cerebro_metric_thread_pointer threadPtr;
      char *module_name, *metric_name;
      int metric_period;
      
      module_name = metric_module_name(metric_handle, i);
#if CEREBRO_DEBUG
      if (conf.debug && conf.speak_debug)
        {
          Pthread_mutex_lock(&debug_output_mutex);
          fprintf(stderr, "**************************************\n");
          fprintf(stderr, "* Setup Metric Module: %s\n", module_name);
          fprintf(stderr, "**************************************\n");
          Pthread_mutex_unlock(&debug_output_mutex);
        }
#endif /* CEREBRO_DEBUG */

      if (metric_module_setup(metric_handle, i) < 0)
        {
          CEREBRO_DBG(("metric_module_setup: %s", module_name));
          continue;
        }

      if (!(metric_name = metric_module_get_metric_name(metric_handle, i)))
        {
          CEREBRO_DBG(("metric_module_get_metric_name: %s", module_name));
          metric_module_cleanup(metric_handle, i);
          continue;
        }

      if (metric_module_get_metric_period(metric_handle, i, &metric_period) < 0)
        {
          CEREBRO_DBG(("metric_module_get_metric_period: %s", module_name));
          metric_module_cleanup(metric_handle, i);
          continue;
        }
 
      metric_info = Malloc(sizeof(struct cerebrod_speaker_metric_info));
      /* No need to Strdup() the name in this case */
      metric_info->metric_name = metric_name;
      metric_info->metric_origin = CEREBROD_METRIC_SPEAKER_ORIGIN_MODULE;

      metric_info->metric_period = metric_period;
      metric_info->index = i;

      if (metric_info->metric_period < 0)
        metric_info->next_call_time = UINT_MAX;
      else
        /* Initialize to 0, so data is sent on the first heartbeat */
        metric_info->next_call_time = 0;

      List_append(metric_list, metric_info);
      metric_list_size++;

      if ((threadPtr = metric_module_get_metric_thread(metric_handle, i)))
        {          
          pthread_t thread;
          pthread_attr_t attr;
          
          Pthread_attr_init(&attr);
          Pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
          Pthread_attr_setstacksize(&attr, CEREBROD_THREAD_STACKSIZE);
          Pthread_create(&thread, &attr, threadPtr, &_metric_is_updated);
          Pthread_attr_destroy(&attr);
        }
    }
  
  if (!metric_list_size)
    goto cleanup;

  return 1;

 cleanup:
  if (metric_handle)
    {
      /* unload will call module cleanup functions */
      metric_modules_unload(metric_handle);
      metric_handle = NULL;
    }
  metric_list_size = 0;
  return 0;
}

/* 
 * _destroy_speaker_metric_info
 *
 * Deallocate speaker_metric_info structure
 */
static void
_destroy_speaker_metric_info(void *x)
{
  struct cerebrod_speaker_metric_info *metric_info;
  
  assert(x);

  metric_info = (struct cerebrod_speaker_metric_info *)x;

  if (metric_info->metric_origin & CEREBROD_METRIC_SPEAKER_ORIGIN_USERSPACE)
    {
      Free(metric_info->metric_name);
      if (metric_info->metric_value)
        Free(metric_info->metric_value);
    }
  
  Free(metric_info);
}

void
cerebrod_speaker_data_initialize(void)
{
  Pthread_mutex_lock(&speaker_data_init_lock);
  if (speaker_data_init)
    goto out;

  /* 
   * Must lock in this initialization routine, b/c the update thread
   * in a metric module may call the update state function.
   */
  Pthread_mutex_lock(&metric_list_lock);

  metric_list = List_create((ListDelF)_destroy_speaker_metric_info);

  if (_setup_metric_modules() < 0)
    CEREBRO_EXIT(("_setup_metric_modules"));

  Pthread_mutex_unlock(&metric_list_lock);

  speaker_data_init++;
 out:
  Pthread_mutex_unlock(&speaker_data_init_lock);
}

/*
 * _next_call_time_cmp
 *
 * callback function for list_sort to sort node names
 */
static int
_next_call_time_cmp(void *x, void *y)
{
  struct cerebrod_speaker_metric_info *a;
  struct cerebrod_speaker_metric_info *b;

  assert(x && y);

  a = (struct cerebrod_speaker_metric_info *)x;
  b = (struct cerebrod_speaker_metric_info *)y;

  if (a->next_call_time == b->next_call_time)
    return 0;
  else if (a->next_call_time < b->next_call_time)
    return -1;
  else 
    return 1;
}

/* 
 * _get_module_metric_value
 *
 * Get the metric value data from a module
 *
 * Returns heartbeat metric data on success, NULL otherwise
 */
static struct cerebrod_heartbeat_metric *
_get_module_metric_value(struct cerebrod_speaker_metric_info *metric_info)
{
  struct cerebrod_heartbeat_metric *hd = NULL;
  char *metric_name;
  u_int32_t mtype, mlen;
  void *mvalue = NULL;
  unsigned int index;

  assert(metric_info);

  hd = Malloc(sizeof(struct cerebrod_heartbeat_metric));
  memset(hd, '\0', sizeof(struct cerebrod_heartbeat_metric));

  index = metric_info->index;

  if (!(metric_name = metric_module_get_metric_name(metric_handle, index)))
    {
      CEREBRO_DBG(("metric_module_get_metric_name: %d", index));
      goto cleanup;
    }

  /* need not overflow */
  strncpy(hd->metric_name, metric_name, CEREBRO_MAX_METRIC_NAME_LEN);
  
  if (metric_module_get_metric_value(metric_handle,
                                     index,
                                     &mtype,
                                     &mlen,
                                     &mvalue) < 0)
    {
      CEREBRO_DBG(("metric_module_get_metric_value: %d", index));
      goto cleanup;
    }

  if (mtype == CEREBRO_METRIC_VALUE_TYPE_STRING 
      && mlen > CEREBRO_MAX_METRIC_STRING_LEN)
    {
      CEREBRO_DBG(("truncate metric string: %d", mlen));
      mlen = CEREBRO_MAX_METRIC_STRING_LEN;
    }
 
  if (mtype == CEREBRO_METRIC_VALUE_TYPE_STRING && !mlen)
    {
      CEREBRO_DBG(("adjusting metric type to none"));
      mtype = CEREBRO_METRIC_VALUE_TYPE_NONE;
    }

  if (check_metric_type_len_value(mtype, mlen, mvalue) < 0)
    goto cleanup;

  hd->metric_value_type = mtype;
  hd->metric_value_len = mlen;
  if (hd->metric_value_len)
    {
      hd->metric_value = Malloc(hd->metric_value_len);
      memcpy(hd->metric_value, mvalue, hd->metric_value_len);
    }
  else
    hd->metric_value = NULL;
  metric_module_destroy_metric_value(metric_handle, index, mvalue);
  
  return hd;

 cleanup:
  
  if (mvalue)
    metric_module_destroy_metric_value(metric_handle, index, mvalue);

  if (hd)
    {
      if (hd->metric_value)
        Free(hd->metric_value);
      Free(hd);
    }
 
  return NULL;
}

/* 
 * _get_userspace_metric_value
 *
 * Get the metric value data supplied by a userspace program
 *
 * Returns heartbeat metric data on success, NULL otherwise
 */
static struct cerebrod_heartbeat_metric *
_get_userspace_metric_value(struct cerebrod_speaker_metric_info *metric_info)
{
  struct cerebrod_heartbeat_metric *hd = NULL;
  u_int32_t mtype, mlen;
  void *mvalue;

  assert(metric_info);

#if CEREBRO_DEBUG
  if (metric_info->next_call_time)
    CEREBRO_DBG(("Unexpected next_call_time"));
#endif /* CEREBRO_DEBUG */

  mtype = metric_info->metric_value_type;
  mlen = metric_info->metric_value_len;
  mvalue = metric_info->metric_value;
  if (check_metric_type_len_value(mtype, mlen, mvalue) < 0)
    goto cleanup;

  hd = Malloc(sizeof(struct cerebrod_heartbeat_metric));
  memset(hd, '\0', sizeof(struct cerebrod_heartbeat_metric));

  /* need not overflow */
  strncpy(hd->metric_name, 
          metric_info->metric_name, 
          CEREBRO_MAX_METRIC_NAME_LEN);

  hd->metric_value_type = metric_info->metric_value_type;
  hd->metric_value_len = metric_info->metric_value_len;
  if (hd->metric_value_len)
    {
      hd->metric_value = Malloc(metric_info->metric_value_len);
      memcpy(hd->metric_value, 
             metric_info->metric_value, 
             metric_info->metric_value_len);
    }
  else
    hd->metric_value = NULL;
      
  return hd;

 cleanup:
  if (hd)
    {
      if (hd->metric_value)
        Free(hd->metric_value);
      Free(hd);
    }
  return NULL;
}

void 
cerebrod_speaker_data_metric_list_sort(void)
{
#if CEREBRO_DEBUG
  int rv;
#endif /* CEREBRO_DEBUG */

  assert(metric_list);

#if CEREBRO_DEBUG
  /* Should be called with lock already set */
  rv = Pthread_mutex_trylock(&metric_list_lock);
  if (rv != EBUSY)
    CEREBRO_EXIT(("mutex not locked: rv=%d", rv));
#endif /* CEREBRO_DEBUG */

  List_sort(metric_list, _next_call_time_cmp);
}

void 
cerebrod_speaker_data_get_metric_data(struct cerebrod_heartbeat *hb,
                                      unsigned int *heartbeat_len)
{
  struct cerebrod_speaker_metric_info *metric_info;
  ListIterator itr = NULL;
  struct timeval tv;

  assert(hb && heartbeat_len);

  if (!speaker_data_init)
    CEREBRO_EXIT(("initialization not complete"));

  Pthread_mutex_lock(&metric_list_lock);

  /* There may not be any metrics to distribute */
  if (!metric_list_size)
    {
      hb->metrics_len = 0;
      hb->metrics = NULL;
      goto out;
    }

  hb->metrics_len = 0;
  hb->metrics = Malloc(sizeof(struct cerebrod_heartbeat_metric *)*(metric_list_size + 1));
  memset(hb->metrics, '\0', sizeof(struct cerebrod_heartbeat_metric *)*(metric_list_size + 1));

  Gettimeofday(&tv, NULL);
  
  itr = List_iterator_create(metric_list);
  while ((metric_info = list_next(itr)))
    {      
      struct cerebrod_heartbeat_metric *hd = NULL;

      if (tv.tv_sec <= metric_info->next_call_time)
        break;

      if (metric_info->metric_origin & CEREBROD_METRIC_SPEAKER_ORIGIN_MODULE)
        hd = _get_module_metric_value(metric_info);

      if (metric_info->metric_origin & CEREBROD_METRIC_SPEAKER_ORIGIN_USERSPACE)
        {
          hd = _get_userspace_metric_value(metric_info);
          metric_info->next_call_time = UINT_MAX;
        }

      if (hd)
        {
          *heartbeat_len += CEREBROD_HEARTBEAT_METRIC_HEADER_LEN;
          *heartbeat_len += hd->metric_value_len;
          hb->metrics[hb->metrics_len] = hd;
          hb->metrics_len++;
        }

      if (metric_info->metric_origin & CEREBROD_METRIC_SPEAKER_ORIGIN_MODULE)
        {
          if (metric_info->metric_period < 0)
            metric_info->next_call_time = UINT_MAX;

          /* 
           * Metric period stays at 0 for metrics that need to be
           * propogated every time
           */
          if (metric_info->metric_period > 0)
            metric_info->next_call_time = tv.tv_sec + metric_info->metric_period;
        }
    } 
  List_iterator_destroy(itr);
  cerebrod_speaker_data_metric_list_sort();
 out:
  Pthread_mutex_unlock(&metric_list_lock);
  return;
}
