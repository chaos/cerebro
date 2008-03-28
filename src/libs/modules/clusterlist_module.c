/*****************************************************************************\
 *  $Id: clusterlist_module.c,v 1.18 2008-03-28 17:06:48 chu11 Exp $
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
#include "cerebro/cerebro_clusterlist_module.h"
#include "cerebro/cerebro_constants.h"

#include "clusterlist_module.h"
#include "module_util.h"

#include "debug.h"
#if !WITH_STATIC_MODULES
#include "ltdl.h"
#endif /* !WITH_STATIC_MODULES */

#if WITH_STATIC_MODULES

#if WITH_GENDERSLLNL
extern struct cerebro_clusterlist_module_info gendersllnl_clusterlist_module_info;
#endif /* WITH_GENDERSLLNL */

#if WITH_GENDERS
extern struct cerebro_clusterlist_module_info genders_clusterlist_module_info;
#endif /* WITH_GENDERS */

#if WITH_HOSTSFILE
extern struct cerebro_clusterlist_module_info hostsfile_clusterlist_module_info;
#endif /* WITH_HOSTSFILE */

/*
 * clusterlist_modules
 *
 * clusterlist modules statically compiled in
 */
void *clusterlist_modules[] =
  {
#if WITH_GENDERSLLNL
    &gendersllnl_clusterlist_module_info,
#endif /* WITH_GENDERSLLNL */
#if WITH_GENDERS
    &genders_clusterlist_module_info,
#endif /* WITH_GENDERS */
#if WITH_HOSTSFILE
    &hostsfile_clusterlist_module_info,
#endif /* WITH_HOSTSFILE */
    NULL
  };

#else /* !WITH_STATIC_MODULES */

/*
 * clusterlist_modules
 *
 * dynamic clusterlist modules to search for by default
 */
char *clusterlist_modules[] = {
  "cerebro_clusterlist_gendersllnl.so",
  "cerebro_clusterlist_genders.so",
  "cerebro_clusterlist_hostsfile.so",
  NULL
};

#endif /* !WITH_STATIC_MODULES */

#define CLUSTERLIST_FILENAME_SIGNATURE  "cerebro_clusterlist_"
#define CLUSTERLIST_MODULE_INFO_SYM     "clusterlist_module_info"
#define CLUSTERLIST_MODULE_DIR          CLUSTERLIST_MODULE_BUILDDIR "/.libs"
#define CLUSTERLIST_MODULE_MAX          1
#define CLUSTERLIST_MODULE_MAGIC_NUMBER 0x33882200

/* 
 * struct clusterlist_module
 *
 * clusterlist module handle
 */
struct clusterlist_module
{
  int32_t magic;
#if !WITH_STATIC_MODULES
  lt_dlhandle dl_handle;
#endif /* !WITH_STATIC_MODULES */
  struct cerebro_clusterlist_module_info *module_info;
};

/* 
 * _clusterlist_module_cb
 *
 * Check and store module
 *
 * Return 1 is module is stored, 0 if not, -1 on fatal error
 */
static int
#if WITH_STATIC_MODULES
_clusterlist_module_cb(void *handle, void *module_info)
#else  /* !WITH_STATIC_MODULES */
_clusterlist_module_cb(void *handle, void *dl_handle, void *module_info)
#endif /* !WITH_STATIC_MODULES */
{
  clusterlist_module_t clusterlist_handle;
  struct cerebro_clusterlist_module_info *clusterlist_module_info;
#if !WITH_STATIC_MODULES
  lt_dlhandle clusterlist_dl_handle;
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

  clusterlist_handle = handle;
#if !WITH_STATIC_MODULES
  clusterlist_dl_handle = dl_handle;
#endif /* !WITH_STATIC_MODULES */
  clusterlist_module_info = module_info;
    
  if (clusterlist_handle->magic != CLUSTERLIST_MODULE_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("invalid handle"));
      return -1;
    }

  if (!clusterlist_module_info->clusterlist_module_name
      || !clusterlist_module_info->interface_version
      || !clusterlist_module_info->setup
      || !clusterlist_module_info->cleanup
      || !clusterlist_module_info->numnodes
      || !clusterlist_module_info->get_all_nodes
      || !clusterlist_module_info->node_in_cluster
      || !clusterlist_module_info->get_nodename)
    {
      CEREBRO_ERR(("invalid module info, cannot load module"));
      return 0;
    }

  if (((*clusterlist_module_info->interface_version)()) != CEREBRO_CLUSTERLIST_INTERFACE_VERSION)
    {
      CEREBRO_ERR(("invalid module interface version, cannot load module"));
      return 0;
    }

