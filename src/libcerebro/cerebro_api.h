/*****************************************************************************\
 *  $Id: cerebro_api.h,v 1.5 2005-05-02 18:19:25 achu Exp $
\*****************************************************************************/
 
#ifndef _CEREBRO_API_H
#define _CEREBRO_API_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>

#include "cerebro_defs.h"
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
 *
 * CEREBRO_MODULE_SETUP_CALLED       - we called cerebro_module_setup
 * CEREBRO_CLUSTERLIST_MODULE_FOUND  - clusterlist module has been found
 * CEREBRO_UPDOWN_DATA_LOADED        - updown data has been loaded
 */
#define CEREBRO_MODULE_SETUP_CALLED        0x00000001              
#define CEREBRO_CONFIG_FILE_LOADED         0x00000002
#define CEREBRO_CLUSTERLIST_MODULE_FOUND   0x00000004
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
  struct cerebro_config config_file_data;
  void *updown_data;
};

/* 
 * cerebro_api_load_config
 *
 * Read and load config file data
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_api_load_config_file(cerebro_t handle);

/* 
 * cerebro_api_load_config
 *
 * Unload config file info
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_api_unload_config_file(cerebro_t handle);

/* 
 * cerebro_load_clusterlist_module
 *
 * Find and load clusterlist module
 *
 * Returns 1 if clusterlist module was loaded, 0 if not, -1 on fatal error
 */
int cerebro_api_load_clusterlist_module(cerebro_t handle);

/* 
 * cerebro_unload_clusterlist_module
 *
 * Unload clusterlist module
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_api_unload_clusterlist_module(cerebro_t handle);

#endif /* _CEREBRO_API_H */
