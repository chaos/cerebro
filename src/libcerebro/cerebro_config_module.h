/*****************************************************************************\
 *  $Id: cerebro_config_module.h,v 1.1 2005-03-23 17:37:33 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_CONFIG_MODULE_H
#define _CEREBRO_CONFIG_MODULE_H

/*
 * Cerebro_config_load_default
 *
 * function prototype for config module function to alter default
 * configuration values
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
  Cerebro_config_load_default load_default;
};

#endif /* _CEREBRO_CONFIG_MODULE_H */
