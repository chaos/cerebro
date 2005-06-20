/*****************************************************************************\
 *  $Id: config_util.h,v 1.1 2005-06-20 16:53:24 achu Exp $
\*****************************************************************************/

#ifndef _CONFIG_UTIL_H
#define _CONFIG_UTIL_H

#include "cerebro/cerebro_config.h"

/* 
 * load_config_module
 *
 * Find and load config module
 *
 * Returns data in structure and 0 on success, -1 on error
 */
int load_config_module(struct cerebro_config *conf);

/* 
 * load_config_file
 *
 * Read and load configuration file
 *
 * Returns data in structure and 0 on success, -1 on error
 */
int load_config_file(struct cerebro_config *conf);

/* 
 * merge_cerebro_configs
 *
 * Merge contents of module_conf and config_file_conf into conf.  The
 * config file conf takes precedence.
 */
int merge_cerebro_configs(struct cerebro_config *conf,
			  struct cerebro_config *module_conf,
			  struct cerebro_config *config_file_conf);

/* 
 * load_config
 *
 * Wrapper that calls config_load_config_module,
 * config_load_config_file, and
 * config_merge_config.
 *
 * Returns data in structure and 0 on success, -1 on error
 */
int load_config(struct cerebro_config *conf);

#endif /* _CONFIG_UTIL_H */
