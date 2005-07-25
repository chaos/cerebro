/*****************************************************************************\
 *  $Id: cerebrod_listener_data.c,v 1.23 2005-07-25 17:20:18 achu Exp $
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

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"

#include "cerebrod_config.h"
#include "cerebrod_listener_data.h"
#include "cerebrod_util.h"

#include "clusterlist_module.h"
#include "debug.h"
#include "hash.h"
#include "list.h"
#include "monitor_module.h"
#include "wrappers.h"

#define LISTENER_DATA_INDEX_SIZE_DEFAULT   1024
#define LISTENER_DATA_INDEX_SIZE_INCREMENT 1024
#define LISTENER_DATA_REHASH_LIMIT         (listener_data_index_size*2)

extern struct cerebrod_config conf;
#if CEREBRO_DEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* CEREBRO_DEBUG */

extern clusterlist_module_t clusterlist_handle;

/*
 * listener_data_init
 * listener_data_init_lock
 *
 * variables for synchronizing initialization between different pthreads
 */
int listener_data_init = 0;
pthread_mutex_t listener_data_init_lock = PTHREAD_MUTEX_INITIALIZER;

/* 
 * listener_data_list
 *
 * contains list of nodes and cluster_data
 */
List listener_data_list = NULL;

/*
 * listener_data_index
 * listener_data_index_numnodes
 * listener_data_index_size
 *
 * hash index into cerebrod_listener_data list for faster access, number of
 * currently indexed entries, and index size
 */
hash_t listener_data_index = NULL;
int listener_data_index_numnodes;
int listener_data_index_size;

/*  
 * listener_data_lock
 *
 * lock to protect pthread access to both the cluster_data list and
 * cluster_data_index
 */
pthread_mutex_t listener_data_lock = PTHREAD_MUTEX_INITIALIZER;

/* 
 * metric_names_index
 *
 * contains list of currently known metrics
 */
hash_t metric_names_index = NULL;

/* 
 * metric_name_defaults
 * metric_name_defaults_count
 *
 * Default metrics, which don't count against the maximum
 * number of metrics that can be monitored.
 */
char *metric_name_defaults[] = {
  CEREBRO_METRIC_METRIC_NAMES,
  CEREBRO_METRIC_CLUSTER_NODES,
  CEREBRO_METRIC_UPDOWN_STATE,
  NULL
};
int metric_name_defaults_count = 3;

/*  
 * metric_names_lock
 *
 * lock to protect pthread access to the metric_name index
 *
 * Locking rule: Can be locked within a struct cerebrod_node_data
 * node_data_lock locked region.
 */
pthread_mutex_t metric_names_lock = PTHREAD_MUTEX_INITIALIZER;

/* 
 * monitor_handle
 *
 * monitoring module handles
 */
monitor_modules_t monitor_handle = NULL;

/*
 * monitor_index
 * monitor_index_size
 *
 * hash index to quickly determine what metrics are being
 * monitored by modules and what index they are.
 */
hash_t monitor_index = NULL;
int monitor_index_size = 0;

/* 
 * _cerebrod_node_data_strcmp
 *
 * callback function for list_sort to sort node names
 */
static int 
_cerebrod_node_data_strcmp(void *x, void *y)
{
  struct cerebrod_node_data *a = x;
  struct cerebrod_node_data *b = y;
  
  assert(x && y);

  return strcmp(a->nodename, b->nodename);
}

/* 
 * _cerebrod_node_data_create_and_init
 *
 * Create and initialize a cerebrod_node_data 
 *
 */
static struct cerebrod_node_data *
_cerebrod_node_data_create_and_init(const char *nodename)
{
  struct cerebrod_node_data *nd;

  assert(nodename);

  nd = Malloc(sizeof(struct cerebrod_node_data));

  nd->nodename = Strdup(nodename);
  nd->discovered = 0;
  nd->last_received_time = 0;
  Pthread_mutex_init(&(nd->node_data_lock), NULL);
  if (conf.metric_server)
    nd->metric_data = Hash_create(conf.metric_max, 
                                  (hash_key_f)hash_key_string,
                                  (hash_cmp_f)strcmp, 
                                  (hash_del_f)NULL);
  else
    nd->metric_data = NULL;
  nd->metric_data_count = 0;
  return nd;
}

