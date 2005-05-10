/*****************************************************************************\
 *  $Id: cerebro_config_module.h,v 1.2 2005-05-10 17:51:55 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_CONFIG_MODULE_H
#define _CEREBRO_CONFIG_MODULE_H

#include <cerebro/cerebro_config.h>

/*
 * Cerebro_config_setup
 *
 * function prototype for config module function to setup the
 * module.  Required to be defined by each config module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_config_setup)(void);

/*
 * Cerebro_config_cleanup
 *
 * function prototype for config module function to cleanup.  Required
 * to be defined by each config module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_config_cleanup)(void);

/*
 * Cerebro_config_load_default
 *
 * function prototype for config module function to alter default
 * cerebro configuration values.  Required to be defined by each
 * config module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_config_load_default)(struct cerebro_config *conf);

/*
 * struct cerebro_config_module_info 
 * 
 * contains config module information and operations.  Required to be
 * defined in each config module.
 */
struct cerebro_config_module_info
{
  char *config_module_name;
  Cerebro_config_setup setup;
  Cerebro_config_cleanup cleanup;
  Cerebro_config_load_default load_default;
};

#endif /* _CEREBRO_CONFIG_MODULE_H */