#if !WITH_STATIC_MODULES
  clusterlist_handle->dl_handle = clusterlist_dl_handle;
#endif /* !WITH_STATIC_MODULES */
  clusterlist_handle->module_info = clusterlist_module_info;
  return 1;
}

clusterlist_module_t 
clusterlist_module_load(void)
{
  struct clusterlist_module *handle = NULL;
  int rv;
  
  if (module_setup() < 0)
    return NULL;

  if (!(handle = (struct clusterlist_module *)malloc(sizeof(struct clusterlist_module))))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      return NULL;
    }
  memset(handle, '\0', sizeof(struct clusterlist_module));
  handle->magic = CLUSTERLIST_MODULE_MAGIC_NUMBER;
      
#if WITH_STATIC_MODULES
  if ((rv = find_and_load_modules(clusterlist_modules,
                                  _clusterlist_module_cb,
                                  handle,
                                  CLUSTERLIST_MODULE_MAX)) < 0)
    goto cleanup;
#else  /* !WITH_STATIC_MODULES */
  if ((rv = find_and_load_modules(CLUSTERLIST_MODULE_DIR,
                                  clusterlist_modules,
                                  CLUSTERLIST_FILENAME_SIGNATURE,
                                  _clusterlist_module_cb,
                                  CLUSTERLIST_MODULE_INFO_SYM,
                                  handle,
                                  CLUSTERLIST_MODULE_MAX)) < 0)
    goto cleanup;
#endif  /* !WITH_STATIC_MODULES */
  
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
 * Check for proper clusterlist module handle
 *
 * Returns 0 on success, -1 on error
 */
static int
_handle_check(clusterlist_module_t handle)
{
  if (!handle || handle->magic != CLUSTERLIST_MODULE_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("invalid handle"));
      return -1;
    }

  return 0;
}

/*
 * _handle_info_check
 *
 * Check for proper clusterlist module handle and module_info
 *
 * Returns 0 on success, -1 on error
 */
static int
_handle_info_check(clusterlist_module_t handle)
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
clusterlist_module_unload(clusterlist_module_t handle)
{
  if (_handle_check(handle) < 0)
    return -1;

  if (handle->module_info)
    clusterlist_module_cleanup(handle);

  handle->magic = ~CLUSTERLIST_MODULE_MAGIC_NUMBER;
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
clusterlist_module_found(clusterlist_module_t handle)
{
  if (_handle_check(handle) < 0)
    return -1;

  return (handle->module_info) ? 1 : 0;
}

char *
clusterlist_module_name(clusterlist_module_t handle)
{
  if (_handle_info_check(handle) < 0)
    return NULL;

  return (handle->module_info)->clusterlist_module_name;
}

int
clusterlist_module_interface_version(clusterlist_module_t handle)
{
  if (_handle_info_check(handle) < 0)
    return -1;
  
  return ((*(handle->module_info)->interface_version)());
}

int
clusterlist_module_setup(clusterlist_module_t handle)
{
  if (_handle_info_check(handle) < 0)
    return -1;
  
  return ((*(handle->module_info)->setup)());
}

int
clusterlist_module_cleanup(clusterlist_module_t handle)
{
  if (_handle_info_check(handle) < 0)
    return -1;
  
  return ((*(handle->module_info)->cleanup)());
}

int
clusterlist_module_numnodes(clusterlist_module_t handle)
{
  if (_handle_info_check(handle) < 0)
    return -1;
  
  return ((*(handle->module_info)->numnodes)());
}

int
clusterlist_module_get_all_nodes(clusterlist_module_t handle, char ***nodes)
{
  if (_handle_info_check(handle) < 0)
    return -1;
  
  return ((*(handle->module_info)->get_all_nodes)(nodes));
}

int
clusterlist_module_node_in_cluster(clusterlist_module_t handle, const char *node)
{
  if (_handle_info_check(handle) < 0)
    return -1;
  
  return ((*(handle->module_info)->node_in_cluster)(node));
}

int
clusterlist_module_get_nodename(clusterlist_module_t handle,
				const char *node, 
				char *buf, 
				unsigned int buflen)
{
  if (_handle_info_check(handle) < 0)
    return -1;
  
  return ((*(handle->module_info)->get_nodename)(node, buf, buflen));
}
