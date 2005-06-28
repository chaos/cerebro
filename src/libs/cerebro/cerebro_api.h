/*****************************************************************************\
 *  $Id: cerebro_api.h,v 1.7 2005-06-28 21:26:52 achu Exp $
\*****************************************************************************/
 
#ifndef _CEREBRO_API_H
#define _CEREBRO_API_H

#include <sys/types.h>

#include "cerebro/cerebro_config.h"
#include "cerebro/cerebro_constants.h"

#include "clusterlist_module.h"
#include "list.h"


/* 
 * CEREBRO_MAGIC_NUMBER
 *
 * Magic number for cerebro handle
 */
#define CEREBRO_MAGIC_NUMBER                     0xF00F1234
#define CEREBRO_METRICLIST_MAGIC_NUMBER          0xF00F2345
#define CEREBRO_METRICLIST_ITERATOR_MAGIC_NUMBER 0xF00F3456
#define CEREBRO_NODELIST_MAGIC_NUMBER            0xF00F4567
#define CEREBRO_NODELIST_ITERATOR_MAGIC_NUMBER   0xF00F5678

/* 
 * Cerebro loaded state flags
 */
#define CEREBRO_CONFIG_LOADED                  0x00000001
#define CEREBRO_CLUSTERLIST_MODULE_LOADED      0x00000002

/* 
 * struct cerebro
 *
 * Used globally as cerebro handle
 */
struct cerebro {
  int32_t magic;
  int32_t errnum;
  char hostname[CEREBRO_MAX_HOSTNAME_LEN+1];
  unsigned int port;
  unsigned int timeout_len;
  unsigned int flags;
  int32_t loaded_state;
  struct cerebro_config config_data;
  clusterlist_module_t clusterlist_handle;
  List metriclists;
  List nodelists;
};

/* 
 * struct cerebro_metriclist
 *
 * Used for nodelist interface
 */
struct cerebro_metriclist {
  int32_t magic;
  int32_t errnum;
  List metric_names;
  List iterators;
  struct cerebro *handle;
};

/* 
 * struct cerebro_metriclist_iterator
 *
 * Used for nodelist iterator interface
 */
struct cerebro_metriclist_iterator {
  int32_t magic;
  int32_t errnum;
  ListIterator itr;
  char *current;
  struct cerebro_metriclist *metriclist;
};

/* 
 * struct cerebro_nodelist_data
 *
 * Stores a nodename and a value
 */
struct cerebro_nodelist_data {
  char nodename[CEREBRO_MAX_NODENAME_LEN+1];
  u_int32_t metric_value_type;
  u_int32_t metric_value_len;
  void *metric_value;
};

/* 
 * struct cerebro_nodelist
 *
 * Used for nodelist interface
 */
struct cerebro_nodelist {
  int32_t magic;
  int32_t errnum;
  char metric_name[CEREBRO_MAX_METRIC_NAME_LEN+1];
  List nodes;
  List iterators;
  struct cerebro *handle;
};

/* 
 * struct cerebro_nodelist_iterator
 *
 * Used for nodelist iterator interface
 */
struct cerebro_nodelist_iterator {
  int32_t magic;
  int32_t errnum;
  ListIterator itr;
  struct cerebro_nodelist_data *current;
  struct cerebro_nodelist *nodelist;
};

#endif /* _CEREBRO_API_H */
