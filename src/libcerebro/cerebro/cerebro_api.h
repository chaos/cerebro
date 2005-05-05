/*****************************************************************************\
 *  $Id: cerebro_api.h,v 1.1 2005-05-05 16:12:57 achu Exp $
\*****************************************************************************/
 
#ifndef _CEREBRO_API_H
#define _CEREBRO_API_H

#include <sys/types.h>

#include "cerebro/cerebro_config.h"

/* 
 * CEREBRO_MAGIC_NUMBER
 *
 * Magic number for cerebro handle
 */
#define CEREBRO_MAGIC_NUMBER 0xF00F1234

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

#endif /* _CEREBRO_API_H */
