/*****************************************************************************\
 *  $Id: monitor_module.c,v 1.13 2005-08-23 21:10:14 achu Exp $
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
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_monitor_module.h"

#include "monitor_module.h"
#include "module_util.h"

#include "debug.h"
#if !WITH_STATIC_MODULES
#include "ltdl.h"
#endif /* !WITH_STATIC_MODULES */

#if WITH_STATIC_MODULES

/* 
 * The bootlog module cannot be built statically, b/c the qsql library
 * requires dynamically loadable modules
 */
#if 0
#if WITH_BOOTLOG
extern struct cerebro_monitor_module_info bootlog_monitor_module_info;
#endif /* WITH_BOOTLOG */
#endif /* 0 */

/*
 * monitor_modules
 *
 * monitor modules statically compiled in
 */
void *monitor_modules[] =
  {
#if 0
#if WITH_BOOTLOG
    &bootlog_monitor_module_info,
#endif /* WITH_BOOTLOG */
#endif /* 0 */
    NULL
  };

#endif /* WITH_STATIC_MODULES */


#define MONITOR_FILENAME_SIGNATURE  "cerebro_monitor_"
#define MONITOR_MODULE_INFO_SYM     "monitor_module_info"
#define MONITOR_MODULE_DIR          MONITOR_MODULE_BUILDDIR "/.libs"
#define MONITOR_MODULE_MAGIC_NUMBER 0x33882233

/* 
 * struct monitor_module
 *
 * monitor module handle
 */
struct monitor_module
{
  int32_t magic;
  unsigned int modules_max;
  unsigned int modules_count;
#if !WITH_STATIC_MODULES
  lt_dlhandle *dl_handle;
#endif /* !WITH_STATIC_MODULES */
  struct cerebro_monitor_module_info **module_info;
};

/*
 * _monitor_module_cb
 *
 * Check and store module
 *
 * Return 1 is module is stored, 0 if not, -1 on fatal error
 */
static int 
#if WITH_STATIC_MODULES
_monitor_module_cb(void *handle, void *module_info)
#else  /* !WITH_STATIC_MODULES */
_monitor_module_cb(void *handle, void *dl_handle, void *module_info)
#endif /* !WITH_STATIC_MODULES */
{
  monitor_modules_t monitor_handle;
  struct cerebro_monitor_module_info *monitor_module_info;
#if !WITH_STATIC_MODULES
  lt_dlhandle monitor_dl_handle;
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

  monitor_handle = handle;
#if !WITH_STATIC_MODULES
  monitor_dl_handle = dl_handle;
#endif /* !WITH_STATIC_MODULES */
  monitor_module_info = module_info;

  if (monitor_handle->magic != MONITOR_MODULE_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("invalid handle"));
      return -1;
    }

  if (!monitor_module_info->monitor_module_name
      || !monitor_module_info->setup
      || !monitor_module_info->cleanup
      || !monitor_module_info->metric_name
      || !monitor_module_info->metric_update)
    {
      CEREBRO_DBG(("invalid module info"));
      return 0;
    }

  if (monitor_handle->modules_count < monitor_handle->modules_max)
    {
#if !WITH_STATIC_MODULES
      monitor_handle->dl_handle[monitor_handle->modules_count] = monitor_dl_handle;
#endif /* !WITH_STATIC_MODULES */
      monitor_handle->module_info[monitor_handle->modules_count] = monitor_module_info;
      monitor_handle->modules_count++;
      return 1;
    }
  else
    return 0;
}

monitor_modules_t 
monitor_modules_load(unsigned int modules_max)
{
  struct monitor_module *handle = NULL;
  int rv;

  if (!modules_max)
    {
      CEREBRO_DBG(("modules_max invalid"));
      return NULL;
    }

  if (module_setup() < 0)
    return NULL;
                                                                                    
  if (!(handle = (struct monitor_module *)malloc(sizeof(struct monitor_module))))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      return NULL;
    }
  memset(handle, '\0', sizeof(struct monitor_module));
  handle->magic = MONITOR_MODULE_MAGIC_NUMBER;
  handle->modules_max = modules_max;
  handle->modules_count = 0;

#if !WITH_STATIC_MODULES
  if (!(handle->dl_handle = (lt_dlhandle *)malloc(sizeof(lt_dlhandle)*handle->modules_max)))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      goto cleanup;
    }
  memset(handle->dl_handle, '\0', sizeof(lt_dlhandle)*handle->modules_max);
