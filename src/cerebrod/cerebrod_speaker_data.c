/*****************************************************************************\
 *  $Id: cerebrod_speaker_data.c,v 1.20 2005-07-11 17:44:51 achu Exp $
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

#include "cerebrod.h"
#include "cerebrod_config.h"
#include "cerebrod_speaker_data.h"

#include "debug.h"
#include "metric_module.h"
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

  metric_list = List_create((ListDelF)_Free);

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

      if ((metric_period = metric_module_get_metric_period(metric_handle, i)) < 0)
        {
          CEREBRO_DBG(("metric_module_get_metric_period: %s", module_name));
          metric_module_cleanup(metric_handle, i);
          continue;
        }
 
      metric_info = Malloc(sizeof(struct cerebrod_speaker_metric_info));
      metric_info->metric_name = metric_name;
      metric_info->metric_origin = CEREBROD_METRIC_ORIGIN_MODULE;
      metric_info->metric_period = metric_period;
      metric_info->index = i;
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
          Pthread_create(&thread, &attr, threadPtr, NULL);
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
  if (metric_list)
    {
      list_destroy(metric_list);
      metric_list = NULL;
    }
  metric_list_size = 0;
  return 0;
}

void
cerebrod_speaker_data_initialize(void)
{
  pthread_mutex_lock(&speaker_data_init_lock);
  if (speaker_data_init)
    goto out;

  if (_setup_metric_modules() < 0)
    CEREBRO_EXIT(("_setup_metric_modules"));

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
 * Get the metric value data from a module and store it in the
 * heartbeat
 *
 * Returns heartbeat metric data on success, NULL otherwise
 */
static struct cerebrod_heartbeat_metric *
_get_module_metric_value(struct cerebrod_speaker_metric_info *metric_info)
{
  struct cerebrod_heartbeat_metric *hd = NULL;
  void *temp_value = NULL;
  char *metric_name;
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
                                     &(hd->metric_value_type),
                                     &(hd->metric_value_len),
                                     &temp_value) < 0)
    {
      CEREBRO_DBG(("metric_module_get_metric_value: %d", index));
      goto cleanup;
    }

  if (hd->metric_value_type == CEREBRO_METRIC_VALUE_TYPE_NONE  
      && !hd->metric_value_len)
    {
      CEREBRO_DBG(("bogus metric information: %d", index));
      goto cleanup;
    }
  
  if (!(hd->metric_value_type >= CEREBRO_METRIC_VALUE_TYPE_NONE
        && hd->metric_value_type <= CEREBRO_METRIC_VALUE_TYPE_STRING))
    {
      CEREBRO_DBG(("bogus metric: index=%d", index));
      goto cleanup;
    }

  if ((hd->metric_value_type == CEREBRO_METRIC_VALUE_TYPE_INT32
       && hd->metric_value_len != sizeof(int32_t))
      || (hd->metric_value_type == CEREBRO_METRIC_VALUE_TYPE_U_INT32
          && hd->metric_value_len != sizeof(u_int32_t))
      || (hd->metric_value_type == CEREBRO_METRIC_VALUE_TYPE_FLOAT
          && hd->metric_value_len != sizeof(float))
      || (hd->metric_value_type == CEREBRO_METRIC_VALUE_TYPE_DOUBLE
          && hd->metric_value_len != sizeof(double)))
    {
      CEREBRO_DBG(("bogus metric: index=%d", index));
      goto cleanup;
    }
  
  if (hd->metric_value_type == CEREBRO_METRIC_VALUE_TYPE_STRING
      && hd->metric_value_len > CEREBRO_MAX_METRIC_STRING_LEN)
    {
      CEREBRO_DBG(("truncate metric string: %d", hd->metric_value_len));
      hd->metric_value_len = CEREBRO_MAX_METRIC_STRING_LEN;
    }
  
  hd->metric_value = Malloc(hd->metric_value_len);
  memcpy(hd->metric_value, temp_value, hd->metric_value_len);

  metric_module_destroy_metric_value(metric_handle, index, temp_value);
  
  return hd;

 cleanup:
  
  if (temp_value)
    metric_module_destroy_metric_value(metric_handle, index, temp_value);

  if (hd)
    {
      if (hd->metric_value)
        Free(hd->metric_value);
      Free(hd);
    }
 
  return NULL;
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

  /* There may not be any metric modules */
  if (!metric_handle || !metric_list || !metric_list_size)
    {
      hb->metrics_len = 0;
      hb->metrics = NULL;
      return;
    }

  hb->metrics_len = 0;
  hb->metrics = Malloc(sizeof(struct cerebrod_heartbeat_metric *)*(metric_list_size + 1));
  memset(hb->metrics, '\0', sizeof(struct cerebrod_heartbeat_metric *)*(metric_list_size + 1));

  Gettimeofday(&tv, NULL);
  
  Pthread_mutex_lock(&metric_list_lock);
  itr = List_iterator_create(metric_list);
  while ((metric_info = list_next(itr)))
    {      
      struct cerebrod_heartbeat_metric *hd = NULL;

      if (tv.tv_sec <= metric_info->next_call_time)
        break;

      if (metric_info->metric_origin & CEREBROD_METRIC_ORIGIN_MODULE)
        hd = _get_module_metric_value(metric_info);

      if (hd)
        {
          *heartbeat_len += CEREBROD_HEARTBEAT_METRIC_HEADER_LEN;
          *heartbeat_len += hd->metric_value_len;
          hb->metrics[hb->metrics_len] = hd;
          hb->metrics_len++;
        }

      /* 
       * Metric period stays at 0 for metrics that need to be
       * propogated every time
       */
      if (metric_info->metric_period)
        metric_info->next_call_time = tv.tv_sec + metric_info->metric_period;
    } 
  List_iterator_destroy(itr);
  List_sort(metric_list, _next_call_time_cmp);
  Pthread_mutex_unlock(&metric_list_lock);

  return;
}