/* 
 * _setup_monitor_modules
 * 
 * Setup monitor modules.  Under almost any circumstance, don't return
 * a -1 error, cerebro can go on without loading monitor modules.
 *
 * Return 1 if modules are loaded, 0 if not, -1 on error
 */
static int
_setup_monitor_modules(void)
{
  int i, monitor_index_len;

  if (!(monitor_handle = monitor_modules_load(conf.monitor_max)))
    {
      CEREBRO_DBG(("monitor_modules_load"));
      goto monitor_cleanup;
    }

  if ((monitor_index_len = monitor_modules_count(monitor_handle)) < 0)
    {
      CEREBRO_DBG(("monitor_modules_count"));
      goto monitor_cleanup;
    }

  if (!monitor_index_len)
    {
#if CEREBRO_DEBUG
      if (conf.debug && conf.listen_debug)
        {
          Pthread_mutex_lock(&debug_output_mutex);
          fprintf(stderr, "**************************************\n");
          fprintf(stderr, "* No Monitor Modules Found\n");
          fprintf(stderr, "**************************************\n");
          Pthread_mutex_unlock(&debug_output_mutex);
        }
#endif /* CEREBRO_DEBUG */
      goto monitor_cleanup;
    }
  
  monitor_index = Hash_create(monitor_index_len, 
                              (hash_key_f)hash_key_string, 
                              (hash_cmp_f)strcmp, 
                              (hash_del_f)_Free);

  for (i = 0; i < monitor_index_len; i++)
    {
      struct cerebrod_monitor_module *monitor_module;
      char *module_name, *metric_name;

      module_name = monitor_module_name(monitor_handle, i);

#if CEREBRO_DEBUG
      if (conf.debug && conf.listen_debug)
        {
          Pthread_mutex_lock(&debug_output_mutex);
          fprintf(stderr, "**************************************\n");
          fprintf(stderr, "* Settup up Monitor Module: %s\n", module_name);
          fprintf(stderr, "**************************************\n");
          Pthread_mutex_unlock(&debug_output_mutex);
        }
#endif /* CEREBRO_DEBUG */

      if (monitor_module_setup(monitor_handle, i) < 0)
        {
          CEREBRO_DBG(("monitor_module_setup failed: %s", module_name));
          continue;
        }

      if (!(metric_name = monitor_module_metric_name(monitor_handle, i)) < 0)
        {
          CEREBRO_DBG(("monitor_module_metric_name failed: %s", module_name));
          monitor_module_cleanup(monitor_handle, i);
          continue;
        }

      monitor_module = Malloc(sizeof(struct cerebrod_monitor_module));
      monitor_module->metric_name = metric_name;
      monitor_module->index = i;
      Pthread_mutex_init(&(monitor_module->monitor_lock), NULL);

      Hash_insert(monitor_index, monitor_module->metric_name, monitor_module);
      monitor_index_size++;
    }

  if (!monitor_index_size)
    goto monitor_cleanup;

  return 1;

 monitor_cleanup:
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
  monitor_index_size = 0;
  return 0;
}

/* 
 * _metric_name_data_destroy
 *
 * destroy metric name data
 */
static void
_metric_name_data_destroy(void *data)
{
  struct cerebrod_metric_name_data *mnd;

  assert(data);
  
  mnd = (struct cerebrod_metric_name_data *)data;

  if (mnd->metric_origin & CEREBROD_METRIC_LISTENER_ORIGIN_MONITORED)
    Free(mnd->metric_name);

  Free(data);
}

