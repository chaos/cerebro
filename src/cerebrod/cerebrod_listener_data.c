/*****************************************************************************\
 *  $Id: cerebrod_listener_data.c,v 1.60 2008-03-28 17:06:47 chu11 Exp $
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
#include "cerebrod_listener_data.h"
#include "cerebrod_event_update.h"
#include "cerebrod_monitor_update.h"
#include "cerebrod_util.h"

#include "clusterlist_module.h"
#include "debug.h"
#include "event_module.h"
#include "hash.h"
#include "list.h"
#include "wrappers.h"

#define LISTENER_DATA_SIZE_DEFAULT              1024
#define LISTENER_DATA_SIZE_INCREMENT            1024

#define LISTENER_DATA_METRIC_NAMES_SIZE_DEFAULT 32
#define LISTENER_DATA_METRIC_NAMES_INCREMENT    32

#define LISTENER_DATA_METRIC_DATA_SIZE_DEFAULT  32
#define LISTENER_DATA_METRIC_DATA_INCREMENT     32

extern struct cerebrod_config conf;
#if CEREBRO_DEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* CEREBRO_DEBUG */

extern clusterlist_module_t clusterlist_handle;
extern int found_clusterlist_module;

/*
 * listener_data_init
 * listener_data_init_lock
 *
 * variables for synchronizing initialization between different pthreads
 */
int listener_data_init = 0;
pthread_mutex_t listener_data_init_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * listener_data
 * listener_data_numnodes
 * listener_data_size
 *
 * hash of listener data, number of entries, and index size
 */
hash_t listener_data = NULL;
int listener_data_numnodes;
int listener_data_size;

/*  
 * listener_data_lock
 *
 * lock to protect pthread access to listener_data
 */
pthread_mutex_t listener_data_lock = PTHREAD_MUTEX_INITIALIZER;

/* 
 * metric_names
 * metric_names_size
 *
 * contains list of currently known metrics
 */
hash_t metric_names = NULL;
int metric_names_size;
int metric_names_count;

