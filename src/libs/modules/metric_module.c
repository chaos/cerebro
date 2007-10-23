/*****************************************************************************\
 *  $Id: metric_module.c,v 1.26 2007-10-23 22:09:33 chu11 Exp $
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

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <limits.h>
#include <errno.h>

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_metric_module.h"

#include "metric_module.h"
#include "module_util.h"

#include "debug.h"
#if !WITH_STATIC_MODULES
#include "ltdl.h"
#endif /* !WITH_STATIC_MODULES */
#include "vector.h"

#if WITH_STATIC_MODULES

#if WITH_BOOTTIME
extern struct cerebro_metric_module_info boottime_metric_module_info;
#endif /* WITH_BOOTTIME */

#if WITH_SLURM_STATE
extern struct cerebro_metric_module_info slurm_state_metric_module_info;
#endif /* WITH_SLURM_STATE */

#if WITH_LOADAVG
extern struct cerebro_metric_module_info loadavg1_metric_module_info;
extern struct cerebro_metric_module_info loadavg5_metric_module_info;
extern struct cerebro_metric_module_info loadavg15_metric_module_info;
#endif /* WITH_LOADAVG */

#if WITH_MEMORY
extern struct cerebro_metric_module_info memtotal_metric_module_info;
extern struct cerebro_metric_module_info memused_metric_module_info;
extern struct cerebro_metric_module_info memfree_metric_module_info;
extern struct cerebro_metric_module_info swaptotal_metric_module_info;
extern struct cerebro_metric_module_info swapused_metric_module_info;
extern struct cerebro_metric_module_info swapfree_metric_module_info;
#endif /* WITH_MEMORY */

#if WITH_NETWORK
extern struct cerebro_metric_module_info bytesin_metric_module_info;
extern struct cerebro_metric_module_info bytesout_metric_module_info;
extern struct cerebro_metric_module_info packetsin_metric_module_info;
extern struct cerebro_metric_module_info packetsout_metric_module_info;
extern struct cerebro_metric_module_info rxerrs_metric_module_info;
extern struct cerebro_metric_module_info txerrs_metric_module_info;
#endif /* WITH_NETWORK */

/*
 * metric_modules
 *
 * metric modules statically compiled in
 */
void *metric_modules[] =
  {
#if WITH_BOOTTIME
    &boottime_metric_module_info,
#endif /* WITH_BOOTTIME */
#if WITH_SLURM_STATE
    &slurm_state_metric_module_info,
#endif /* WITH_SLURM_STATE */
#if WITH_LOADAVG
    &loadavg1_metric_module_info,
    &loadavg5_metric_module_info,
    &loadavg15_metric_module_info,
#endif /* WITH_LOADAVG */
#if WITH_MEMORY
    &memtotal_metric_module_info,
    &memused_metric_module_info,
    &memfree_metric_module_info,
    &swaptotal_metric_module_info,
    &swapused_metric_module_info,
    &swapfree_metric_module_info,
#endif /* WITH_MEMORY */
#if WITH_NETWORK
    &bytesin_metric_module_info,
    &bytesout_metric_module_info,
    &packetsin_metric_module_info,
    &packetsout_metric_module_info,
    &rxerrs_metric_module_info,
    &txerrs_metric_module_info,
#endif /* WITH_NETWORK */
    NULL
  };

#endif /* WITH_STATIC_MODULES */

#define METRIC_FILENAME_SIGNATURE  "cerebro_metric_"
#define METRIC_MODULE_INFO_SYM     "metric_module_info"
#define METRIC_MODULE_DIR          METRIC_MODULE_BUILDDIR "/.libs"
#define METRIC_MODULE_MAGIC_NUMBER 0x33882222

/* 
 * struct metric_module
 *
 * metric module handle
 */
struct metric_module
{
  int32_t magic;
  unsigned int modules_count;
#if !WITH_STATIC_MODULES
  Vector dl_handles;
#endif /* !WITH_STATIC_MODULES */
  Vector module_infos;
};

/*
 * _metric_module_cb
 *
 * Check and store module
 *
 * Return 1 is module is stored, 0 if not, -1 on fatal error
 */
