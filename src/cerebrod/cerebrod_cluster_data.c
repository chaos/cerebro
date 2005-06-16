/*****************************************************************************\
 *  $Id: cerebrod_cluster_data.c,v 1.8 2005-06-16 21:16:08 achu Exp $
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
#include "cerebro_module.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_error.h"

#include "cerebrod.h"
#include "cerebrod_cluster_data.h"
#include "cerebrod_config.h"
#include "cerebrod_util.h"
#include "cerebrod_wrappers.h"
#include "list.h"
#include "hash.h"

#define CEREBROD_CLUSTER_DATA_INDEX_SIZE_DEFAULT   1024
#define CEREBROD_CLUSTER_DATA_INDEX_SIZE_INCREMENT 1024
#define CEREBROD_CLUSTER_DATA_REHASH_LIMIT         (cerebrod_cluster_data_index_size*2)

extern struct cerebrod_config conf;
#if CEREBRO_DEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* CEREBRO_DEBUG */

extern cerebro_clusterlist_module_t clusterlist_handle;

/*
 * cerebrod_cluster_data_initialization_complete
 * cerebrod_cluster_data_initialization_complete_lock
 *
 * variables for synchronizing initialization between different pthreads
 */
int cerebrod_cluster_data_initialization_complete = 0;
pthread_mutex_t cerebrod_cluster_data_initialization_complete_lock = PTHREAD_MUTEX_INITIALIZER;

/* 
 * cerebrod_cluster_data_list
 *
 * contains list of nodes with cluster_data
 */
List cerebrod_cluster_data_list = NULL;

/* 
 * cerebrod_metric_name_list
 *
 * contains list of metrics currently known
 */
List cerebrod_metric_name_list = NULL;

/*
 * cerebrod_cluster_data_index
 * cerebrod_cluster_data_index_numnodes
 * cerebrod_cluster_data_index_size
 *
 * hash index into cerebrod_cluster_data list for faster access, number of
 * currently indexed entries, and index size
 */
hash_t cerebrod_cluster_data_index = NULL;
int cerebrod_cluster_data_index_numnodes;
int cerebrod_cluster_data_index_size;

/*  
 * cerebrod_cluster_data_lock
 *
 * lock to protect pthread access to both the cluster_data list and
 * cluster_data_index
 *
 */
pthread_mutex_t cerebrod_cluster_data_lock = PTHREAD_MUTEX_INITIALIZER;

/*  
 * cerebrod_metric_name_lock
 *
 * lock to protect pthread access to the metric_name list
 *
 * Locking rule: Can be locked within a lock struct cerebrod_node_data
 * node_data_lock, but not a the above 'cerebrod_cluster_data_lock'.
 */
pthread_mutex_t cerebrod_metric_name_lock = PTHREAD_MUTEX_INITIALIZER;

#if !WITH_STATIC_MODULES
/* 
 * monitor_handle
 *
 * monitoring module handles
 */
cerebro_monitor_modules_t monitor_handle = NULL;

/*
 * monitor_list
 *
 * list of monitors
 */
List monitor_list = NULL;

/*
 * monitor_index
 * monitor_index_size
 *
 * hash index to quickly determine what metrics are being
 * monitored by modules and what index they are.
 */
hash_t monitor_index = NULL;
int monitor_index_size;

#endif /* !WITH_STATIC_MODULES */

/* 
 * _cerebrod_node_data_strcmp
 *
 * callback function for list_sort to sort node names
 */
static int 
_cerebrod_node_data_strcmp(void *x, void *y)
{
  assert(x);
  assert(y);

  return strcmp(((struct cerebrod_node_data *)x)->nodename,
                ((struct cerebrod_node_data *)y)->nodename);
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
                                  (hash_del_f)_Free);
  else
    nd->metric_data = NULL;
  nd->metric_data_count = 0;
  return nd;
}

