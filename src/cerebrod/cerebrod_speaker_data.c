/*****************************************************************************\
 *  $Id: cerebrod_speaker_data.c,v 1.51 2008-03-28 17:06:48 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2008 Lawrence Livermore National Security, LLC.
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

#include "cerebrod.h"
#include "cerebrod_config.h"
#include "cerebrod_speaker.h"
#include "cerebrod_speaker_data.h"

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
 * speaker_data_init
 * speaker_data_init_lock
 *
 * variables for synchronizing initialization between different pthreads
 */
int speaker_data_init = 0;
#if !WITH_CEREBROD_NO_THREADS
pthread_mutex_t speaker_data_init_lock = PTHREAD_MUTEX_INITIALIZER;
#endif /* !WITH_CEREBROD_NO_THREADS */

/*
 * metric_handle
 * metric_modules_count
 *
 * Handle for metric modules and the count
 */
metric_modules_t metric_handle = NULL;
int metric_handle_count = 0;

/*
 * metric_list
 *
 * Metrics to grab data from
 */
List metric_list = NULL;
int metric_list_size = 0;

#if !WITH_CEREBROD_NO_THREADS
/*
 * metric_list_lock
 *
 * lock to protect pthread access to the metric_list
 */
pthread_mutex_t metric_list_lock = PTHREAD_MUTEX_INITIALIZER;
#endif /* !WITH_CEREBROD_NO_THREADS */

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
  int i;
#if CEREBRO_DEBUG
#if !WITH_CEREBROD_NO_THREADS
  int rv;
#endif /* !WITH_CEREBROD_NO_THREADS */
#endif /* CEREBRO_DEBUG */

  assert(metric_list);

#if CEREBRO_DEBUG
#if !WITH_CEREBROD_NO_THREADS
  /* Should be called with lock already set */
  rv = Pthread_mutex_trylock(&metric_list_lock);
  if (rv != EBUSY)
    CEREBRO_EXIT(("mutex not locked: rv=%d", rv));