static int 
#if WITH_STATIC_MODULES
_metric_module_cb(void *handle, void *module_info)
#else  /* !WITH_STATIC_MODULES */
_metric_module_cb(void *handle, void *dl_handle, void *module_info)
#endif /* !WITH_STATIC_MODULES */
{
  metric_modules_t metric_handle;
  struct cerebro_metric_module_info *metric_module_info;
#if !WITH_STATIC_MODULES
  lt_dlhandle metric_dl_handle;
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
  
  metric_handle = handle;
#if !WITH_STATIC_MODULES
  metric_dl_handle = dl_handle;
#endif /* !WITH_STATIC_MODULES */
  metric_module_info = module_info;
  
  if (metric_handle->magic != METRIC_MODULE_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("invalid handle"));
      return -1;
    }
  
  if (!metric_module_info->metric_module_name
      || !metric_module_info->interface_version
      || !metric_module_info->setup
      || !metric_module_info->cleanup
      || !metric_module_info->get_metric_name
      || !metric_module_info->get_metric_period
      || !metric_module_info->get_metric_flags
      || !metric_module_info->get_metric_value
      || !metric_module_info->destroy_metric_value
      || !metric_module_info->get_metric_thread)
    {
      CEREBRO_ERR(("invalid module info, cannot load module"));
      return 0;
    }

  if (((*metric_module_info->interface_version)()) != CEREBRO_METRIC_INTERFACE_VERSION)
    {
      CEREBRO_ERR(("invalid module interface version, cannot load module"));
      return 0;
    }

#if !WITH_STATIC_MODULES
  if (!vector_set(metric_handle->dl_handles, 
                  metric_dl_handle,
                  metric_handle->modules_count))
    {
      CEREBRO_DBG(("vector_set: %s", strerror(errno)));
      return 0;
    }
#endif /* !WITH_STATIC_MODULES */
  if (!vector_set(metric_handle->module_infos,
                  metric_module_info, 
                  metric_handle->modules_count))
    {
      CEREBRO_DBG(("vector_set: %s", strerror(errno)));
#if !WITH_STATIC_MODULES
      vector_set(metric_handle->dl_handles,
                 NULL,
                 metric_handle->modules_count);
#endif /* !WITH_STATIC_MODULES */
      return 0;
    }
  metric_handle->modules_count++;
  return 1;
}

metric_modules_t 
metric_modules_load(void)
{
  struct metric_module *handle = NULL;
  int rv;

  if (module_setup() < 0)
    return NULL;

  if (!(handle = (struct metric_module *)malloc(sizeof(struct metric_module))))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      return NULL;
    }
  memset(handle, '\0', sizeof(struct metric_module));
  handle->magic = METRIC_MODULE_MAGIC_NUMBER;
  handle->modules_count = 0;

#if !WITH_STATIC_MODULES
  if (!(handle->dl_handles = vector_create((VectorDelF)lt_dlclose)))
    {
      CEREBRO_DBG(("vector_create: %s", strerror(errno)));
      goto cleanup;
    }
#endif /* !WITH_STATIC_MODULES */
  
  if (!(handle->module_infos = vector_create(NULL)))
    {
      CEREBRO_DBG(("vector_create: %s", strerror(errno)));
      goto cleanup;
    }

#if WITH_STATIC_MODULES
  if ((rv = find_and_load_modules(metric_modules,
                                  _metric_module_cb,
                                  handle,
                                  INT_MAX)) < 0)
    goto cleanup;
#else  /* !WITH_STATIC_MODULES */
  if ((rv = find_and_load_modules(METRIC_MODULE_DIR,
                                  NULL,
                                  METRIC_FILENAME_SIGNATURE,
                                  _metric_module_cb,
                                  METRIC_MODULE_INFO_SYM,
                                  handle,
                                  INT_MAX)) < 0)
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
      if (handle->dl_handles)
        vector_destroy(handle->dl_handles);
#endif /* !WITH_STATIC_MODULES */
      if (handle->module_infos)
        vector_destroy(handle->module_infos);
      free(handle);
    }
  module_cleanup();
  return NULL;
}

/*
 * _handle_check
 *
 * Check for proper metric module handle
 *
 * Returns 0 on success, -1 on error
 */