#endif /* !WITH_STATIC_MODULES */
  
  if (!(handle->module_info = (struct cerebro_monitor_module_info * *)malloc(sizeof(struct cerebro_monitor_module_info *)*handle->modules_max)))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      goto cleanup;
    }
  memset(handle->module_info, '\0', sizeof(struct cerebro_monitor_module_info *)*handle->modules_max);

#if WITH_STATIC_MODULES
  if ((rv = find_and_load_modules(monitor_modules,
                                  _monitor_module_cb,
                                  handle,
                                  handle->modules_max)) < 0)
    goto cleanup;
#else  /* !WITH_STATIC_MODULES */
  if ((rv = find_and_load_modules(MONITOR_MODULE_DIR,
                                  NULL,
                                  MONITOR_FILENAME_SIGNATURE,
                                  _monitor_module_cb,
                                  MONITOR_MODULE_INFO_SYM,
                                  handle,
                                  handle->modules_max)) < 0)
    goto cleanup;
#endif /* !WITH_STATIC_MODULES */

  if (rv)
    goto out;

  /* Responsibility of caller to call count to see if no modules were
   * loaded 
   */

 out:
  return handle;

 cleanup:
  if (handle)
    {
#if !WITH_STATIC_MODULES
      if (handle->dl_handle)
        {
          int i;
          for (i = 0; i < handle->modules_count; i++)
            lt_dlclose(handle->dl_handle[i]);
          free(handle->dl_handle);
        }
#endif /* !WITH_STATIC_MODULES */
      if (handle->module_info)
        free(handle->module_info);
      free(handle);
    }
  module_cleanup();
  return NULL;
}

/*
 * _handle_check
 *
 * Check for proper monitor module handle
 *
 * Returns 0 on success, -1 on error
 */
static int
_handle_check(monitor_modules_t handle)
{
  if (!handle || handle->magic != MONITOR_MODULE_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("invalid handle"));
      return -1;
    }

  return 0;
}

/*
 * _handle_index_check
 *
 * Check for proper monitor module handle and index
 *
 * Returns 0 on success, -1 on error
 */
static int
_handle_index_check(monitor_modules_t handle, unsigned int index)
{
  if (_handle_check(handle) < 0)
    return -1;

  if (!(index < handle->modules_count))
    {
      CEREBRO_DBG(("invalid index"));
      return -1;
    }

  return 0;
}

int 
monitor_modules_unload(monitor_modules_t handle)
{
  int i;

  if (_handle_check(handle) < 0)
    return -1;

  for (i = 0; i < handle->modules_count; i++)
    monitor_module_cleanup(handle, i);

#if !WITH_STATIC_MODULES
  if (handle->dl_handle)
    {
      for (i = 0; i < handle->modules_count; i++)
        lt_dlclose(handle->dl_handle[i]);
      free(handle->dl_handle);
      handle->dl_handle = NULL;
    }
#endif /* !WITH_STATIC_MODULES */

  if (handle->module_info)
    {
      free(handle->module_info);
      handle->module_info = NULL;
    }

  handle->magic = ~MONITOR_MODULE_MAGIC_NUMBER;
  handle->modules_max = 0;
  handle->modules_count = 0;

  free(handle);

  module_cleanup();
  return 0;
  
}

int 
monitor_modules_count(monitor_modules_t handle)
{
  if (_handle_check(handle) < 0)
    return -1;

  return handle->modules_count;
}

char *
monitor_module_name(monitor_modules_t handle, unsigned int index)
{
  if (_handle_index_check(handle, index) < 0)
    return NULL;

  return (handle->module_info[index])->monitor_module_name;
}

int 
monitor_module_setup(monitor_modules_t handle, unsigned int index)
{
  if (_handle_index_check(handle, index) < 0)
    return -1;

  return ((*(handle->module_info[index])->setup)());
}

int 
monitor_module_cleanup(monitor_modules_t handle, unsigned int index)
{
  if (_handle_index_check(handle, index) < 0)
    return -1;

  return ((*(handle->module_info[index])->cleanup)());
}

char *
monitor_module_metric_name(monitor_modules_t handle, unsigned int index)
{
  if (_handle_index_check(handle, index) < 0)
    return NULL;

  return ((*(handle->module_info[index])->metric_name)());
}

int 
monitor_module_metric_update(monitor_modules_t handle,
			     unsigned int index,
			     const char *nodename,
			     unsigned int metric_value_type,
			     unsigned int metric_value_len,
			     void *metric_value)
{
  if (_handle_index_check(handle, index) < 0)
    return -1;

  return ((*(handle->module_info[index])->metric_update)(nodename,
                                                         metric_value_type,
                                                         metric_value_len,
                                                         metric_value));
}