/* 
 * metric_name_defaults
 * metric_name_defaults_count
 *
 * Default metrics
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

struct cerebrod_metric_data *
metric_data_create(const char *metric_name)
{
  struct cerebrod_metric_data *md;

  assert(metric_name);

  md = (struct cerebrod_metric_data *)Malloc(sizeof(struct cerebrod_metric_data));
  memset(md, '\0', sizeof(struct cerebrod_metric_data));
  md->metric_name = Strdup(metric_name);

  return md;
}

void 
metric_data_destroy(void *data)
{
  struct cerebrod_metric_data *md;

  assert(data);
  
  md = (struct cerebrod_metric_data *)data;
  Free(md->metric_name);
  Free(md->metric_value);
  Free(md);
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
    {
      nd->metric_data = Hash_create(LISTENER_DATA_METRIC_DATA_SIZE_DEFAULT, 
                                    (hash_key_f)hash_key_string,
                                    (hash_cmp_f)strcmp, 
                                    (hash_del_f)metric_data_destroy);
      nd->metric_data_size = LISTENER_DATA_METRIC_DATA_SIZE_DEFAULT;
    }
  else
    {
      nd->metric_data = NULL;
      nd->metric_data_size = 0;
    }
  nd->metric_data_count = 0;
  return nd;
}

struct cerebrod_metric_name_data *
metric_name_data_create(const char *metric_name, u_int32_t metric_origin)
{
  struct cerebrod_metric_name_data *mnd;

  assert(metric_name 
         && (metric_origin >= CEREBROD_METRIC_LISTENER_ORIGIN_DEFAULT
             && metric_origin <= CEREBROD_METRIC_LISTENER_ORIGIN_MONITORED));

  mnd = Malloc(sizeof(struct cerebrod_metric_name_data));
  memset(mnd, '\0', sizeof(struct cerebrod_metric_name_data));

  if (strlen(metric_name) > CEREBRO_MAX_METRIC_NAME_LEN)
    CEREBRO_DBG(("metric name length invalid"));

  if (metric_origin & CEREBROD_METRIC_LISTENER_ORIGIN_DEFAULT)
    mnd->metric_name = (char *)metric_name;
  else
    mnd->metric_name = Strdup(metric_name);

  mnd->metric_origin = metric_origin;

  return mnd;
}

void
metric_name_data_destroy(void *data)
{
  struct cerebrod_metric_name_data *mnd;

  assert(data);
  
  mnd = (struct cerebrod_metric_name_data *)data;

  if (mnd->metric_origin & CEREBROD_METRIC_LISTENER_ORIGIN_MONITORED)
    Free(mnd->metric_name);
  Free(mnd);
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

  if (found_clusterlist_module)
    {
      if ((numnodes = clusterlist_module_numnodes(clusterlist_handle)) < 0)
        CEREBRO_EXIT(("clusterlist_module_numnodes"));
    }

  if (numnodes > 0)
    {
      listener_data_numnodes = numnodes;
      listener_data_size = numnodes;
    }
  else
    {
      listener_data_numnodes = 0;
      listener_data_size = LISTENER_DATA_SIZE_DEFAULT;
    }

  listener_data = Hash_create(listener_data_size,
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

          Hash_insert(listener_data, nd->nodename, nd);

          free(nodes[i]);
        }

      free(nodes);
    }

  if (conf.metric_server)
    {
      int i;
      
      metric_names_size = metric_name_defaults_count + LISTENER_DATA_METRIC_NAMES_SIZE_DEFAULT;
      metric_names = Hash_create(metric_names_size, 
                                 (hash_key_f)hash_key_string, 
                                 (hash_cmp_f)strcmp, 
                                 (hash_del_f)metric_name_data_destroy);

      metric_names_count = 0;
      for (i = 0; i < metric_name_defaults_count; i++)
        {
          struct cerebrod_metric_name_data *mnd;

          mnd = metric_name_data_create(metric_name_defaults[i],
                                        CEREBROD_METRIC_LISTENER_ORIGIN_DEFAULT);

          Hash_insert(metric_names, mnd->metric_name, mnd);
          metric_names_count++;
        }
    }

  if (cerebrod_event_modules_setup() < 0)
    CEREBRO_EXIT(("cerebrod_event_modules_setup"));

  if (cerebrod_monitor_modules_setup() < 0)
    CEREBRO_EXIT(("cerebrod_monitor_modules_setup"));

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
  struct cerebrod_metric_data *md;
  char *nodename, *buf;
 
  assert(data && key && arg);
 
  md = (struct cerebrod_metric_data *)data;
  nodename = (char *)arg;
 
  fprintf(stderr, "* %s: metric name=%s type=%d len=%d ",
          nodename, md->metric_name, md->metric_value_type, 
          md->metric_value_len);

  if (md->metric_value_type == CEREBRO_DATA_VALUE_TYPE_INT32)
    fprintf(stderr, "metric_value=%d", *((int32_t *)md->metric_value));
  else if (md->metric_value_type == CEREBRO_DATA_VALUE_TYPE_U_INT32)
    fprintf(stderr, "metric_value=%u", *((u_int32_t *)md->metric_value));
  else if (md->metric_value_type == CEREBRO_DATA_VALUE_TYPE_FLOAT)
    fprintf(stderr, "metric_value=%f", *((float *)md->metric_value));
  else if (md->metric_value_type == CEREBRO_DATA_VALUE_TYPE_DOUBLE)
    fprintf(stderr, "metric_value=%f", *((double *)md->metric_value));
  else if (md->metric_value_type == CEREBRO_DATA_VALUE_TYPE_STRING)
    {
      /* Watch for NUL termination */
      buf = Malloc(md->metric_value_len + 1);
      memset(buf, '\0', md->metric_value_len + 1);
      memcpy(buf, md->metric_value, md->metric_value_len);
      fprintf(stderr, "metric_value=%s", buf);
      Free(buf);
    }
#if SIZEOF_LONG == 4
  else if (md->metric_value_type == CEREBRO_DATA_VALUE_TYPE_INT64)
    fprintf(stderr, "metric_value=%lld", *((int64_t *)md->metric_value));
  else if (md->metric_value_type == CEREBRO_DATA_VALUE_TYPE_U_INT64)
    fprintf(stderr, "metric_value=%llu", *((u_int64_t *)md->metric_value));
  fprintf(stderr, "\n");
#else /* SIZEOF_LONG == 8 */
  else if (md->metric_value_type == CEREBRO_DATA_VALUE_TYPE_INT64)
    fprintf(stderr, "metric_value=%ld", *((int64_t *)md->metric_value));
  else if (md->metric_value_type == CEREBRO_DATA_VALUE_TYPE_U_INT64)
    fprintf(stderr, "metric_value=%lu", *((u_int64_t *)md->metric_value));
  fprintf(stderr, "\n");
