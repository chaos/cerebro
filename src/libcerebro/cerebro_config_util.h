/*****************************************************************************\
 *  $Id: cerebro_config_util.h,v 1.1 2005-05-09 14:21:53 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_CONFIG_UTIL_H
#define _CEREBRO_CONFIG_UTIL_H

#include <cerebro/cerebro_config.h>
#include <cerebro/cerebro_constants.h>

/* 
 * _cerebro_config_load_config_module
 *
 * Find and load config module
 *
 * Returns data in structure and 0 on success, -1 on error
 */
int _cerebro_config_load_config_module(struct cerebro_config *conf);

/* 
 * _cerebro_config_load_config_file
 *
 * Read and load configuration file
 *
 * Returns data in structure and 0 on success, -1 on error
 */
int _cerebro_config_load_config_file(struct cerebro_config *conf);

/* 
 * _cerebro_config_merge_cerebro_config
 *
 * Merge contents of module_conf and config_file_conf into conf.  The
 * config file conf takes precedence.
 */
int _cerebro_config_merge_cerebro_config(struct cerebro_config *conf,
					 struct cerebro_config *module_conf,
					 struct cerebro_config *config_file_conf);

/* 
 * cerebro_config_load
 *
 * Wrapper that calls cerebro_config_load_config_module,
 * cerebro_config_load_config_file, and
 * cerebro_config_merge_cerebro_config.
 *
 * Returns data in structure and 0 on success, -1 on error
 */
int cerebro_config_load(struct cerebro_config *conf);

#endif /* _CEREBRO_CONFIG_UTIL_H */