static int
_handle_check(metric_modules_t handle)
{
  if (!handle || handle->magic != METRIC_MODULE_MAGIC_NUMBER)
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
_handle_index_check(metric_modules_t handle, unsigned int index)
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
metric_modules_unload(metric_modules_t handle)
{
  int i;

  if (_handle_check(handle) < 0)
    return -1;

  for (i = 0; i < handle->modules_count; i++)
    metric_module_cleanup(handle, i);

#if !WITH_STATIC_MODULES
  if (handle->dl_handles)
    {
      vector_destroy(handle->dl_handles);
      handle->dl_handles = NULL;
    }
#endif /* !WITH_STATIC_MODULES */

  if (handle->module_infos)
    {
      vector_destroy(handle->module_infos);
      handle->module_infos = NULL;
    }

  handle->magic = ~METRIC_MODULE_MAGIC_NUMBER;
  handle->modules_count = 0;

  free(handle);

  module_cleanup();
  return 0;
  
}

int 
metric_modules_count(metric_modules_t handle)
{
  if (_handle_check(handle) < 0)
    return -1;

  return handle->modules_count;
}

char *
metric_module_name(metric_modules_t handle, unsigned int index)
{
  struct cerebro_metric_module_info *module_info;

  if (_handle_index_check(handle, index) < 0)
    return NULL;

  if (!(module_info = vector_get(handle->module_infos, index)))
    {
      CEREBRO_DBG(("vector_get: %s", strerror(errno)));
      return NULL;
    }

  return module_info->metric_module_name;
}

int
metric_module_interface_version(metric_modules_t handle)
{
  struct cerebro_metric_module_info *module_info;

  if (_handle_index_check(handle, index) < 0)
    return -1;

  if (!(module_info = vector_get(handle->module_infos, index)))
    {
      CEREBRO_DBG(("vector_get: %s", strerror(errno)));
      return -1;
    }

  return ((*module_info->interface_version)());
}

int 
metric_module_setup(metric_modules_t handle, unsigned int index)
{
  struct cerebro_metric_module_info *module_info;

  if (_handle_index_check(handle, index) < 0)
    return -1;
  
  if (!(module_info = vector_get(handle->module_infos, index)))
    {
      CEREBRO_DBG(("vector_get: %s", strerror(errno)));
      return -1;
    }

  return ((*module_info->setup)());
}

int 
metric_module_cleanup(metric_modules_t handle, unsigned int index)
{
  struct cerebro_metric_module_info *module_info;

  if (_handle_index_check(handle, index) < 0)
    return -1;
  
  if (!(module_info = vector_get(handle->module_infos, index)))
    {
      CEREBRO_DBG(("vector_get: %s", strerror(errno)));
      return -1;
    }

  return ((*module_info->cleanup)());
}

char *
metric_module_get_metric_name(metric_modules_t handle, unsigned int index)
{
  struct cerebro_metric_module_info *module_info;

  if (_handle_index_check(handle, index) < 0)
    return NULL;
  
  if (!(module_info = vector_get(handle->module_infos, index)))
    {
      CEREBRO_DBG(("vector_get: %s", strerror(errno)));
      return NULL;
    }

  return ((*module_info->get_metric_name)());
}

int
metric_module_get_metric_period(metric_modules_t handle, 
                                unsigned int index,
                                int *period)
{
  struct cerebro_metric_module_info *module_info;

  if (_handle_index_check(handle, index) < 0)
    return -1;
  
  if (!(module_info = vector_get(handle->module_infos, index)))
    {
      CEREBRO_DBG(("vector_get: %s", strerror(errno)));
      return -1;
    }

  return ((*module_info->get_metric_period)(period));
}

int
metric_module_get_metric_flags(metric_modules_t handle, 
                               unsigned int index,
                               u_int32_t *flags)
{
  struct cerebro_metric_module_info *module_info;

  if (_handle_index_check(handle, index) < 0)
    return -1;
  
  if (!(module_info = vector_get(handle->module_infos, index)))
    {
      CEREBRO_DBG(("vector_get: %s", strerror(errno)));
      return -1;
    }

  return ((*module_info->get_metric_flags)(flags));
}

int 
metric_module_get_metric_value(metric_modules_t handle,
			       unsigned int index,
			       unsigned int *metric_value_type,
			       unsigned int *metric_value_len,
			       void **metric_value)
{
  struct cerebro_metric_module_info *module_info;

  if (_handle_index_check(handle, index) < 0)
    return -1;
  
  if (!(module_info = vector_get(handle->module_infos, index)))
    {
      CEREBRO_DBG(("vector_get: %s", strerror(errno)));
      return -1;
    }

  return ((*module_info->get_metric_value)(metric_value_type,
                                           metric_value_len,
                                           metric_value));
}

int 
metric_module_destroy_metric_value(metric_modules_t handle,
				   unsigned int index,
				   void *metric_value)
{
  struct cerebro_metric_module_info *module_info;

  if (_handle_index_check(handle, index) < 0)
    return -1;
  
  if (!(module_info = vector_get(handle->module_infos, index)))
    {
      CEREBRO_DBG(("vector_get: %s", strerror(errno)));
      return -1;
    }

  return ((*module_info->destroy_metric_value)(metric_value));
}

Cerebro_metric_thread_pointer 
metric_module_get_metric_thread(metric_modules_t handle,
                                unsigned int index)
{
  struct cerebro_metric_module_info *module_info;

  if (_handle_index_check(handle, index) < 0)
    return NULL;
  
  if (!(module_info = vector_get(handle->module_infos, index)))
    {
      CEREBRO_DBG(("vector_get: %s", strerror(errno)));
      return NULL;
    }

  return ((*module_info->get_metric_thread)());
}

int 
metric_module_send_message_function_pointer(metric_modules_t handle,
                                            unsigned int index,
                                            Cerebro_metric_send_message function_pointer)
{
  struct cerebro_metric_module_info *module_info;

  if (_handle_index_check(handle, index) < 0)
    return -1;
  
  if (!(module_info = vector_get(handle->module_infos, index)))
    {
      CEREBRO_DBG(("vector_get: %s", strerror(errno)));
      return -1;
    }

  return ((*module_info->send_message_function_pointer)(function_pointer));
}