#endif /* SIZEOF_LONG == 8 */

  return 1;
}

/*
 * _node_data_dump
 *
 * callback function from list_for_each to dump node data
 */
static int
_node_data_dump(void *x, const void *key, void *arg)
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
            fprintf(stderr, "* invalid dump count: num=%d data count=%d\n",
                    num, nd->metric_data_count);
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
 * _listener_data_dump
 *
 * Dump contents of node data list
 */
static void
_listener_data_dump(void)
{
#if CEREBRO_DEBUG
  if (!(conf.debug && conf.listen_debug))
    return;

  Pthread_mutex_lock(&listener_data_lock);
  Pthread_mutex_lock(&debug_output_mutex);
  fprintf(stderr, "**************************************\n");
  fprintf(stderr, "* Node Data List State\n");
  fprintf(stderr, "* -----------------------\n");
  fprintf(stderr, "* Nodes: %d\n", listener_data_numnodes);
  fprintf(stderr, "* -----------------------\n");
  if (listener_data_numnodes > 0)
    {
      int num;
      
      num = Hash_for_each(listener_data, _node_data_dump, NULL);
      if (num != listener_data_numnodes)
        fprintf(stderr, "* invalid dump count: num=%d numnodes=%d\n",
                num, listener_data_numnodes);
    }
  else
    fprintf(stderr, "_listener_data_dump: empty list\n");
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
 * _metric_names_dump
 *
 * Dump contents of the metric name list
 */
static void
_metric_names_dump(void)
{
#if CEREBRO_DEBUG
  if (!(conf.debug && conf.listen_debug && metric_names))
    return;

  Pthread_mutex_lock(&metric_names_lock);
  Pthread_mutex_lock(&debug_output_mutex);
  fprintf(stderr, "**************************************\n");
  fprintf(stderr, "* Metric Name List\n");
  fprintf(stderr, "* -----------------------\n");
  Hash_for_each(metric_names, _metric_name_output, NULL);
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
                    struct cerebrod_message_metric *mm,
                    u_int32_t received_time)
{
  struct cerebrod_metric_data *md;
#if CEREBRO_DEBUG
  int rv;
#endif /* CEREBRO_DEBUG */
  
  assert(nd && metric_name && mm);

  /* If metric server isn't running, metric_names is NULL */
  if (!metric_names)
    return;

  assert(nd->metric_data);

#if CEREBRO_DEBUG
  /* Should be called with lock already set */
  rv = Pthread_mutex_trylock(&nd->node_data_lock);
  if (rv != EBUSY)
    CEREBRO_EXIT(("mutex not locked: rv=%d", rv));
#endif /* CEREBRO_DEBUG */
  
  if (!(md = Hash_find(nd->metric_data, metric_name)))
    {
      /* Re-hash if our hash is getting too small */
      if ((nd->metric_data_count + 1) > (nd->metric_data_size*2))
	cerebrod_rehash(&nd->metric_data,
			&nd->metric_data_size,
			LISTENER_DATA_METRIC_DATA_INCREMENT,
			nd->metric_data_count,
			&nd->node_data_lock);

      md = metric_data_create(metric_name);
      Hash_insert(nd->metric_data, md->metric_name, md);
      nd->metric_data_count++;
    }
  else
    {
      if (md->metric_value_type != mm->metric_value_type)
        CEREBRO_DBG(("metric type modified: old=%d new=%d",
                     md->metric_value_type, mm->metric_value_type));
    }
  
  md->metric_value_received_time = received_time;
  md->metric_value_type = mm->metric_value_type;

  /* Realloc size */
  if (md->metric_value_len != mm->metric_value_len)
    {
      if (md->metric_value)
        {
          CEREBRO_DBG(("metric length modified: old=%d new=%d",
                       md->metric_value_len, mm->metric_value_len));
          Free(md->metric_value);
        }
      md->metric_value = Malloc(mm->metric_value_len);
    }
  md->metric_value_len = mm->metric_value_len;
  memcpy(md->metric_value, mm->metric_value, mm->metric_value_len);
}

void 
cerebrod_listener_data_update(char *nodename,
                              struct cerebrod_message *msg,
                              u_int32_t received_time)
{
  struct cerebrod_node_data *nd;
  int i, update_output_flag = 0;

  assert(nodename && msg);

  if (!listener_data_init)
    CEREBRO_EXIT(("initialization not complete"));

  if (!listener_data)
    CEREBRO_EXIT(("initialization not complete"));

  Pthread_mutex_lock(&listener_data_lock);
  if (!(nd = Hash_find(listener_data, nodename)))
    {
      /* Re-hash if our hash is getting too small */
      if ((listener_data_numnodes + 1) > (listener_data_size*2))
	cerebrod_rehash(&listener_data,
			&listener_data_size,
			LISTENER_DATA_SIZE_INCREMENT,
			listener_data_numnodes,
			&listener_data_lock);

      nd = _cerebrod_node_data_create_and_init(nodename);
      Hash_insert(listener_data, nd->nodename, nd);
      listener_data_numnodes++;

      cerebrod_event_add_node_timeout_data(nd, received_time);
      
      /* Ok to call debug output function, since
       * listener_data_lock is locked.
       */
      _output_node_data_insert(nd);
    }
  Pthread_mutex_unlock(&listener_data_lock);

  /* 
   * The below loop used to be in _metric_data_update().  It was removed
   * b/c a race was discovered in debug mode.
   *
   * If the same host sends 3 cerebro message packets simultaneously
   * (lets assume due to network congestion), in debug mode we can
   * get the following deadlock:
   *
   * thread 1: enters _metric_names_dump.  Locks metric_names_lock.  Waits
   * on debug_output_lock.
   *
   * thread 2: enters _listener_data_dump.  Locks debug_output_lock
   * waits on node_data_lock.
   *
   * thread 3: locks node_data_lock below in this function.  Call
   * _metric_update_data() and waits on metric_names_lock.
   */
  if (conf.metric_server)
    {
      Pthread_mutex_lock(&metric_names_lock);
      for (i = 0; i < msg->metrics_len; i++)
        {
          char metric_name_buf[CEREBRO_MAX_METRIC_NAME_LEN+1];
          struct cerebrod_message_metric *mm = msg->metrics[i];
          
          /* Guarantee ending '\0' character */
          memset(metric_name_buf, '\0', CEREBRO_MAX_METRIC_NAME_LEN+1);
          memcpy(metric_name_buf, mm->metric_name, CEREBRO_MAX_METRIC_NAME_LEN);
          
          if (!strlen(metric_name_buf))
            {
              CEREBRO_DBG(("null message_data metric_name received"));
              continue;
            }
          
          if (!Hash_find(metric_names, metric_name_buf))
            {
              struct cerebrod_metric_name_data *mnd;
              
              /* Re-hash if our hash is getting too small */
              if ((metric_names_count + 1) > (metric_names_size*2))
                cerebrod_rehash(&metric_names,
                                &metric_names_size,
                                LISTENER_DATA_METRIC_NAMES_INCREMENT,
                                metric_names_count,
                                &metric_names_lock);
              
              mnd = metric_name_data_create(metric_name_buf,
                                            CEREBROD_METRIC_LISTENER_ORIGIN_MONITORED);
              
              Hash_insert(metric_names, mnd->metric_name, mnd);
              metric_names_count++;
            }
        }
      Pthread_mutex_unlock(&metric_names_lock);
    }

  Pthread_mutex_lock(&(nd->node_data_lock));
  if (received_time >= nd->last_received_time)
    {
      nd->discovered = 1;
      nd->last_received_time = received_time;

      cerebrod_event_update_node_received_time(nd, received_time);

      for (i = 0; i < msg->metrics_len; i++)
        {
          char metric_name_buf[CEREBRO_MAX_METRIC_NAME_LEN+1];
          struct cerebrod_message_metric *mm = msg->metrics[i];

          /* Guarantee ending '\0' character */
          memset(metric_name_buf, '\0', CEREBRO_MAX_METRIC_NAME_LEN+1);
          memcpy(metric_name_buf, mm->metric_name, CEREBRO_MAX_METRIC_NAME_LEN);

          if (!strlen(metric_name_buf))
            {
              CEREBRO_DBG(("null message_data metric_name received"));
              continue;
            }

          _metric_data_update(nd, metric_name_buf, mm, received_time);
          cerebrod_monitor_modules_update(nodename, nd, metric_name_buf, mm);
          cerebrod_event_modules_update(nodename, nd, metric_name_buf, mm);
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

  _listener_data_dump();
  _metric_names_dump();
}

#endif /* !WITH_CEREBROD_SPEAKER_ONLY */