void 
cerebrod_listener_data_initialize(void)
{
  int numnodes = 0;

  pthread_mutex_lock(&listener_data_init_lock);
  if (listener_data_init)
    goto out;
  
  if (!conf.listen)
    CEREBRO_EXIT(("listen server not setup"));

  if (!clusterlist_handle)
    CEREBRO_EXIT(("clusterlist_handle null"));

  if ((numnodes = clusterlist_module_numnodes(clusterlist_handle)) < 0)
    CEREBRO_EXIT(("clusterlist_module_numnodes"));

  if (numnodes > 0)
    {
      listener_data_index_numnodes = numnodes;
      listener_data_index_size = numnodes;
    }
  else
    {
      listener_data_index_numnodes = 0;
      listener_data_index_size = LISTENER_DATA_INDEX_SIZE_DEFAULT;
    }

  listener_data_list = List_create((ListDelF)NULL);
  listener_data_index = Hash_create(listener_data_index_size,
                                    (hash_key_f)hash_key_string,
                                    (hash_cmp_f)strcmp,
                                    (hash_del_f)NULL);
  
  /* If the clusterlist module contains nodes, retrieve all of these
   * nodes and put them into the cerebrod_listener_data list.  All updates
   * will involve updating data rather than involve insertion.
   */
  if (numnodes > 0)
    {
      int i;
      char **nodes;

      if (clusterlist_module_get_all_nodes(clusterlist_handle, &nodes) < 0)
        CEREBRO_EXIT(("clusterlist_module_get_all_nodes"));

      for (i = 0; i < numnodes; i++)
        {
          struct cerebrod_node_data *nd;
          
          nd = _cerebrod_node_data_create_and_init(nodes[i]);

          List_append(listener_data_list, nd);
          Hash_insert(listener_data_index, nd->nodename, nd);

          free(nodes[i]);
        }

      List_sort(listener_data_list, (ListCmpF)_cerebrod_node_data_strcmp);

      free(nodes);
    }

  if (conf.metric_server)
    {
      int i, size;
      
      size = metric_name_defaults_count + conf.metric_max;
      metric_names_index = Hash_create(size, 
                                       (hash_key_f)hash_key_string, 
                                       (hash_cmp_f)strcmp, 
                                       (hash_del_f)_metric_name_data_destroy);

      for (i = 0; i < metric_name_defaults_count; i++)
        {
          struct cerebrod_metric_name_data *mnd;

          mnd = Malloc(sizeof(struct cerebrod_metric_name_data));
          memset(mnd, '\0', sizeof(struct cerebrod_metric_name_data));

          if (strlen(metric_name_defaults[i]) > CEREBRO_MAX_METRIC_NAME_LEN)
            CEREBRO_EXIT(("default metric name length invalid"));

          mnd->metric_name = metric_name_defaults[i];
          mnd->metric_origin = CEREBROD_METRIC_LISTENER_ORIGIN_DEFAULT;

          Hash_insert(metric_names_index, mnd->metric_name, mnd);
        }
    }

  if (_setup_monitor_modules() < 0)
    CEREBRO_EXIT(("_setup_metric_modules"));

  listener_data_init++;
 out:
  Pthread_mutex_unlock(&listener_data_init_lock);
}

/*  
 * _output_node_data_insert
 *
 * Output debugging info about a recently inserted node
 */
static void
_output_node_data_insert(struct cerebrod_node_data *nd)
{
#if CEREBRO_DEBUG
  assert(nd);
 
  if (!(conf.debug && conf.listen_debug))
    return;

  Pthread_mutex_lock(&debug_output_mutex);
  fprintf(stderr, "**************************************\n");
  fprintf(stderr, "* Insert: Node=%s\n", nd->nodename);
  fprintf(stderr, "**************************************\n");
  Pthread_mutex_unlock(&debug_output_mutex);
#endif /* CEREBRO_DEBUG */
}

/*  
 * _output_node_data_update
 *
 * Output debugging info about a recently updated node
 */
static void
_output_node_data_update(struct cerebrod_node_data *nd)
{
#if CEREBRO_DEBUG
  assert(nd);
 
  if (!(conf.debug && conf.listen_debug))
    return;

  Pthread_mutex_lock(&debug_output_mutex);
  fprintf(stderr, "**************************************\n");
  fprintf(stderr, "* Update: Node=%s\n", nd->nodename);
  fprintf(stderr, "**************************************\n");
  Pthread_mutex_unlock(&debug_output_mutex);
#endif /* CEREBRO_DEBUG */
}

#if CEREBRO_DEBUG
/*
 * _metric_data_dump
 *
 * callback function from hash_for_each to dump metric node data
 */
