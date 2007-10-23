/*****************************************************************************\
 *  $Id: event_module.c,v 1.7 2007-10-23 22:09:33 chu11 Exp $
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
#include "cerebro/cerebro_event_module.h"

#include "event_module.h"
#include "module_util.h"

#include "debug.h"
#if !WITH_STATIC_MODULES
#include "ltdl.h"
#endif /* !WITH_STATIC_MODULES */
#include "vector.h"

#if WITH_STATIC_MODULES

#if WITH_UPDOWN_EVENT
extern struct cerebro_event_module_info updown_event_module_info;
#endif /* WITH_UPDOWN_EVENT */

/*
 * event_modules
 *
 * event modules statically compiled in
 */
void *event_modules[] =
  {
#if WITH_UPDOWN_EVENT
    updown_event_module_info,
#endif /* WITH_UPDOWN_EVENT */
    NULL
  };

#endif /* WITH_STATIC_MODULES */

#define EVENT_FILENAME_SIGNATURE  "cerebro_event_"
#define EVENT_MODULE_INFO_SYM     "event_module_info"
#define EVENT_MODULE_DIR          EVENT_MODULE_BUILDDIR "/.libs"
#define EVENT_MODULE_MAGIC_NUMBER 0x53812232

/* 
 * struct event_module
 *
 * event module handle
 */
struct event_module
{
  int32_t magic;
  unsigned int modules_count;
#if !WITH_STATIC_MODULES
  Vector dl_handles;
#endif /* !WITH_STATIC_MODULES */
  Vector module_infos;
};

/*
 * _event_module_cb
 *
 * Check and store module
 *
 * Return 1 is module is stored, 0 if not, -1 on fatal error
 */
static int 
#if WITH_STATIC_MODULES
_event_module_cb(void *handle, void *module_info)
#else  /* !WITH_STATIC_MODULES */
_event_module_cb(void *handle, void *dl_handle, void *module_info)
#endif /* !WITH_STATIC_MODULES */
{
  event_modules_t event_handle;
  struct cerebro_event_module_info *event_module_info;
#if !WITH_STATIC_MODULES
  lt_dlhandle event_dl_handle;
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

  event_handle = handle;
#if !WITH_STATIC_MODULES
  event_dl_handle = dl_handle;
#endif /* !WITH_STATIC_MODULES */
  event_module_info = module_info;

  if (event_handle->magic != EVENT_MODULE_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("invalid handle"));
      return -1;
    }

  if (!event_module_info->event_module_name
      || !event_module_info->interface_version
      || !event_module_info->setup
      || !event_module_info->cleanup
      || !event_module_info->event_names
      || !event_module_info->metric_names
      || !event_module_info->timeout_length
      || !event_module_info->node_timeout
      || !event_module_info->metric_update
      || !event_module_info->destroy)
    {
      CEREBRO_ERR(("invalid module info, cannot load module"));
      return 0;
    }

  if (((*event_module_info->interface_version)()) != CEREBRO_EVENT_INTERFACE_VERSION)
    {
      CEREBRO_ERR(("invalid module interface version, cannot load module"));
      return 0;
    }

#if !WITH_STATIC_MODULES
  if (!vector_set(event_handle->dl_handles, 
                  event_dl_handle,
                  event_handle->modules_count))
    {
      CEREBRO_DBG(("vector_set: %s", strerror(errno)));
      return 0;
    }
#endif /* !WITH_STATIC_MODULES */
  if (!vector_set(event_handle->module_infos,
                  event_module_info, 
                  event_handle->modules_count))
    {
      CEREBRO_DBG(("vector_set: %s", strerror(errno)));
#if !WITH_STATIC_MODULES
      vector_set(event_handle->dl_handles,
                 NULL,
                 event_handle->modules_count);
#endif /* !WITH_STATIC_MODULES */
      return 0;
    }
  event_handle->modules_count++;
  return 1;
}

event_modules_t 
event_modules_load(void)
{
  struct event_module *handle = NULL;
  int rv;

  if (module_setup() < 0)
    return NULL;
                                                                                    
  if (!(handle = (struct event_module *)malloc(sizeof(struct event_module))))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      return NULL;
    }
  memset(handle, '\0', sizeof(struct event_module));
  handle->magic = EVENT_MODULE_MAGIC_NUMBER;
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
  if ((rv = find_and_load_modules(event_modules,
                                  _event_module_cb,
                                  handle,
                                  INT_MAX)) < 0)
    goto cleanup;