void 
cerebrod_cluster_data_initialize(void)
{
  int numnodes = 0;
#if !WITH_STATIC_MODULES
  int i;
#endif /* !WITH_STATIC_MODULES */

  pthread_mutex_lock(&cerebrod_cluster_data_initialization_complete_lock);
  if (cerebrod_cluster_data_initialization_complete)
    goto out;

  if ((numnodes = _cerebro_clusterlist_module_numnodes(clusterlist_handle)) < 0)
    cerebro_err_exit("%s(%s:%d): _cerebro_clusterlist_module_numnodes",
		     __FILE__, __FUNCTION__, __LINE__);

  if (numnodes > 0)
    {
      cerebrod_cluster_data_index_numnodes = numnodes;
      cerebrod_cluster_data_index_size = numnodes;
    }
  else
    {
      cerebrod_cluster_data_index_numnodes = 0;
      cerebrod_cluster_data_index_size = CEREBROD_CLUSTER_DATA_INDEX_SIZE_DEFAULT;
    }

  cerebrod_cluster_data_list = List_create((ListDelF)_Free);
  cerebrod_cluster_data_index = Hash_create(cerebrod_cluster_data_index_size,
                                            (hash_key_f)hash_key_string,
                                            (hash_cmp_f)strcmp,
                                            (hash_del_f)NULL);
  
  /* If the clusterlist module contains nodes, retrieve all of these
   * nodes and put them into the cerebrod_cluster_data list.  All updates
   * will involve updating data rather than involve insertion.
   */
  if (numnodes > 0)
    {
      int i;
      char **nodes;

      if (_cerebro_clusterlist_module_get_all_nodes(clusterlist_handle,
                                                    &nodes) < 0)
        cerebro_err_exit("%s(%s:%d): _cerebro_clusterlist_module_get_all_nodes",
                         __FILE__, __FUNCTION__, __LINE__);

      for (i = 0; i < numnodes; i++)
        {
          struct cerebrod_node_data *nd;
          
          nd = _cerebrod_node_data_create_and_init(nodes[i]);

          List_append(cerebrod_cluster_data_list, nd);
          Hash_insert(cerebrod_cluster_data_index, nd->nodename, nd);

          free(nodes[i]);
        }

      list_sort(cerebrod_cluster_data_list, (ListCmpF)_cerebrod_node_data_strcmp);

      free(nodes);
    }

  if (conf.metric_server)
    {
      cerebrod_metric_name_list = List_create((ListDelF)_Free);

      if (conf.metric_max)
        {
          /* Initialize with the handle of default metrics */
          if (List_count(cerebrod_metric_name_list) < conf.metric_max)
            List_append(cerebrod_metric_name_list, Strdup(CEREBRO_METRIC_CLUSTER_NODES));
          if (List_count(cerebrod_metric_name_list) < conf.metric_max)
            List_append(cerebrod_metric_name_list, Strdup(CEREBRO_METRIC_UP_NODES));
          if (List_count(cerebrod_metric_name_list) < conf.metric_max)
            List_append(cerebrod_metric_name_list, Strdup(CEREBRO_METRIC_DOWN_NODES));
          if (List_count(cerebrod_metric_name_list) < conf.metric_max)
            List_append(cerebrod_metric_name_list, Strdup(CEREBRO_METRIC_UPDOWN_STATE));
          if (List_count(cerebrod_metric_name_list) < conf.metric_max)
            List_append(cerebrod_metric_name_list, Strdup(CEREBRO_METRIC_LAST_RECEIVED_TIME));
        }
    }

#if !WITH_STATIC_MODULES
  /* XXX is metric_max correct?? */

  if (!conf.listen)
    cerebro_err_exit("%s(%s:%d): listen server not setup",
                     __FILE__, __FUNCTION__, __LINE__);

  if (!(monitor_handle = _cerebro_module_load_monitor_modules(conf.metric_max)))
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_module_load_monitor_modules failed",
                        __FILE__, __FUNCTION__, __LINE__);
      goto done;
    }


  if ((monitor_index_size = _cerebro_monitor_module_count(monitor_handle)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_monitor_module_count failed",
                        __FILE__, __FUNCTION__, __LINE__);
      _cerebro_module_destroy_monitor_handle(monitor_handle);
      monitor_handle = NULL;
      goto done;
    }

  if (!monitor_index_size)
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
      _cerebro_module_destroy_monitor_handle(monitor_handle);
      monitor_handle = NULL;
      goto done;
    }
  
  monitor_list = List_create((ListDelF)_Free);
  monitor_index = Hash_create(monitor_index_size,
                              (hash_key_f)hash_key_string,
                              (hash_cmp_f)strcmp,
                              (hash_del_f)_Free);

  for (i = 0; i < monitor_index_size; i++)
    {
      struct cerebrod_monitor *monitor;
      char *module_name;
      char **metric_names;
      int metric_count;
      int j;

      if (!(module_name = _cerebro_monitor_module_name(monitor_handle, i)))
        {
          cerebro_err_debug("%s(%s:%d): _cerebro_monitor_module_name failed",
                            __FILE__, __FUNCTION__, __LINE__);
          goto monitor_cleanup;
        }

      if (_cerebro_monitor_module_setup(monitor_handle, i) < 0)
        {
          cerebro_err_debug("%s(%s:%d): _cerebro_monitor_module_setup failed",
                            __FILE__, __FUNCTION__, __LINE__);
          goto monitor_cleanup;
        }

      monitor = Malloc(sizeof(struct cerebrod_monitor));
      monitor->module_name = Strdup(module_name);
      Pthread_mutex_init(&(monitor->monitor_lock), NULL);

      List_append(monitor_list, monitor);

      if ((metric_count = _cerebro_monitor_module_metric_names(monitor_handle,
                                                               i,
                                                               &metric_names)) < 0)
        {
          cerebro_err_debug("%s(%s:%d): _cerebro_monitor_module_metric_names failed",
                            __FILE__, __FUNCTION__, __LINE__);
          goto monitor_cleanup;
        }
      
      for (j = 0; j < metric_count; j++)
        {
          struct cerebrod_monitor_metric *monitor_metric;
      
#if CEREBRO_DEBUG
          if (conf.debug && conf.listen_debug)
            {
              Pthread_mutex_lock(&debug_output_mutex);
              fprintf(stderr, "**************************************\n");
              fprintf(stderr, "* Settup up Monitor Module/Metric: %s/%s\n",
                      _cerebro_monitor_module_name(monitor_handle, i),
                      metric_names[i]);
              fprintf(stderr, "**************************************\n");
              Pthread_mutex_unlock(&debug_output_mutex);
            }
#endif /* CEREBRO_DEBUG */

          monitor_metric = Malloc(sizeof(struct cerebrod_monitor));
          monitor_metric->metric_name = Strdup(metric_names[j]);
          monitor_metric->index = i;
          monitor_metric->monitor = monitor;

          Hash_insert(monitor_index, 
                      monitor_metric->metric_name,
                      monitor_metric);
        }

      for (j = 0; j < metric_count; j++)
        free(metric_names[j]);
      free(metric_names);
    }

  goto done;

 monitor_cleanup:
  if (monitor_list)
    {
      List_destroy(monitor_list);
      monitor_list = NULL;
    }
  if (monitor_handle)
    {
      _cerebro_module_destroy_monitor_handle(monitor_handle);
      monitor_handle = NULL;
    }
  if (monitor_index)
    {
      Hash_destroy(monitor_index);
      monitor_index = NULL;
    }
  monitor_index_size = 0;
  goto done;

 done:
