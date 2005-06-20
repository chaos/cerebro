/*****************************************************************************\
 *  $Id: cerebro_config_util.h,v 1.2 2005-06-20 16:53:24 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_CONFIG_UTIL_H
#define _CEREBRO_CONFIG_UTIL_H

#include "cerebro.h"

/* 
 * _cerebro_load_config
 *
 * Read and load config file data
 *
 * Returns 0 on success, -1 on error
 */
int _cerebro_load_config(cerebro_t handle);

/* 
 * _cerebro_unload_config
 *
 * Unload config file info
 *
 * Returns 0 on success, -1 on error
 */
int _cerebro_unload_config(cerebro_t handle);

#endif /* _CEREBRO_CONFIG_UTIL_H */
