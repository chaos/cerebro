/*****************************************************************************\
 *  $Id: cerebro_api.h,v 1.9 2005-05-04 17:24:05 achu Exp $
\*****************************************************************************/
 
#ifndef _CEREBRO_API_H
#define _CEREBRO_API_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>

#include "cerebro_constants.h"
#include "cerebro_clusterlist_module.h"
#include "cerebro_config.h"
#include "cerebro_config_module.h"

#if !WITH_STATIC_MODULES
#include "ltdl.h"
#endif /* !WITH_STATIC_MODULES */

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