#endif /* !WITH_STATIC_MODULES */

  cerebrod_cluster_data_initialization_complete++;
 out:
  Pthread_mutex_unlock(&cerebrod_cluster_data_initialization_complete_lock);
}

/*  
 * _cerebrod_cluster_data_output_insert
 *
 * Output debugging info about a recently inserted node
 */
static void
_cerebrod_cluster_data_output_insert(struct cerebrod_node_data *nd)
{
#if CEREBRO_DEBUG
  assert(nd);
 
  if (conf.debug && conf.listen_debug)
    {
      Pthread_mutex_lock(&debug_output_mutex);
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Cluster_Data Insertion: Node=%s\n", nd->nodename);
      fprintf(stderr, "**************************************\n");
      Pthread_mutex_unlock(&debug_output_mutex);
    }
#endif /* CEREBRO_DEBUG */
}

/*  
 * _cerebrod_cluster_data_output_update
 *
 * Output debugging info about a recently updated node
 */
static void
_cerebrod_cluster_data_output_update(struct cerebrod_node_data *nd)
{
#if CEREBRO_DEBUG
  assert(nd);
 
  if (conf.debug && conf.listen_debug)
    {
      struct tm tm;
      char strbuf[CEREBROD_STRING_BUFLEN];
 
      Localtime_r((time_t *)&(nd->last_received_time), &tm);
      strftime(strbuf, CEREBROD_STRING_BUFLEN, "%H:%M:%S", &tm);
 
      Pthread_mutex_lock(&debug_output_mutex);
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* cluster_data Update: Node=%s Last_Received_Time=%s\n", 
	      nd->nodename, strbuf);
      fprintf(stderr, "**************************************\n");
      Pthread_mutex_unlock(&debug_output_mutex);
    }
#endif /* CEREBRO_DEBUG */
}

/*
 * _cerebrod_node_data_metric_data_dump
 *
 * callback function from hash_for_each to dump metric node data
 */
