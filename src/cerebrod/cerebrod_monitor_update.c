/*****************************************************************************\
 *  $Id: cerebrod_monitor_update.c,v 1.9 2010-02-02 01:01:20 chu11 Exp $
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
#include <errno.h>
#include <assert.h>

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"

#include "cerebrod_config.h"
#include "cerebrod_debug.h"
#include "cerebrod_monitor_update.h"
#include "cerebrod_util.h"

#include "debug.h"
#include "hash.h"
#include "monitor_module.h"
#include "wrappers.h"

#define LISTENER_DATA_SIZE_DEFAULT              1024
#define LISTENER_DATA_SIZE_INCREMENT            1024

#define LISTENER_DATA_METRIC_NAMES_SIZE_DEFAULT 32
#define LISTENER_DATA_METRIC_NAMES_INCREMENT    32

#define LISTENER_DATA_METRIC_DATA_SIZE_DEFAULT  32
#define LISTENER_DATA_METRIC_DATA_INCREMENT     32

extern struct cerebrod_config conf;
extern pthread_mutex_t debug_output_mutex;

extern pthread_mutex_t listener_data_init_lock;

/*
 * monitor_handle
 *
 * monitoring module handles
 */
monitor_modules_t monitor_handle = NULL;

/*
 * monitor_index
 *
 * hash index to quickly determine what metrics are being
 * monitored by modules and what index they are.
 */
hash_t monitor_index = NULL;

/*
 * _cerebrod_monitor_module_info_destroy
 *
 * Destroy a monitor_module_info struct.
 */
static void
_cerebrod_monitor_module_info_destroy(void *data)
{
  struct cerebrod_monitor_module_info *monitor_module;

  assert(data);

  monitor_module = (struct cerebrod_monitor_module_info *)data;
  Free(monitor_module->metric_names);
  Free(monitor_module);
}

/*
 * _cerebrod_monitor_module_list_destroy
 *
 * Destroy a monitor_module_list struct.
 */
static void
_cerebrod_monitor_module_list_destroy(void *data)
{
  struct cerebrod_monitor_module_list *ml;

  assert(data);

  ml = (struct cerebrod_monitor_module_list *)data;
  List_destroy(ml->monitor_list);
  Free(ml);
}

/*
 * Under almost any circumstance, don't return a -1 error, cerebro can
 * go on without loading monitor modules. The listener_data_init_lock
 * should already be set.
 */
