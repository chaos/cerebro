/*****************************************************************************\
 *  $Id: config_module.h,v 1.4 2005-07-22 17:21:07 achu Exp $
 *****************************************************************************
 *  Copyright (C) 2005 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <http://www.llnl.gov/linux/cerebro/>.
 *
 *  Cerebro is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  Cerebro is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with Genders; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
\*****************************************************************************/

#ifndef _CONFIG_MODULE_H
#define _CONFIG_MODULE_H

#include "cerebro/cerebro_config.h"

typedef struct config_module *config_module_t;

/*
 * config_module_load
 *
 * Find and load the config module.  If none is found, will assume a
 * default config module.
 * 
 * Returns config module handle on success, NULL on error
 */
config_module_t config_module_load(void);

/*
 * config_module_unload
 *
 * Unload/cleanup the config module specified by the handle
 *
 * Returns 0 on success, -1 on error
 */
int config_module_unload(config_module_t handle);

/*
 * config_module_name
 *
 * Return config module name
 */
char *config_module_name(config_module_t handle);

/*
 * config_module_setup
 *
 * call config module setup function
 */
int config_module_setup(config_module_t handle);

/*
 * config_module_cleanup
 *
 * call config module cleanup function
 */
int config_module_cleanup(config_module_t handle);

/*
 * config_module_load_config
 *
 * call config module get all nodes function
 */
int config_module_load_config(config_module_t handle, 
			      struct cerebro_config *conf);


#endif /* _CONFIG_MODULE_H */
