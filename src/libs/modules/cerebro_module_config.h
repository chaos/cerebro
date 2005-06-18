/*****************************************************************************\
 *  $Id: cerebro_module_config.h,v 1.1 2005-06-18 06:47:06 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_MODULE_CONFIG_H
#define _CEREBRO_MODULE_CONFIG_H

#include "cerebro/cerebro_config.h"

typedef struct cerebro_config_module *cerebro_config_module_t; 
/*
 * _cerebro_module_load_config_module
 *
 * Find and load the config module.  If none is found, cerebro
 * library will assume a default config module.
 * 
 * Returns config module handle on success, NULL on error
 */
cerebro_config_module_t _cerebro_module_load_config_module(void);

/*
 * _cerebro_module_destroy_config_handle
 *
 * Destroy/Unload the config module specified by the handle
 *
 * Returns 0 on success, -1 on error
 */
int _cerebro_module_destroy_config_handle(cerebro_config_module_t config_handle);

/*
 * _cerebro_config_module_name
 *
 * Return config module name
 */
char *_cerebro_config_module_name(cerebro_config_module_t config_handle);

/*
 * _cerebro_config_module_setup
 *
 * call config module setup function
 */
int _cerebro_config_module_setup(cerebro_config_module_t config_handle);

/*
 * _cerebro_config_module_cleanup
 *
 * call config module cleanup function
 */
int _cerebro_config_module_cleanup(cerebro_config_module_t config_handle);

/*
 * _cerebro_config_module_load_default
 *
 * call config module get all nodes function
 */
int _cerebro_config_module_load_default(cerebro_config_module_t config_handle,
                                        struct cerebro_config *conf);


#endif /* _CEREBRO_MODULE_CONFIG_H */
