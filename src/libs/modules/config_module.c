/*****************************************************************************\
 *  $Id: config_module.c,v 1.24 2010-02-02 01:01:20 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2018 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2005-2007 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <https://github.com/chaos/cerebro>.
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
#if !WITH_STATIC_MODULES
#include "ltdl.h"
#endif /* !WITH_STATIC_MODULES */

#if WITH_STATIC_MODULES

/*
 * config_modules
 *
 * config modules statically compiled in
 */
void *config_modules[] =
  {
    NULL
  };

#else /* !WITH_STATIC_MODULES */

/*
 * config_modules
 *
 * dynamic configuration modules to search for by default
 */
char *config_modules[] = {
  NULL
};

#endif /* !WITH_STATIC_MODULES */

#define CONFIG_FILENAME_SIGNATURE      "cerebro_config_"
#define CONFIG_MODULE_INFO_SYM         "config_module_info"
#define CONFIG_MODULE_DIR              CONFIG_MODULE_BUILDDIR "/.libs"
#define CONFIG_MODULE_MAX              1
#define CONFIG_MODULE_MAGIC_NUMBER     0x33882211

/*
 * struct config_module
 *
 * config module handle
 */
struct config_module
{
  int32_t magic;
#if !WITH_STATIC_MODULES
  lt_dlhandle dl_handle;
#endif /* !WITH_STATIC_MODULES */
  struct cerebro_config_module_info *module_info;
};

/*
 * _config_module_cb
 *
 * Check and store module
 *
 * Return 1 is module is stored, 0 if not, -1 on fatal error
 */
static int
#if WITH_STATIC_MODULES
_config_module_cb(void *handle, void *module_info)
#else  /* !WITH_STATIC_MODULES */
_config_module_cb(void *handle, void *dl_handle, void *module_info)
#endif /* !WITH_STATIC_MODULES */
{
  config_module_t config_handle;
  struct cerebro_config_module_info *config_module_info;
#if !WITH_STATIC_MODULES
  lt_dlhandle config_dl_handle;
#endif /* !WITH_STATIC_MODULES */

#if WITH_STATIC_MODULES
  if (!handle || !module_info)
#else /* !WITH_STATIC_MODULES */
  if (!handle || !dl_handle || !module_info)
#endif /* !WITH_STATIC_MODULES */
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  config_handle = handle;
#if !WITH_STATIC_MODULES
  config_dl_handle = dl_handle;
#endif /* !WITH_STATIC_MODULES */
  config_module_info = module_info;

  if (config_handle->magic != CONFIG_MODULE_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("invalid handle"));
      return -1;
    }

  if (!config_module_info->config_module_name
      || !config_module_info->interface_version
      || !config_module_info->setup
      || !config_module_info->cleanup
      || !config_module_info->load_config)
    {
      CEREBRO_ERR(("invalid module info, cannot load module"));
      return 0;
    }

  if (((*config_module_info->interface_version)()) != CEREBRO_CONFIG_INTERFACE_VERSION)
    {
      CEREBRO_ERR(("invalid module interface version, cannot load module"));
      return 0;
    }

#if !WITH_STATIC_MODULES
  config_handle->dl_handle = config_dl_handle;
#endif /* !WITH_STATIC_MODULES */
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
      CEREBRO_ERR(("malloc: %s", strerror(errno)));
      return NULL;
    }
  memset(handle, '\0', sizeof(struct config_module));
  handle->magic = CONFIG_MODULE_MAGIC_NUMBER;

#if WITH_STATIC_MODULES
  if ((rv = find_and_load_modules(config_modules,
                                  _config_module_cb,
                                  handle,
                                  CONFIG_MODULE_MAX)) < 0)
    goto cleanup;
#else  /* !WITH_STATIC_MODULES */
  if ((rv = find_and_load_modules(CONFIG_MODULE_DIR,
                                  config_modules,
                                  CONFIG_FILENAME_SIGNATURE,
                                  _config_module_cb,
                                  CONFIG_MODULE_INFO_SYM,
                                  handle,
                                  CONFIG_MODULE_MAX)) < 0)
    goto cleanup;
#endif /* !WITH_STATIC_MODULES */

  if (rv)
    goto out;

#if !WITH_STATIC_MODULES
  handle->dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */

  /* Responsibility of caller to call found to see if a module was
   * loaded
   */

 out:
  return handle;

 cleanup:
  if (handle)
    {
#if !WITH_STATIC_MODULES
      if (handle->dl_handle)
        lt_dlclose(handle->dl_handle);
#endif /* !WITH_STATIC_MODULES */
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
  if (!handle || handle->magic != CONFIG_MODULE_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("invalid handle"));
      return -1;
    }

  return 0;
}

/*
 * _handle_info_check
 *
 * Check for proper config module handle and module_info
 *
 * Returns 0 on success, -1 on error
 */
static int
_handle_info_check(config_module_t handle)
{
  if (_handle_check(handle) < 0)
    return -1;

  if (!handle->module_info)
    {
      CEREBRO_DBG(("module not loaded"));
      return -1;
    }

  return 0;
}

int
config_module_unload(config_module_t handle)
{
  if (_handle_check(handle) < 0)
    return -1;

  if (handle->module_info)
    config_module_cleanup(handle);

  handle->magic = ~CONFIG_MODULE_MAGIC_NUMBER;
#if !WITH_STATIC_MODULES
  if (handle->dl_handle)
    lt_dlclose(handle->dl_handle);
#endif /* !WITH_STATIC_MODULES */
  handle->module_info = NULL;
  free(handle);

  module_cleanup();
  return 0;
}

int
config_module_found(config_module_t handle)
{
  if (_handle_check(handle) < 0)
    return -1;

  return (handle->module_info) ? 1 : 0;
}

char *
config_module_name(config_module_t handle)
{
  if (_handle_info_check(handle) < 0)
    return NULL;

  return (handle->module_info)->config_module_name;
}

int
config_module_interface_version(config_module_t handle)
{
  if (_handle_info_check(handle) < 0)
    return -1;

  return ((*(handle->module_info)->interface_version)());
}

int
config_module_setup(config_module_t handle)
{
  if (_handle_info_check(handle) < 0)
    return -1;

  return ((*(handle->module_info)->setup)());
}

int
config_module_cleanup(config_module_t handle)
{
  if (_handle_info_check(handle) < 0)
    return -1;

  return ((*(handle->module_info)->cleanup)());
}

int
config_module_load_config(config_module_t handle, struct cerebro_config *conf)
{
  if (_handle_info_check(handle) < 0)
    return -1;

  return ((*(handle->module_info)->load_config)(conf));
}