#else  /* !WITH_STATIC_MODULES */
  if ((rv = find_and_load_modules(EVENT_MODULE_DIR,
                                  NULL,
                                  EVENT_FILENAME_SIGNATURE,
                                  _event_module_cb,
                                  EVENT_MODULE_INFO_SYM,
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
 * Check for proper event module handle
 *
 * Returns 0 on success, -1 on error
 */
static int
_handle_check(event_modules_t handle)
{
  if (!handle || handle->magic != EVENT_MODULE_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("invalid handle"));
      return -1;
    }

  return 0;
}

/*
 * _handle_index_check
 *
 * Check for proper event module handle and index
 *
 * Returns 0 on success, -1 on error
 */
static int
_handle_index_check(event_modules_t handle, unsigned int index)
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
event_modules_unload(event_modules_t handle)
{
  int i;

  if (_handle_check(handle) < 0)
    return -1;

  for (i = 0; i < handle->modules_count; i++)
    event_module_cleanup(handle, i);

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

  handle->magic = ~EVENT_MODULE_MAGIC_NUMBER;
  handle->modules_count = 0;

  free(handle);

  module_cleanup();
  return 0;
  
}

int 
event_modules_count(event_modules_t handle)
{
  if (_handle_check(handle) < 0)
    return -1;

  return handle->modules_count;
}

char *
event_module_name(event_modules_t handle, unsigned int index)
{
  struct cerebro_event_module_info *module_info;

  if (_handle_index_check(handle, index) < 0)
    return NULL;

  if (!(module_info = vector_get(handle->module_infos, index)))
    {
      CEREBRO_DBG(("vector_get: %s", strerror(errno)));
      return NULL;
    }

  return module_info->event_module_name;
}

int
event_module_interface_version(event_modules_t handle)
{
  struct cerebro_event_module_info *module_info;

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
event_module_setup(event_modules_t handle, unsigned int index)
{
  struct cerebro_event_module_info *module_info;

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
event_module_cleanup(event_modules_t handle, unsigned int index)
{
  struct cerebro_event_module_info *module_info;

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
event_module_event_names(event_modules_t handle, unsigned int index)
{
  struct cerebro_event_module_info *module_info;

  if (_handle_index_check(handle, index) < 0)
    return NULL;

  if (!(module_info = vector_get(handle->module_infos, index)))
    {
      CEREBRO_DBG(("vector_get: %s", strerror(errno)));
      return NULL;
    }

  return ((*module_info->event_names)());
}

char *
event_module_metric_names(event_modules_t handle, unsigned int index)
{
  struct cerebro_event_module_info *module_info;

  if (_handle_index_check(handle, index) < 0)
    return NULL;

  if (!(module_info = vector_get(handle->module_infos, index)))
    {
      CEREBRO_DBG(("vector_get: %s", strerror(errno)));
      return NULL;
    }

  return ((*module_info->metric_names)());
}

int 
event_module_timeout_length(event_modules_t handle, unsigned int index)
{
  struct cerebro_event_module_info *module_info;
  
  if (_handle_index_check(handle, index) < 0)
    return -1;
  
  if (!(module_info = vector_get(handle->module_infos, index)))
    {
      CEREBRO_DBG(("vector_get: %s", strerror(errno)));
      return -1;
    }
  
  return ((*module_info->timeout_length)());
}

int 
event_module_node_timeout(event_modules_t handle,
                          unsigned int index,
                          const char *nodename,
                          struct cerebro_event **event)
{
  struct cerebro_event_module_info *module_info;

  if (_handle_index_check(handle, index) < 0)
    return -1;

  if (!(module_info = vector_get(handle->module_infos, index)))
    {
      CEREBRO_DBG(("vector_get: %s", strerror(errno)));
      return -1;
    }

  return ((*module_info->node_timeout)(nodename, event));
}

int 
event_module_metric_update(event_modules_t handle,
                           unsigned int index,
                           const char *nodename,
                           const char *metric_name,
                           unsigned int metric_value_type,
                           unsigned int metric_value_len,
                           void *metric_value,
                           struct cerebro_event **event)
{
  struct cerebro_event_module_info *module_info;

  if (_handle_index_check(handle, index) < 0)
    return -1;

  if (!(module_info = vector_get(handle->module_infos, index)))
    {
      CEREBRO_DBG(("vector_get: %s", strerror(errno)));
      return -1;
    }

  return ((*module_info->metric_update)(nodename,
                                        metric_name,
                                        metric_value_type,
                                        metric_value_len,
                                        metric_value,
                                        event));
}

void 
event_module_destroy(event_modules_t handle,
                     unsigned int index,
                     struct cerebro_event *event)
{
  struct cerebro_event_module_info *module_info;

  if (_handle_index_check(handle, index) < 0)
    return;

  if (!(module_info = vector_get(handle->module_infos, index)))
    {
      CEREBRO_DBG(("vector_get: %s", strerror(errno)));
      return;
    }

  return ((*module_info->destroy)(event));
}
