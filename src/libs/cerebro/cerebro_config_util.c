/*****************************************************************************\
 *  $Id: cerebro_config_util.c,v 1.8 2005-07-22 17:21:07 achu Exp $
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

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */

#include "cerebro_api.h"
#include "cerebro_config_util.h"
#include "cerebro_util.h"
#include "cerebro/cerebro_config.h"

#include "conffile.h"
#include "config_module.h"
#include "config_util.h"
#include "debug.h"

int 
_cerebro_load_config(cerebro_t handle)
{
  unsigned int errnum;

  if (_cerebro_handle_check(handle) < 0)
    return -1;

  if (handle->loaded_state & CEREBRO_CONFIG_LOADED)
    return 0;
  
  memset(&(handle->config_data), '\0', sizeof(struct cerebro_config));
  if (load_config(&(handle->config_data), &errnum) < 0)
    {
      CEREBRO_DBG(("merge_cerebro_configs"));
      handle->errnum = errnum;
      return -1;
    }
  
  handle->loaded_state |= CEREBRO_CONFIG_LOADED;
  return 0;
}

int 
_cerebro_unload_config(cerebro_t handle)
{
  if (_cerebro_handle_check(handle) < 0)
    return -1;

  memset(&(handle->config_data), '\0', sizeof(struct cerebro_config));
  
  handle->loaded_state &= ~CEREBRO_CONFIG_LOADED;
  return 0;
}