static int
_metric_data_dump(void *data, const void *key, void *arg)
{
  struct cerebrod_listener_metric_data *md;
  char *nodename, *buf;
 
  assert(data && key && arg);
 
  md = (struct cerebrod_listener_metric_data *)data;
  nodename = (char *)arg;
 
  fprintf(stderr, "* %s: metric name=%s type=%d len=%d ",
          nodename, md->metric_name, md->metric_value_type, 
          md->metric_value_len);

  if (md->metric_value_type == CEREBRO_METRIC_VALUE_TYPE_INT32)
    fprintf(stderr, "metric_value=%d", *((int32_t *)md->metric_value));
  else if (md->metric_value_type == CEREBRO_METRIC_VALUE_TYPE_U_INT32)
    fprintf(stderr, "metric_value=%u", *((u_int32_t *)md->metric_value));
  else if (md->metric_value_type == CEREBRO_METRIC_VALUE_TYPE_FLOAT)
    fprintf(stderr, "metric_value=%f", *((float *)md->metric_value));
  else if (md->metric_value_type == CEREBRO_METRIC_VALUE_TYPE_DOUBLE)
    fprintf(stderr, "metric_value=%f", *((double *)md->metric_value));
  else if (md->metric_value_type == CEREBRO_METRIC_VALUE_TYPE_STRING)
    {
      /* Watch for NUL termination */
      buf = Malloc(md->metric_value_len + 1);
      memset(buf, '\0', md->metric_value_len + 1);
      memcpy(buf, md->metric_value, md->metric_value_len);
      fprintf(stderr, "metric_value=%s", buf);
      Free(buf);
    }
  fprintf(stderr, "\n");

  return 1;
}

/*
 * _node_data_dump
 *
 * callback function from list_for_each to dump node data
 */
static int
_node_data_dump(void *x, void *arg)
{
  struct cerebrod_node_data *nd;

  assert(x);
 
  nd = (struct cerebrod_node_data *)x;
 
  Pthread_mutex_lock(&(nd->node_data_lock));
  if (nd->discovered && conf.metric_server_debug)
    {
      int num;
      
      fprintf(stderr, "* %s: discovered=%d\n", nd->nodename, nd->discovered);
      fprintf(stderr, "* %s: last_received_time=%u\n", 
              nd->nodename, nd->last_received_time);

      if (nd->metric_data)
        {
          num = Hash_for_each(nd->metric_data, _metric_data_dump, nd->nodename);
          if (num != nd->metric_data_count)
            {
              fprintf(stderr, "* invalid dump count: num=%d numnodes=%d",
                      num, nd->metric_data_count);
            }
          else
            fprintf(stderr, "* %s: metric_data_count=%d\n", 
                    nd->nodename, nd->metric_data_count);
        }
    }
  Pthread_mutex_unlock(&(nd->node_data_lock));

  return 1;
}
#endif /* CEREBRO_DEBUG */

/*
 * _listener_data_list_dump
 *
 * Dump contents of node data list
 */
static void
_listener_data_list_dump(void)
{
#if CEREBRO_DEBUG
  if (!(conf.debug && conf.listen_debug))
    return;

  Pthread_mutex_lock(&listener_data_lock);
  Pthread_mutex_lock(&debug_output_mutex);
  fprintf(stderr, "**************************************\n");
  fprintf(stderr, "* Node Data List State\n");
  fprintf(stderr, "* -----------------------\n");
  fprintf(stderr, "* Nodes: %d\n", listener_data_index_numnodes);
  fprintf(stderr, "* -----------------------\n");
  if (listener_data_index_numnodes > 0)
    {
      int num;
      
      num = List_for_each(listener_data_list, _node_data_dump, NULL);
      if (num != listener_data_index_numnodes)
        {
          fprintf(stderr, "* invalid dump count: num=%d numnodes=%d",
                  num, listener_data_index_numnodes);
        }
    }
  else
    fprintf(stderr, "_listener_data_list_dump: empty list\n");
  fprintf(stderr, "**************************************\n");
  Pthread_mutex_unlock(&debug_output_mutex);
  Pthread_mutex_unlock(&listener_data_lock);
#endif /* CEREBRO_DEBUG */
}

/*
 * _metric_name_output
 *
 * callback function from hash_for_each to dump metric names
 */