static int
_cerebrod_node_data_metric_data_dump(void *data, const void *key, void *arg)
{
  struct cerebrod_metric_data *md;
  char *buf;
  char *nodename;
 
  assert(data);
  assert(key);
  assert(arg);
 
  md = (struct cerebrod_metric_data *)data;
  nodename = (char *)arg;
 
  fprintf(stderr, "* %s: metric_name=%s metric_value_type=%d metric_value_len=%d ",
          nodename,
          md->metric_name,
          md->metric_value_type,
          md->metric_value_len);

  switch (md->metric_value_type)
    {
    case CEREBRO_METRIC_VALUE_TYPE_INT32:
      fprintf(stderr, "metric_value=%d", *((int32_t *)md->metric_value));
      break;
    case CEREBRO_METRIC_VALUE_TYPE_U_INT32:
      fprintf(stderr, "metric_value=%u", *((u_int32_t *)md->metric_value));
      break;
    case CEREBRO_METRIC_VALUE_TYPE_FLOAT:
      fprintf(stderr, "metric_value=%f", *((float *)md->metric_value));
      break;
    case CEREBRO_METRIC_VALUE_TYPE_DOUBLE:
      fprintf(stderr, "metric_value=%f", *((double *)md->metric_value));
      break;
    case CEREBRO_METRIC_VALUE_TYPE_STRING:
      /* Watch for NUL termination */
      buf = Malloc(md->metric_value_len + 1);
      memset(buf, '\0', md->metric_value_len + 1);
      memcpy(buf, md->metric_value, md->metric_value_len);
      fprintf(stderr, "metric_value=%s", buf);
      Free(buf);
      break;
    case CEREBRO_METRIC_VALUE_TYPE_RAW:
      /* Don't output raw data */
      break;
    default:
      cerebro_err_debug("%s(%s:%d): nodename=%s invalid metric_value_type=%d",
                        __FILE__, __FUNCTION__, __LINE__,
                        (char *)key,
                        md->metric_value_type);
    }
  fprintf(stderr, "\n");

  return 1;
}

/*
 * _cerebrod_node_data_item_dump
 *
 * callback function from list_for_each to dump node data
 */
#if CEREBRO_DEBUG
static int
_cerebrod_node_data_item_dump(void *x, void *arg)
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
          num = Hash_for_each(nd->metric_data,
                              _cerebrod_node_data_metric_data_dump,
                              nd->nodename);
          if (num != nd->metric_data_count)
            {
              fprintf(stderr, "%s(%s:%d): invalid dump count: num=%d numnodes=%d",
                      __FILE__, __FUNCTION__, __LINE__, num, nd->metric_data_count);
              exit(1);
            }
          fprintf(stderr, "* %s: metric_data_count=%d\n", 
                  nd->nodename, nd->metric_data_count);
        }
    }
  Pthread_mutex_unlock(&(nd->node_data_lock));

  return 1;
}
#endif /* CEREBRO_DEBUG */

/*
 * _cerebrod_cluster_data_list_dump
 *
 * Dump contents of node data list
 */
static void
_cerebrod_cluster_data_list_dump(void)
{
#if CEREBRO_DEBUG
  if (conf.debug && conf.listen_debug)
    {
      Pthread_mutex_lock(&cerebrod_cluster_data_lock);
      Pthread_mutex_lock(&debug_output_mutex);
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Node Data List State\n");
      fprintf(stderr, "* -----------------------\n");
      fprintf(stderr, "* Listed Nodes: %d\n", cerebrod_cluster_data_index_numnodes);
      fprintf(stderr, "* -----------------------\n");
      if (cerebrod_cluster_data_index_numnodes > 0)
        {
          int num;

          num = List_for_each(cerebrod_cluster_data_list,
			      _cerebrod_node_data_item_dump,
			      NULL);
          if (num != cerebrod_cluster_data_index_numnodes)
	    {
              fprintf(stderr, "%s(%s:%d): invalid dump count: num=%d numnodes=%d",
                      __FILE__, __FUNCTION__, __LINE__, 
                      num, cerebrod_cluster_data_index_numnodes);
	      exit(1);
	    }
        }
      else
        fprintf(stderr, "_cerebrod_cluster_data_list_dump: "
                "called with empty list\n");
      fprintf(stderr, "**************************************\n");
      Pthread_mutex_unlock(&debug_output_mutex);
      Pthread_mutex_unlock(&cerebrod_cluster_data_lock);
    }
#endif /* CEREBRO_DEBUG */
}