int
cerebrod_monitor_modules_setup(void)
{
  int i, monitor_module_count, monitor_index_len, monitor_index_count = 0;
  struct cerebrod_monitor_module_list *ml = NULL;
#if CEREBRO_DEBUG
  int rv;
#endif /* CEREBRO_DEBUG */

#if CEREBRO_DEBUG
  /* Should be called with lock already set */
  rv = Pthread_mutex_trylock(&listener_data_init_lock);
  if (rv != EBUSY)
    CEREBROD_EXIT(("mutex not locked: rv=%d", rv));
#endif /* CEREBRO_DEBUG */

  if (!(monitor_handle = monitor_modules_load()))
    {
      CEREBROD_DBG(("monitor_modules_load"));
      goto cleanup;
    }

  if ((monitor_module_count = monitor_modules_count(monitor_handle)) < 0)
    {
      CEREBROD_DBG(("monitor_modules_count"));
      goto cleanup;
    }

  if (!monitor_module_count)
    {
      if (conf.debug && conf.listen_debug)
        {
          Pthread_mutex_lock(&debug_output_mutex);
          fprintf(stderr, "**************************************\n");
          fprintf(stderr, "* No Monitor Modules Found\n");
          fprintf(stderr, "**************************************\n");
          Pthread_mutex_unlock(&debug_output_mutex);
        }
      goto cleanup;
    }

  /* Each monitor module may wish to monitor multiple metrics.  We'll
   * assume there will never be more than 2 metrics per monitor module, and
   * that will be enough to avoid all hash collisions.
   */
  monitor_index_len = monitor_module_count * 2;

  monitor_index = Hash_create(monitor_index_len,
                              (hash_key_f)hash_key_string,
                              (hash_cmp_f)strcmp,
                              (hash_del_f)_cerebrod_monitor_module_list_destroy);

  for (i = 0; i < monitor_module_count; i++)
    {
      struct cerebrod_monitor_module_info *monitor_module;
      char *module_name, *metric_names;
      char *metricPtr, *metricbuf;

      module_name = monitor_module_name(monitor_handle, i);

      if (conf.monitor_module_exclude_len)
        {
          int found_exclude = 0;
          int j;

          for (j = 0; j < conf.monitor_module_exclude_len; j++)
            {
              if (!strcasecmp(conf.monitor_module_exclude[j], module_name))
                {
                  found_exclude++;
                  break;
                }
            }

          if (found_exclude)
            {
              if (conf.debug && conf.listen_debug)
                {
                  Pthread_mutex_lock(&debug_output_mutex);
                  fprintf(stderr, "**************************************\n");
                  fprintf(stderr, "* Skip Monitor Module: %s\n", module_name);
                  fprintf(stderr, "**************************************\n");
                  Pthread_mutex_unlock(&debug_output_mutex);
                }
              CEREBROD_ERR(("Dropping monitor module: %s", module_name));
              continue;
            }
        }

      if (conf.debug && conf.listen_debug)
        {
          Pthread_mutex_lock(&debug_output_mutex);
          fprintf(stderr, "**************************************\n");
          fprintf(stderr, "* Setup Monitor Module: %s\n", module_name);
          fprintf(stderr, "**************************************\n");
          Pthread_mutex_unlock(&debug_output_mutex);
        }

      if (monitor_module_setup(monitor_handle, i) < 0)
        {
          CEREBROD_DBG(("monitor_module_setup failed: %s", module_name));
          continue;
        }

      if (!(metric_names = monitor_module_metric_names(monitor_handle, i)))
        {
          CEREBROD_DBG(("monitor_module_metric_names failed: %s", module_name));
          monitor_module_cleanup(monitor_handle, i);
          continue;
        }

      monitor_module = Malloc(sizeof(struct cerebrod_monitor_module_info));
      monitor_module->metric_names = Strdup(metric_names);
      monitor_module->index = i;
      Pthread_mutex_init(&(monitor_module->monitor_lock), NULL);

      /* The monitoring module may support multiple metrics */

      metricPtr = strtok_r(monitor_module->metric_names, ",", &metricbuf);
      while (metricPtr)
        {
          if (!(ml = Hash_find(monitor_index, metricPtr)))
            {
              ml = (struct cerebrod_monitor_module_list *)Malloc(sizeof(struct cerebrod_monitor_module_list));
              ml->monitor_list = List_create((ListDelF)_cerebrod_monitor_module_info_destroy);
              Pthread_mutex_init(&(ml->monitor_list_lock), NULL);

              List_append(ml->monitor_list, monitor_module);
              Hash_insert(monitor_index, metricPtr, ml);
              monitor_index_count++;
            }
          else
            List_append(ml->monitor_list, monitor_module);

          metricPtr = strtok_r(NULL, ",", &metricbuf);
        }
    }

  if (!monitor_index_count)
    goto cleanup;

  return 1;

 cleanup:
  if (monitor_handle)
    {
      monitor_modules_unload(monitor_handle);
      monitor_handle = NULL;
    }
  if (monitor_index)
    {
      Hash_destroy(monitor_index);
      monitor_index = NULL;
    }
  return 0;
}

/*
 * cerebrod_monitor_modules_update
 *
 * Send metric data to the appropriate monitor modules, if necessary.
 * The struct cerebrod_node_data lock should already be locked.
 */
void
cerebrod_monitor_modules_update(const char *nodename,
                                struct cerebrod_node_data *nd,
                                const char *metric_name,
                                struct cerebrod_message_metric *mm)
{
  struct cerebrod_monitor_module_info *monitor_module;
  struct cerebrod_monitor_module_list *ml;
#if CEREBRO_DEBUG
  int rv;
#endif /* CEREBRO_DEBUG */

  assert(nodename && nd && metric_name && mm);

  if (!monitor_index)
    return;

#if CEREBRO_DEBUG
  /* Should be called with lock already set */
  rv = Pthread_mutex_trylock(&nd->node_data_lock);
  if (rv != EBUSY)
    CEREBROD_EXIT(("mutex not locked: rv=%d", rv));
#endif /* CEREBRO_DEBUG */

  if ((ml = Hash_find(monitor_index, metric_name)))
    {
      ListIterator itr = NULL;

      Pthread_mutex_lock(&(ml->monitor_list_lock));
      itr = List_iterator_create(ml->monitor_list);
      Pthread_mutex_unlock(&(ml->monitor_list_lock));

      while ((monitor_module = list_next(itr)))
	{
	  Pthread_mutex_lock(&monitor_module->monitor_lock);
	  monitor_module_metric_update(monitor_handle,
				       monitor_module->index,
				       nodename,
				       metric_name,
				       mm->metric_value_type,
				       mm->metric_value_len,
				       mm->metric_value);
	  Pthread_mutex_unlock(&monitor_module->monitor_lock);
	}

      Pthread_mutex_lock(&(ml->monitor_list_lock));
      List_iterator_destroy(itr);
      Pthread_mutex_unlock(&(ml->monitor_list_lock));
    }
}

#endif /* !WITH_CEREBROD_SPEAKER_ONLY */