#if CEREBRO_DEBUG
static int
_metric_name_output(void *data, const void *key, void *arg)
{
  struct cerebrod_metric_name_data *mnd;

  assert(data);

  mnd = (struct cerebrod_metric_name_data *)data;

  fprintf(stderr, "* %s\n", mnd->metric_name);
  return 1;
}
#endif /* CEREBRO_DEBUG */

/*
 * _metric_names_index_dump
 *
 * Dump contents of the metric name list
 */
static void
_metric_names_index_dump(void)
{
#if CEREBRO_DEBUG
  if (!(conf.debug && conf.listen_debug && metric_names_index))
    return;

  Pthread_mutex_lock(&metric_names_lock);
  Pthread_mutex_lock(&debug_output_mutex);
  fprintf(stderr, "**************************************\n");
  fprintf(stderr, "* Metric Name List\n");
  fprintf(stderr, "* -----------------------\n");
  Hash_for_each(metric_names_index, _metric_name_output, NULL);
  fprintf(stderr, "**************************************\n");
  Pthread_mutex_unlock(&debug_output_mutex);
  Pthread_mutex_unlock(&metric_names_lock);
#endif /* CEREBRO_DEBUG */
}

/* 
 * _metric_data_update
 *
 * Update the data of a particular metric.  The struct
 * cerebrod_node_data lock should already be locked.
 */
static void
_metric_data_update(struct cerebrod_node_data *nd,
                    const char *metric_name,
                    struct cerebrod_heartbeat_metric *hd,
                    u_int32_t received_time)
{
  struct cerebrod_listener_metric_data *md;
#if CEREBRO_DEBUG
  int rv;
#endif /* CEREBRO_DEBUG */
  
  assert(nd && metric_name && hd && nd->metric_data);

  /* If metric server isn't running, metric_names_index is NULL */
  if (!metric_names_index)
    return;

#if CEREBRO_DEBUG
  /* Should be called with lock already set */
  rv = Pthread_mutex_trylock(&nd->node_data_lock);
  if (rv != EBUSY)
    CEREBRO_EXIT(("mutex not locked: rv=%d", rv));
#endif /* CEREBRO_DEBUG */
  
  Pthread_mutex_lock(&metric_names_lock);
  if (!Hash_find(metric_names_index, metric_name))
    {
      struct cerebrod_metric_name_data *mnd;
      
      if ((Hash_count(metric_names_index) - metric_name_defaults_count) >= conf.metric_max)
        {
          CEREBRO_DBG(("too many metrics"));
          Pthread_mutex_unlock(&metric_names_lock);
          return;
        }
      
      mnd = Malloc(sizeof(struct cerebrod_metric_name_data));
      memset(mnd, '\0', sizeof(struct cerebrod_metric_name_data));
      mnd->metric_name = Strdup(metric_name);
      mnd->metric_origin = CEREBROD_METRIC_LISTENER_ORIGIN_MONITORED;
      Hash_insert(metric_names_index, mnd->metric_name, mnd);
    }
  Pthread_mutex_unlock(&metric_names_lock);             

  if (!(md = Hash_find(nd->metric_data, metric_name)))
    {
      /* Should be impossible to hit due to checks in
       * cerebrod_listener_data_update()
       */
      if (nd->metric_data_count >= conf.metric_max)
        {
          CEREBRO_DBG(("too many metrics: nodename=%s", nd->nodename));
          return;
        }
      
      md = (struct cerebrod_listener_metric_data *)Malloc(sizeof(struct cerebrod_listener_metric_data));
      memset(md, '\0', sizeof(struct cerebrod_listener_metric_data));
      md->metric_name = Strdup(metric_name);
      Hash_insert(nd->metric_data, md->metric_name, md);
      nd->metric_data_count++;
    }
  else
    {
      if (md->metric_value_type != hd->metric_value_type)
        CEREBRO_DBG(("metric type modified: old=%d new=%d",
                     md->metric_value_type, hd->metric_value_type));
    }
  
  md->metric_value_received_time = received_time;
  md->metric_value_type = hd->metric_value_type;
  /* Realloc size */
  if (md->metric_value_len != hd->metric_value_len)
    {
      if (md->metric_value)
        {
          CEREBRO_DBG(("metric length modified: old=%d new=%d",
                       md->metric_value_len, hd->metric_value_len));
          Free(md->metric_value);
        }
      md->metric_value = Malloc(hd->metric_value_len);
    }
  md->metric_value_len = hd->metric_value_len;
  memcpy(md->metric_value, hd->metric_value, hd->metric_value_len);
}

