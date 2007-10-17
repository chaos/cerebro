/*****************************************************************************\
 *  $Id: config_module_default.c,v 1.5 2007-10-17 22:04:50 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007 Lawrence Livermore National Security, LLC.
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

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "cerebro/cerebro_config_module.h"

#define DEFAULT_CONFIG_MODULE_NAME "default"

int
default_config_setup(void)
{
  return 0;
}

int
default_config_cleanup(void)
{
  return 0;
}

int
default_config_load_default(struct cerebro_config *conf)
{
  return 0;
}

struct cerebro_config_module_info default_config_module_info =
  {
    DEFAULT_CONFIG_MODULE_NAME,
    &default_config_setup,
    &default_config_cleanup,
    &default_config_load_default,
  };
