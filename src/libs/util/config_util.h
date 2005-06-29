/*****************************************************************************\
 *  $Id: config_util.h,v 1.2 2005-06-29 22:43:47 achu Exp $
\*****************************************************************************/

#ifndef _CONFIG_UTIL_H
#define _CONFIG_UTIL_H

#include "cerebro/cerebro_config.h"

/* 
 * load_config
 *
 * Wrapper that calls config_load_config_module,
 * config_load_config_file, and
 * config_merge_config.
 *
 * Returns data in structure and 0 on success, -1 on error
 */
int load_config(struct cerebro_config *conf, unsigned int *errnum);

#endif /* _CONFIG_UTIL_H */
