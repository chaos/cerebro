/*****************************************************************************\
 *  $Id: cerebro_api.h,v 1.2 2005-04-28 23:36:23 achu Exp $
\*****************************************************************************/
 
#ifndef _CEREBRO_API_H
#define _CEREBRO_API_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>

#include "cerebro_defs.h"
#include "cerebro_clusterlist_module.h"
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
 * CEREBRO_CONFIG_LOADED      - Indicates configuration file has been loaded
 * CEREBRO_MODULES_LOADED     - Indicates modules have been loaded
 * CEREBRO_UPDOWN_DATA_LOADED - Indicates updown data has been loaded
 */
#define CEREBRO_CONFIG_LOADED        0x00000001
#define CEREBRO_MODULES_LOADED       0x00000002
#define CEREBRO_UPDOWN_DATA_LOADED   0x00000004

/* 
 * struct cerebro_config
 *
 * Store all possible cerebro configuration information
 */
struct cerebro_config
{
  char *clusterlist_module;
  int clusterlist_module_flag;
  char **clusterlist_module_options;
  int clusterlist_module_options_flag;
  char *config_module;
  int config_module_flag;

  char **updown_hostnames;
  int updown_hostnames_flag;
  unsigned int updown_port;
  int updown_port_flag;
  unsigned int updown_timeout_len;
  int updown_timeout_len_flag;
  int updown_flags;
  int updown_flags_flag;
};

/* 
 * struct cerebro
 *
 * Used globally as cerebro handle
 */
struct cerebro {
  int32_t magic;
  int32_t errnum;
  int32_t loaded_state;
  struct cerebro_config config;
#if !WITH_STATIC_MODULES
  lt_dlhandle clusterlist_dl_handle;
#endif /* !WITH_STATIC_MODULES */
  struct cerebro_clusterlist_module_info *clusterlist_module_info;
#if !WITH_STATIC_MODULES
  lt_dlhandle config_dl_handle;
#endif /* !WITH_STATIC_MODULES */
  struct cerebro_config_module_info *config_module_info;
  void *updown_data;
};

/* 
 * cerebro_load_config
 *
 * Read and load information from the cerebro configuration file
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_load_config(cerebro_t handle);

/* 
 * cerebro_load_config
 *
 * Unload configuration info
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_unload_config(cerebro_t handle);

/* 
 * cerebro_load_modules
 *
 * Find and load cerebro modules
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_load_modules(cerebro_t handle);

/* 
 * cerebro_unload_modules
 *
 * Unload cerebro modules
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_unload_modules(cerebro_t handle);

#endif /* _CEREBRO_API_H */