#endif /* !WITH_CEREBROD_NO_THREADS */
#endif /* CEREBRO_DEBUG */

  if (!(metric_handle = metric_modules_load()))
    {
      CEREBRO_DBG(("metric_modules_load"));
      goto cleanup;
    }
  
  if ((metric_handle_count = metric_modules_count(metric_handle)) < 0)
    {
      CEREBRO_DBG(("metric_module_count failed"));
      goto cleanup;
    }
  
  if (!metric_handle_count)
    {
#if CEREBRO_DEBUG
      if (conf.debug && conf.speak_debug)
        {
#if !WITH_CEREBROD_NO_THREADS
          Pthread_mutex_lock(&debug_output_mutex);
#endif /* !WITH_CEREBROD_NO_THREADS */
          fprintf(stderr, "**************************************\n");
          fprintf(stderr, "* No Metric Modules Found\n");
          fprintf(stderr, "**************************************\n");
#if !WITH_CEREBROD_NO_THREADS
          Pthread_mutex_unlock(&debug_output_mutex);
#endif /* !WITH_CEREBROD_NO_THREADS */
        }
#endif /* CEREBRO_DEBUG */
      goto cleanup;
    }

  for (i = 0; i < metric_handle_count; i++)
    {
      struct cerebrod_speaker_metric_info *metric_info;
#if !WITH_CEREBROD_NO_THREADS
      Cerebro_metric_thread_pointer threadPtr;
#endif /* !WITH_CEREBROD_NO_THREADS */
      char *module_name, *metric_name;
      int metric_period;
      u_int32_t metric_flags;
      
      module_name = metric_module_name(metric_handle, i);

      if (conf.metric_module_exclude_len)
        {
          int found_exclude = 0;
          int j;

          for (j = 0; j < conf.metric_module_exclude_len; j++)
            {
              if (!strcasecmp(conf.metric_module_exclude[j], module_name))
                {
                  found_exclude++;
                  break;
                }
            }
          
          if (found_exclude)
            {
#if CEREBRO_DEBUG
              if (conf.debug && conf.speak_debug)
                {
#if !WITH_CEREBROD_NO_THREADS
                  Pthread_mutex_lock(&debug_output_mutex);
#endif /* !WITH_CEREBROD_NO_THREADS */
                  fprintf(stderr, "**************************************\n");
                  fprintf(stderr, "* Skip Metric Module: %s\n", module_name);
                  fprintf(stderr, "**************************************\n");
#if !WITH_CEREBROD_NO_THREADS
                  Pthread_mutex_unlock(&debug_output_mutex);
#endif /* !WITH_CEREBROD_NO_THREADS */
                }
#endif /* CEREBRO_DEBUG */
              CEREBRO_ERR(("Dropping metric module: %s", module_name));
              continue;
            }
        }
      
#if CEREBRO_DEBUG
      if (conf.debug && conf.speak_debug)
        {
#if !WITH_CEREBROD_NO_THREADS
          Pthread_mutex_lock(&debug_output_mutex);
#endif /* !WITH_CEREBROD_NO_THREADS */
          fprintf(stderr, "**************************************\n");
          fprintf(stderr, "* Setup Metric Module: %s\n", module_name);
          fprintf(stderr, "**************************************\n");
#if !WITH_CEREBROD_NO_THREADS
          Pthread_mutex_unlock(&debug_output_mutex);
#endif /* !WITH_CEREBROD_NO_THREADS */
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

      if (metric_module_get_metric_flags(metric_handle, i, &metric_flags) < 0)
        {
          CEREBRO_DBG(("metric_module_get_metric_flags: %s", module_name));
          metric_module_cleanup(metric_handle, i);
          continue;
        }

      if (metric_flags & CEREBRO_METRIC_MODULE_FLAGS_SEND_ON_PERIOD
          && metric_period <= 0)
        {
          CEREBRO_DBG(("metric module period invalid: %s", module_name));
          metric_module_cleanup(metric_handle, i);
          continue;
        }

      if (metric_module_send_message_function_pointer(metric_handle, i, &cerebrod_send_message) < 0)
        {
          CEREBRO_DBG(("metric_module_send_message_function_pointer: %s", module_name));
          metric_module_cleanup(metric_handle, i);
          continue;
        }
 
      metric_info = Malloc(sizeof(struct cerebrod_speaker_metric_info));
      /* No need to Strdup() the name in this case */
      metric_info->metric_name = metric_name;
      metric_info->metric_origin = CEREBROD_METRIC_SPEAKER_ORIGIN_MODULE;

      metric_info->metric_period = metric_period;
      metric_info->metric_flags = metric_flags;
      metric_info->index = i;

      /* 
       * If metric period is < 0, it presumably never will be sent
       * (metric is likely handled by a metric_thread), so set
       * next_call_time to UINT_MAX.
       *
       * If this is a metric that will be piggy-backed on heartbeats,
       * then initialize next_call_time to 0, so the data is sent on
       * the first heartbeat 
       *
       * If this is a metric that will not be piggy-backed on
       * heartbeats, set the next_call_time to UINT_MAX.  Let the
       * speaker logic decide when packets should be sent.
       */
      if (metric_info->metric_period < 0
          || metric_info->metric_flags & CEREBRO_METRIC_MODULE_FLAGS_SEND_ON_PERIOD)
        metric_info->next_call_time = UINT_MAX;
      else
        metric_info->next_call_time = 0;
      
      List_append(metric_list, metric_info);
      metric_list_size++;

#if !WITH_CEREBROD_NO_THREADS
      if ((threadPtr = metric_module_get_metric_thread(metric_handle, i)))
        {          
          pthread_t thread;
          pthread_attr_t attr;
          
          Pthread_attr_init(&attr);
          Pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
          Pthread_attr_setstacksize(&attr, CEREBROD_THREAD_STACKSIZE);
          Pthread_create(&thread, &attr, threadPtr, NULL);
          Pthread_attr_destroy(&attr);
        }
#endif /* !WITH_CEREBROD_NO_THREADS */
    }
  
  if (!metric_list_size)
    goto cleanup;

  cerebrod_speaker_data_metric_list_sort();
  return 1;

 cleanup:
  if (metric_handle)
    {
      /* unload will call module cleanup functions */
      metric_modules_unload(metric_handle);
      metric_handle = NULL;
      metric_handle_count = 0;
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
#if !WITH_CEREBROD_NO_THREADS
  Pthread_mutex_lock(&speaker_data_init_lock);
#endif /* !WITH_CEREBROD_NO_THREADS */
  if (speaker_data_init)
    goto out;

#if !WITH_CEREBROD_NO_THREADS
  /* 
   * Must lock in this initialization routine, b/c the update thread
   * in a metric module may call the update state function.
   */
  Pthread_mutex_lock(&metric_list_lock);
#endif /* !WITH_CEREBROD_NO_THREADS */

  metric_list = List_create((ListDelF)_destroy_speaker_metric_info);

  if (_setup_metric_modules() < 0)
    CEREBRO_EXIT(("_setup_metric_modules"));

#if !WITH_CEREBROD_NO_THREADS
  Pthread_mutex_unlock(&metric_list_lock);
#endif /* !WITH_CEREBROD_NO_THREADS */

  speaker_data_init++;
 out:
#if !WITH_CEREBROD_NO_THREADS
  Pthread_mutex_unlock(&speaker_data_init_lock);
#endif /* !WITH_CEREBROD_NO_THREADS */
  ;                             /* in case !WITH_CEREBRO_NO_THREADS */
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

void 
cerebrod_speaker_data_metric_list_sort(void)
{
#if CEREBRO_DEBUG
#if !WITH_CEREBROD_NO_THREADS
  int rv;
#endif /* !WITH_CEREBROD_NO_THREADS */
#endif /* CEREBRO_DEBUG */

  assert(metric_list);

#if CEREBRO_DEBUG
#if !WITH_CEREBROD_NO_THREADS
  /* Should be called with lock already set */
  rv = Pthread_mutex_trylock(&metric_list_lock);
  if (rv != EBUSY)
    CEREBRO_EXIT(("mutex not locked: rv=%d", rv));
#endif /* !WITH_CEREBROD_NO_THREADS */
#endif /* CEREBRO_DEBUG */

  List_sort(metric_list, _next_call_time_cmp);
}

/* 
 * _get_module_metric_value
 *
 * Get the metric value data from a module
 *
 * Returns message metric data on success, NULL otherwise
 */
static struct cerebrod_message_metric *
_get_module_metric_value(unsigned int index)
{
  struct cerebrod_message_metric *mm = NULL;
  char *metric_name;
  u_int32_t mtype, mlen;
  void *mvalue = NULL;

  assert(index < metric_handle_count);

  mm = Malloc(sizeof(struct cerebrod_message_metric));
  memset(mm, '\0', sizeof(struct cerebrod_message_metric));

  if (!(metric_name = metric_module_get_metric_name(metric_handle, index)))
    {
      CEREBRO_DBG(("metric_module_get_metric_name: %d", index));
      goto cleanup;
    }

  /* need not overflow */
  strncpy(mm->metric_name, metric_name, CEREBRO_MAX_METRIC_NAME_LEN);
  
  if (metric_module_get_metric_value(metric_handle,
                                     index,
                                     &mtype,
                                     &mlen,
                                     &mvalue) < 0)
    {
      CEREBRO_DBG(("metric_module_get_metric_value: %d", index));
      goto cleanup;
    }

  if (mtype == CEREBRO_DATA_VALUE_TYPE_STRING 
      && mlen > CEREBRO_MAX_DATA_STRING_LEN)
    {
      CEREBRO_DBG(("truncate metric string: %d", mlen));
      mlen = CEREBRO_MAX_DATA_STRING_LEN;
    }
 
  if (mtype == CEREBRO_DATA_VALUE_TYPE_STRING && !mlen)
    {
      CEREBRO_DBG(("adjusting metric type to none"));
      mtype = CEREBRO_DATA_VALUE_TYPE_NONE;
    }

  if (check_data_type_len_value(mtype, mlen, mvalue) < 0)
    goto cleanup;

  mm->metric_value_type = mtype;
  mm->metric_value_len = mlen;
  if (mm->metric_value_len)
    {
      mm->metric_value = Malloc(mm->metric_value_len);
      memcpy(mm->metric_value, mvalue, mm->metric_value_len);
    }
  else
    mm->metric_value = NULL;
  metric_module_destroy_metric_value(metric_handle, index, mvalue);
  
  return mm;

 cleanup:
  
  if (mvalue)
    metric_module_destroy_metric_value(metric_handle, index, mvalue);

  if (mm)
    {
      if (mm->metric_value)
        Free(mm->metric_value);
      Free(mm);
    }
 
  return NULL;
}

/* 
 * _get_userspace_metric_value
 *
 * Get the metric value data supplied by a userspace program
 *
 * Returns message metric data on success, NULL otherwise
 */
static struct cerebrod_message_metric *
_get_userspace_metric_value(struct cerebrod_speaker_metric_info *metric_info)
{
  struct cerebrod_message_metric *mm = NULL;
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
  if (check_data_type_len_value(mtype, mlen, mvalue) < 0)
    goto cleanup;

  mm = Malloc(sizeof(struct cerebrod_message_metric));
  memset(mm, '\0', sizeof(struct cerebrod_message_metric));

  /* need not overflow */
  strncpy(mm->metric_name, 
          metric_info->metric_name, 
          CEREBRO_MAX_METRIC_NAME_LEN);

  mm->metric_value_type = metric_info->metric_value_type;
  mm->metric_value_len = metric_info->metric_value_len;
  if (mm->metric_value_len)
    {
      mm->metric_value = Malloc(metric_info->metric_value_len);
      memcpy(mm->metric_value, 
             metric_info->metric_value, 
             metric_info->metric_value_len);
    }
  else
    mm->metric_value = NULL;
      
  return mm;

 cleanup:
  if (mm)
    {
      if (mm->metric_value)
        Free(mm->metric_value);
      Free(mm);
    }
  return NULL;
}

void 
cerebrod_speaker_data_get_heartbeat_metric_data(struct cerebrod_message *msg,
                                                unsigned int *message_len,
                                                int *more_data_to_send)
{
  struct cerebrod_speaker_metric_info *metric_info;
  ListIterator itr = NULL;
  struct timeval tv;

  assert(msg && message_len && more_data_to_send);

  if (!speaker_data_init)
    CEREBRO_EXIT(("initialization not complete"));

  *more_data_to_send = 0;

#if !WITH_CEREBROD_NO_THREADS
  Pthread_mutex_lock(&metric_list_lock);
#endif /* !WITH_CEREBROD_NO_THREADS */

  /* There may not be any metrics to distribute */
  if (!metric_list_size)
    {
      msg->metrics_len = 0;
      msg->metrics = NULL;
      goto lock_out;
    }

  msg->metrics_len = 0;
  msg->metrics = Malloc(sizeof(struct cerebrod_message_metric *)*(metric_list_size + 1));
  memset(msg->metrics, '\0', sizeof(struct cerebrod_message_metric *)*(metric_list_size + 1));

  Gettimeofday(&tv, NULL);
  
  itr = List_iterator_create(metric_list);
  while ((metric_info = list_next(itr)))
    {      
      struct cerebrod_message_metric *mm = NULL;

      if (tv.tv_sec <= metric_info->next_call_time)
        break;

      if (metric_info->metric_origin & CEREBROD_METRIC_SPEAKER_ORIGIN_MODULE
          && !(metric_info->metric_flags & CEREBRO_METRIC_MODULE_FLAGS_SEND_ON_PERIOD))
	mm = _get_module_metric_value(metric_info->index);

      if (metric_info->metric_origin & CEREBROD_METRIC_SPEAKER_ORIGIN_USERSPACE)
        mm = _get_userspace_metric_value(metric_info);

      if (mm)
        {
          unsigned int new_len;

          new_len = *message_len + CEREBROD_MESSAGE_METRIC_HEADER_LEN + mm->metric_value_len;
          if (new_len >= CEREBRO_MAX_PACKET_LEN)
            {
              *more_data_to_send = 1;
              goto sort_out;
            } 
          else
            {
              *message_len += CEREBROD_MESSAGE_METRIC_HEADER_LEN;
              *message_len += mm->metric_value_len;
              msg->metrics[msg->metrics_len] = mm;
              msg->metrics_len++;
            }
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

      if (metric_info->metric_origin & CEREBROD_METRIC_SPEAKER_ORIGIN_USERSPACE)
        metric_info->next_call_time = UINT_MAX;
    } 
 sort_out:
  List_iterator_destroy(itr);
  cerebrod_speaker_data_metric_list_sort();
 lock_out:
#if !WITH_CEREBROD_NO_THREADS
  Pthread_mutex_unlock(&metric_list_lock);
#endif /* !WITH_CEREBROD_NO_THREADS */
  return;
}

void 
cerebrod_speaker_data_get_module_metric_data(struct cerebrod_message *msg,
                                             unsigned int *message_len,
                                             unsigned int index,
                                             int *more_data_to_send)
{
  struct cerebrod_message_metric *mm = NULL;

  assert(msg && message_len && more_data_to_send);
  
  assert(metric_handle && metric_list && metric_list_size);

  if (!speaker_data_init)
    CEREBRO_EXIT(("initialization not complete"));

  *more_data_to_send = 0;

#if !WITH_CEREBROD_NO_THREADS
  Pthread_mutex_lock(&metric_list_lock);
#endif /* !WITH_CEREBROD_NO_THREADS */

  msg->metrics_len = 0;
  msg->metrics = Malloc(sizeof(struct cerebrod_message_metric *)*2);
  memset(msg->metrics, '\0', sizeof(struct cerebrod_message_metric *)*2);

  /* Since this is the only metric value we're sending, we should
   * never have more data than can fit in a single UDP packet.
   */
  if ((mm = _get_module_metric_value(index)))
    {
      *message_len += CEREBROD_MESSAGE_METRIC_HEADER_LEN;
      *message_len += mm->metric_value_len;
      msg->metrics[msg->metrics_len] = mm;
      msg->metrics_len++;
    }
  
#if !WITH_CEREBROD_NO_THREADS
  Pthread_mutex_unlock(&metric_list_lock);
#endif /* !WITH_CEREBROD_NO_THREADS */
  return;
}
