/*****************************************************************************\
 *  $Id: config_module.c,v 1.14 2005-08-19 00:04:10 achu Exp $
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
#include <errno.h>

#include "cerebro.h"
#include "cerebro/cerebro_config_module.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_error.h"

#include "config_module.h"
#include "module_util.h"

#include "debug.h"
#include "ltdl.h"

/*
 * config_modules
 * config_modules_len
 *
 * dynamic configuration modules to search for by default
 */
char *config_modules[] = {
  "cerebro_config_chaos.so",
  "cerebro_config_bgl.so",
  NULL
};
int config_modules_len = 1;

#define CONFIG_FILENAME_SIGNATURE      "cerebro_config_"
#define CONFIG_MODULE_INFO_SYM         "config_module_info"
#define CONFIG_MODULE_DIR              CONFIG_MODULE_BUILDDIR "/.libs"
#define CONFIG_MODULE_MAGIC_NUMBER     0x33882211

/* 
 * struct config_module
 *
 * config module handle
 */
struct config_module
{
  int32_t magic;
  lt_dlhandle dl_handle;
  struct cerebro_config_module_info *module_info;
};

extern struct cerebro_config_module_info default_config_module_info;

/*
 * _config_module_cb
 *
 * Check and store module
 *
 * Return 1 is module is stored, 0 if not, -1 on fatal error
 */
static int
_config_module_cb(void *handle, void *dl_handle, void *module_info)
{
  config_module_t config_handle;
  struct cerebro_config_module_info *config_module_info;
  lt_dlhandle config_dl_handle;
  
  if (!handle || !dl_handle || !module_info)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  config_handle = handle;
  config_module_info = module_info;
  config_dl_handle = dl_handle;

  if (config_handle->magic != CONFIG_MODULE_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("invalid handle"));
      return -1;
    }

  if (!config_module_info->config_module_name
      || !config_module_info->setup
      || !config_module_info->cleanup
      || !config_module_info->load_config)
    {
      CEREBRO_DBG(("invalid module info"));
      return 0;
    }

  config_handle->dl_handle = config_dl_handle;
  config_handle->module_info = config_module_info;
  return 1;
}

config_module_t
config_module_load(void)
{
  struct config_module *handle = NULL;
  int rv;

  if (module_setup() < 0)
    return NULL;

  if (!(handle = (struct config_module *)malloc(sizeof(struct config_module))))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      return NULL;
    }
  memset(handle, '\0', sizeof(struct config_module));
  handle->magic = CONFIG_MODULE_MAGIC_NUMBER;

  if ((rv = find_and_load_modules(CONFIG_MODULE_DIR,
                                  config_modules,
                                  config_modules_len,
                                  CONFIG_FILENAME_SIGNATURE,
                                  _config_module_cb,
                                  CONFIG_MODULE_INFO_SYM,
                                  handle,
                                  1)) < 0)
    goto cleanup;

  if (rv)
    goto out;

  handle->dl_handle = NULL;
  handle->module_info = &default_config_module_info;
 out:
  return handle;

 cleanup:
  if (handle)
    {
      if (handle->dl_handle)
        lt_dlclose(handle->dl_handle);
      free(handle);
    }
  module_cleanup();
  return NULL;
}

/*
 * _handle_check
 *
 * Check for proper config module handle
 *
 * Returns 0 on success, -1 on error
 */
static int
_handle_check(config_module_t handle)
{
  if (!handle 
      || handle->magic != CONFIG_MODULE_MAGIC_NUMBER 
      || !handle->module_info)
    {
      CEREBRO_DBG(("invalid handle"));
      return -1;
    }

  return 0;
}

int
config_module_unload(config_module_t handle)
{
  if (_handle_check(handle) < 0)
    return -1;
  
  config_module_cleanup(handle);

  handle->magic = ~CONFIG_MODULE_MAGIC_NUMBER;
  if (handle->dl_handle)
    lt_dlclose(handle->dl_handle);
  handle->module_info = NULL;
  free(handle);

  module_cleanup();
  return 0;
}

char *
config_module_name(config_module_t handle)
{
  if (_handle_check(handle) < 0)
    return NULL;

  return (handle->module_info)->config_module_name;
}

int
config_module_setup(config_module_t handle)
{
  if (_handle_check(handle) < 0)
    return -1;
  
  return ((*(handle->module_info)->setup)());
}

int
config_module_cleanup(config_module_t handle)
{
  if (_handle_check(handle) < 0)
    return -1;
  
  return ((*(handle->module_info)->cleanup)());
}

int
config_module_load_config(config_module_t handle, struct cerebro_config *conf)
{
  if (_handle_check(handle) < 0)
    return -1;
  
  return ((*(handle->module_info)->load_config)(conf));
}
