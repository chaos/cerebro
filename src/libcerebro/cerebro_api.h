/*****************************************************************************\
 *  $Id: cerebro_api.h,v 1.14 2005-05-23 21:30:29 achu Exp $
\*****************************************************************************/
 
#ifndef _CEREBRO_API_H
#define _CEREBRO_API_H

#include <sys/types.h>

#include "cerebro/cerebro_config.h"

#include "list.h"

/* 
 * CEREBRO_MAGIC_NUMBER
 *
 * Magic number for cerebro handle
 */
#define CEREBRO_MAGIC_NUMBER                   0xF00F1234
#define CEREBRO_NODELIST_MAGIC_NUMBER          0xF00F2345
#define CEREBRO_NODELIST_ITERATOR_MAGIC_NUMBER 0xF00F3456
#define CEREBRO_UPDOWN_MAGIC_NUMBER            0xF00F4567

/* 
 * Cerebro loaded state flags
 */
#define CEREBRO_MODULE_SETUP_CALLED        0x00000001              
#define CEREBRO_CONFIG_LOADED              0x00000002
#define CEREBRO_CLUSTERLIST_MODULE_LOADED  0x00000004
#define CEREBRO_UPDOWN_DATA_LOADED         0x00000008

/* 
 * struct cerebro
 *
 * Used globally as cerebro handle
 */
struct cerebro {
  int32_t magic;
  int32_t errnum;
  int32_t loaded_state;
  struct cerebro_config config_data;
  void *updown_data;
};

/* 
 * struct cerebro_nodelist
 *
 * Used for nodelist interface
 */
struct cerebro_nodelist {
  int32_t magic;
  int32_t errnum;
  List nodes;
  /* XXX list of iterators? */
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
};

#endif /* _CEREBRO_API_H */