/*
 * _cerebrod_metric_name_item_dump
 *
 * callback function from hash_for_each to dump node data
 */
#if CEREBRO_DEBUG
static int
_cerebrod_metric_name_item_dump(void *x, void *arg)
{
  assert(x);
  fprintf(stderr, "* %s\n", (char *)x);
  return 1;
}
#endif /* CEREBRO_DEBUG */

/*
 * _cerebrod_metric_name_list_dump
 *
 * Dump contents of node data list
 */
static void
_cerebrod_metric_name_list_dump(void)
{
#if CEREBRO_DEBUG
  if (conf.debug && conf.listen_debug)
    {
      if (cerebrod_metric_name_list)
        {
          Pthread_mutex_lock(&cerebrod_metric_name_lock);
          Pthread_mutex_lock(&debug_output_mutex);
          fprintf(stderr, "**************************************\n");
          fprintf(stderr, "* Metric Name List\n");
          fprintf(stderr, "* -----------------------\n");
          List_for_each(cerebrod_metric_name_list,
                        _cerebrod_metric_name_item_dump,
                        NULL);
          fprintf(stderr, "**************************************\n");
          Pthread_mutex_unlock(&debug_output_mutex);
          Pthread_mutex_unlock(&cerebrod_metric_name_lock);
        }
    }
#endif /* CEREBRO_DEBUG */
}

/* 
 * _cerebrod_metric_data_update
 *
 * Update the data of a particular metric.  The struct
 * cerebrod_node_data lock should already be locked.
 */
static void
_cerebrod_metric_data_update(struct cerebrod_node_data *nd,
                             char *metric_name,
                             struct cerebrod_heartbeat_metric *hd,
                             u_int32_t received_time)
{
  struct cerebrod_metric_data *md;
#if CEREBRO_DEBUG
  int rv;
#endif /* CEREBRO_DEBUG */

  assert(nd);
  assert(hd);
  assert(nd->metric_data);

#if CEREBRO_DEBUG
  /* Should be called with lock already set */
  rv = Pthread_mutex_trylock(&nd->node_data_lock);
  if (rv != EBUSY)
    cerebro_err_exit("%s(%s:%d): mutex not locked: rv=%d",
                     __FILE__, __FUNCTION__, __LINE__, rv);
#endif /* CEREBRO_DEBUG */
  
  if (!(md = Hash_find(nd->metric_data, metric_name)))
    {
      /* Should be impossible to hit due to checks in
       * cerebrod_cluster_data_update()
       */
      if (nd->metric_data_count >= conf.metric_max)
        {
          cerebro_err_debug("%s(%s:%d): too many metrics: nodename=%s",
                            __FILE__, __FUNCTION__, __LINE__,
                            nd->nodename);
          return;
        }
      
      md = (struct cerebrod_metric_data *)Malloc(sizeof(struct cerebrod_metric_data));
      memset(md, '\0', sizeof(struct cerebrod_metric_data));
      md->metric_name = Strdup(metric_name);
      Hash_insert(nd->metric_data, md->metric_name, md);
      nd->metric_data_count++;
    }
  else
    {
      if (md->metric_value_type != hd->metric_value_type)
        cerebro_err_debug("%s(%s:%d): metric type modified: old=%d new=%d",
                          __FILE__, __FUNCTION__, __LINE__,
                          md->metric_value_type, hd->metric_value_type);
    }
  
  md->last_received_time = received_time;
  md->metric_value_type = hd->metric_value_type;
  /* Realloc size */
  if (md->metric_value_len != hd->metric_value_len)
    {
      if (md->metric_value)
        {
          cerebro_err_debug("%s(%s:%d): metric length modified: old=%d new=%d",
                            __FILE__, __FUNCTION__, __LINE__,
                            md->metric_value_len, hd->metric_value_len);
          Free(md->metric_value);
        }
      md->metric_value = Malloc(hd->metric_value_len);
    }
  md->metric_value_len = hd->metric_value_len;
  memcpy(md->metric_value, hd->metric_value, hd->metric_value_len);
}

