/*****************************************************************************\
 *  $Id: cerebrod_speaker_data.c,v 1.1 2005-06-21 19:16:56 achu Exp $
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
#include "cerebro/cerebro_error.h"
#include "cerebro/cerebrod_heartbeat_protocol.h"

#include "cerebrod.h"
#include "cerebrod_config.h"
#include "cerebrod_speaker_data.h"
#include "cerebrod_wrappers.h"

#include "metric_module.h"

extern struct cerebrod_config conf;
#if CEREBRO_DEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* CEREBRO_DEBUG */

/*
 * cerebrod_speaker_data_initialization_complete
 * cerebrod_speaker_data_initialization_complete_lock
 *
 * variables for synchronizing initialization between different pthreads
 */
int cerebrod_speaker_data_initialization_complete = 0;
pthread_mutex_t cerebrod_speaker_data_initialization_complete_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * metric_handle
 *
 * Handle for metric modules;
 */
metric_modules_t metric_handle = NULL;

/*
 * metric_list
 *
 * Metric modules to grab data from
 */
List metric_list;
int metric_list_size = 0;

/*
 * _cerebrod_speaker_metric_module_destroy
 *
 * Destroy a cerebrod_speaker_metric_module structure
 */
static void
_cerebrod_speaker_metric_module_destroy(void *x)
{
  struct cerebrod_speaker_metric_module *metric_module;

  assert(x);

  metric_module = (struct cerebrod_speaker_metric_module *)x;
  Free(metric_module);
}

void
cerebrod_speaker_data_initialize(void)
{
  int i, metric_index_len, numnodes = 0;

  pthread_mutex_lock(&cerebrod_speaker_data_initialization_complete_lock);
  if (cerebrod_speaker_data_initialization_complete)
    goto out;

  /*
   * Load and Setup metric modules
   */

  if (!(metric_handle = metric_modules_load(conf.metric_max)))
    {
      cerebro_err_debug("%s(%s:%d): metric_modules_load",
                        __FILE__, __FUNCTION__, __LINE__);
      return;
    }

  if ((metric_index_len = metric_modules_count(metric_handle)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): metric_module_count failed",
                        __FILE__, __FUNCTION__, __LINE__);
      metric_modules_unload(metric_handle);
      metric_handle = NULL;
      return;
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
      metric_modules_unload(metric_handle);
      metric_handle = NULL;
      goto done;
    }

  metric_list = List_create((ListDelF)_cerebrod_speaker_metric_module_destroy);

  for (i = 0; i < metric_index_len; i++)
    {
      struct cerebrod_speaker_metric_module *metric_module;
      char *metric_name;
      int metric_period;

#if CEREBRO_DEBUG
      if (conf.debug && conf.speak_debug)
        {
          Pthread_mutex_lock(&debug_output_mutex);
          fprintf(stderr, "**************************************\n");
          fprintf(stderr, "* Settup up Metric Module: %s\n",
                  metric_module_name(metric_handle, i));
          fprintf(stderr, "**************************************\n");
          Pthread_mutex_unlock(&debug_output_mutex);
        }
#endif /* CEREBRO_DEBUG */
      if (metric_module_setup(metric_handle, i) < 0)
        {
          cerebro_err_debug("%s(%s:%d): metric_module_setup failed: "
                            "metric_module = %s",
                            __FILE__, __FUNCTION__, __LINE__,
                            metric_module_name(metric_handle, i));
          continue;
        }

      if (!(metric_name = metric_module_get_metric_name(metric_handle,
                                                        i)))
        {
          cerebro_err_debug("%s(%s:%d): metric_module_get_metric_name failed",
                            __FILE__, __FUNCTION__, __LINE__);
          metric_module_cleanup(metric_handle, i);
          continue;
        }

      if ((metric_period = metric_module_get_metric_period(metric_handle,
                                                        i)) < 0)
        {
          cerebro_err_debug("%s(%s:%d): metric_module_get_metric_period failed",
                            __FILE__, __FUNCTION__, __LINE__);
          metric_module_cleanup(metric_handle, i);
          continue;
        }

      metric_module = Malloc(sizeof(struct cerebrod_metric));
      metric_module->metric_name = metric_name;
      metric_module->metric_period = metric_period;
      metric_module->index = i;
      /* Initialize to 0, so data is sent on the first heartbeat */
      metric_module->next_call_time = 0;
      List_append(metric_list, metric);
      metric_list_size++;
    }
  
  if (!metric_list_size)
    goto metric_cleanup;

  goto done;

 metric_cleanup:
  /* unload will call module cleanup functions */
  metric_modules_unload(metric_handle);
  list_destroy(metric_list);
  metric_handle = NULL;
  metric_list = NULL;
  metric_list_size = 0;

 done:
  cerebrod_speaker_data_initialization_complete++;
 out:
  Pthread_mutex_unlock(&cerebrod_speaker_data_initialization_complete_lock);
}

void 
cerebrod_speaker_data_get_metric_data(struct cerebrod_heartbeat *hb)
{
  ListIterator itr = NULL;
  int hb_metric_count = 0;

  assert(hb);

  if (!cerebrod_speaker_data_initialization_complete)
    cerebro_err_exit("%s(%s:%d): initialization not complete",
                     __FILE__, __FUNCTION__, __LINE__);

  /* Could be no metric modules */
  if (!metric_handle || !metric_list || !metric_list_size)
    return;

  
  
}