/* 
 * _monitor_update
 *
 * Send metric data to the appropriate monitor, if necessary.  The
 * struct cerebrod_node_data lock should already be locked.
 */
static void
_monitor_update(const char *nodename,
                struct cerebrod_node_data *nd,
                const char *metric_name,
                struct cerebrod_heartbeat_metric *hd)
{
  struct cerebrod_monitor_module *monitor_module;
#if CEREBRO_DEBUG
  int rv;
#endif /* CEREBRO_DEBUG */
  
  assert(nodename && nd && metric_name && hd);

  if (!monitor_index)
    return;

#if CEREBRO_DEBUG
  /* Should be called with lock already set */
  rv = Pthread_mutex_trylock(&nd->node_data_lock);
  if (rv != EBUSY)
    CEREBRO_EXIT(("mutex not locked: rv=%d", rv));
#endif /* CEREBRO_DEBUG */
  
  if ((monitor_module = Hash_find(monitor_index, metric_name)))
    {
      Pthread_mutex_lock(&monitor_module->monitor_lock);
      monitor_module_metric_update(monitor_handle,
                                   monitor_module->index,
                                   nodename,
                                   hd->metric_value_type,
                                   hd->metric_value_len,
                                   hd->metric_value);
      Pthread_mutex_unlock(&monitor_module->monitor_lock);
    }
}

void 
cerebrod_listener_data_update(char *nodename,
                              struct cerebrod_heartbeat *hb,
                              u_int32_t received_time)
{
  struct cerebrod_node_data *nd;
  int i, update_output_flag = 0;

  assert(nodename && hb);

  if (!listener_data_init)
    CEREBRO_EXIT(("initialization not complete"));

  if (!listener_data_list || !listener_data_index)
    CEREBRO_EXIT(("initialization not complete"));

  Pthread_mutex_lock(&listener_data_lock);
  if (!(nd = Hash_find(listener_data_index, nodename)))
    {
      /* Re-hash if our hash is getting too small */
      if ((listener_data_index_numnodes + 1) > LISTENER_DATA_REHASH_LIMIT)
	cerebrod_rehash(&listener_data_index,
			&listener_data_index_size,
			LISTENER_DATA_INDEX_SIZE_INCREMENT,
			listener_data_index_numnodes,
			&listener_data_lock);

      nd = _cerebrod_node_data_create_and_init(nodename);
      List_append(listener_data_list, nd);
      Hash_insert(listener_data_index, nd->nodename, nd);
      listener_data_index_numnodes++;

      /* Ok to call debug output function, since
       * listener_data_lock is locked.
       */
      _output_node_data_insert(nd);
    }
  Pthread_mutex_unlock(&listener_data_lock);
  
  Pthread_mutex_lock(&(nd->node_data_lock));
  if (received_time >= nd->last_received_time)
    {
      nd->discovered = 1;
      nd->last_received_time = received_time;

      for (i = 0; i < hb->metrics_len; i++)
        {
          char metric_name_buf[CEREBRO_MAX_METRIC_NAME_LEN+1];
          struct cerebrod_heartbeat_metric *hd = hb->metrics[i];

          /* Guarantee ending '\0' character */
          memset(metric_name_buf, '\0', CEREBRO_MAX_METRIC_NAME_LEN+1);
          memcpy(metric_name_buf, hd->metric_name, CEREBRO_MAX_METRIC_NAME_LEN);

          if (!strlen(metric_name_buf))
            {
              CEREBRO_DBG(("null heartbeat_data metric_name received"));
              continue;
            }

          _metric_data_update(nd, metric_name_buf, hd, received_time);
          _monitor_update(nodename, nd, metric_name_buf, hd);
        }

      /* Can't call a debug output function in here, it can cause a
       * deadlock b/c the listener_data_lock is not locked.  Use
       * a flag instead.
       */
      update_output_flag++;
    }
  Pthread_mutex_unlock(&(nd->node_data_lock));

  if (update_output_flag)
    _output_node_data_update(nd);

  _listener_data_list_dump();
  _metric_names_index_dump();
}
