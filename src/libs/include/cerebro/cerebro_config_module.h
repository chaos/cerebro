/*****************************************************************************\
 *  $Id: cerebro_config_module.h,v 1.8 2008-03-28 17:06:48 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2008 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2005-2007 The Regents of the University of California.
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
 *  with Cerebro. If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#ifndef _CEREBRO_CONFIG_MODULE_H
#define _CEREBRO_CONFIG_MODULE_H

#include <cerebro/cerebro_config.h>

#define CEREBRO_CONFIG_INTERFACE_VERSION 1

/* 
 * Cerebro_config_interface_version
 *
 * function prototype for config module function to return the
 * current config interface version.  Should always return
 * current value of macro CEREBRO_CONFIG_INTERFACE_VERSION.
 *
 * Returns version number on success, -1 one error
 */
typedef int (*Cerebro_config_interface_version)(void);

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
 * Cerebro_config_load_config
 *
 * function prototype for config module function to alter default
 * cerebro configuration values.  Required to be defined by each
 * config module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_config_load_config)(struct cerebro_config *conf);

/*
 * struct cerebro_config_module_info 
 * 
 * contains config module information and operations.  Required to be
 * defined in each config module.
 */
struct cerebro_config_module_info
{
  char *config_module_name;
  Cerebro_config_interface_version interface_version;
  Cerebro_config_setup setup;
  Cerebro_config_cleanup cleanup;
  Cerebro_config_load_config load_config;
};

#endif /* _CEREBRO_CONFIG_MODULE_H */