void 
cerebrod_cluster_data_update(char *nodename,
                             struct cerebrod_heartbeat *hb,
                             u_int32_t received_time)
{
  struct cerebrod_node_data *nd;
  int i, update_output_flag = 0;

  assert(nodename);
  assert(hb);

  /* It is possible no servers are turned on */
  if (!cerebrod_cluster_data_initialization_complete)
    return;

  if (!cerebrod_cluster_data_list || !cerebrod_cluster_data_index)
    cerebro_err_exit("%s(%s:%d): initialization not complete",
                     __FILE__, __FUNCTION__, __LINE__);

  Pthread_mutex_lock(&cerebrod_cluster_data_lock);
  if (!(nd = Hash_find(cerebrod_cluster_data_index, nodename)))
    {
      /* Re-hash if our hash is getting too small */
      if ((cerebrod_cluster_data_index_numnodes + 1) > CEREBROD_CLUSTER_DATA_REHASH_LIMIT)
	cerebrod_rehash(&cerebrod_cluster_data_index,
			&cerebrod_cluster_data_index_size,
			CEREBROD_CLUSTER_DATA_INDEX_SIZE_INCREMENT,
			cerebrod_cluster_data_index_numnodes,
			&cerebrod_cluster_data_lock);

      nd = _cerebrod_node_data_create_and_init(nodename);
      List_append(cerebrod_cluster_data_list, nd);
      Hash_insert(cerebrod_cluster_data_index, nd->nodename, nd);
      cerebrod_cluster_data_index_numnodes++;

      /* Ok to call debug output function, since
       * cerebrod_cluster_data_lock is locked.
       */
      _cerebrod_cluster_data_output_insert(nd);
    }
  Pthread_mutex_unlock(&cerebrod_cluster_data_lock);
  
  Pthread_mutex_lock(&(nd->node_data_lock));
  if (received_time >= nd->last_received_time)
    {
      nd->discovered = 1;
      nd->last_received_time = received_time;

#if WITH_STATIC_MODULES
      if (cerebrod_metric_name_list)
#else  /* !WITH_STATIC_MODULES */
      if (cerebrod_metric_name_list || monitor_index)
#endif /* !WITH_STATIC_MODULES */
        {
          
          for (i = 0; i < hb->metrics_len; i++)
            {
              char metric_name_buf[CEREBRO_METRIC_NAME_MAXLEN+1];
#if !WITH_STATIC_MODULES
              struct cerebrod_monitor_metric *monitor_metric;
#endif /* !WITH_STATIC_MODULES */
              
              /* Guarantee ending '\0' character */
              memset(metric_name_buf, '\0', CEREBRO_METRIC_NAME_MAXLEN+1);
              memcpy(metric_name_buf, 
                     hb->metrics[i]->metric_name, 
                     CEREBRO_METRIC_NAME_MAXLEN);
              
              if (cerebrod_metric_name_list)
                {
                  Pthread_mutex_lock(&cerebrod_metric_name_lock);
                  if (!List_find_first(cerebrod_metric_name_list,
                                       list_find_first_string,
                                       metric_name_buf))
                    {
                      if (List_count(cerebrod_metric_name_list) >= conf.metric_max)
                        {
                          Pthread_mutex_unlock(&cerebrod_metric_name_lock);
                          continue;
                        }
                      List_append(cerebrod_metric_name_list, 
                                  Strdup(metric_name_buf));
                    }
                  Pthread_mutex_unlock(&cerebrod_metric_name_lock);
                  
                  _cerebrod_metric_data_update(nd,
                                               metric_name_buf,
                                               hb->metrics[i],
                                               received_time);
                }

#if !WITH_STATIC_MODULES
              if (monitor_index)
                {
                  if ((monitor_metric = Hash_find(monitor_index, metric_name_buf)))
                    {
                      Pthread_mutex_lock(&monitor_metric->monitor->monitor_lock);
                      _cerebro_monitor_module_metric_update(monitor_handle,
                                                            monitor_metric->index,
                                                            nodename,
                                                            metric_name_buf,
                                                            hb->metrics[i]->metric_value_type,
                                                            hb->metrics[i]->metric_value_len,
                                                            hb->metrics[i]->metric_value);
                      Pthread_mutex_unlock(&monitor_metric->monitor->monitor_lock);
                    }
                }
#endif /* !WITH_STATIC_MODULES */
            }
        }

      /* Can't call a debug output function in here, it can cause a
       * deadlock b/c the cerebrod_cluster_data_lock is not locked.  Use
       * a flag instead.
       */
      update_output_flag++;
    }
  Pthread_mutex_unlock(&(nd->node_data_lock));


  if (update_output_flag)
    _cerebrod_cluster_data_output_update(nd);

  _cerebrod_cluster_data_list_dump();
  _cerebrod_metric_name_list_dump();
}

