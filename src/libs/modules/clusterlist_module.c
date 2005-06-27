/*****************************************************************************\
 *  $Id: clusterlist_module.c,v 1.8 2005-06-27 23:27:06 achu Exp $
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
#include "ltdl.h"

/*
 * clusterlist_modules
 * clusterlist_modules_len
 *
 * dynamic clusterlist modules to search for by default
 */
char *clusterlist_modules[] = {
  "cerebro_clusterlist_gendersllnl.so",
  "cerebro_clusterlist_genders.so",
  "cerebro_clusterlist_hostsfile.so",
  NULL
};
int clusterlist_modules_len = 3;

#define CLUSTERLIST_FILENAME_SIGNATURE  "cerebro_clusterlist_"
#define CLUSTERLIST_MODULE_INFO_SYM     "clusterlist_module_info"
#define CLUSTERLIST_MODULE_DIR          CLUSTERLIST_MODULE_BUILDDIR "/.libs"
#define CLUSTERLIST_MODULE_MAGIC_NUMBER 0x33882200

/* 
 * struct clusterlist_module
 *
 * clusterlist module handle
 */
struct clusterlist_module
{
  int32_t magic;
  lt_dlhandle dl_handle;
  struct cerebro_clusterlist_module_info *module_info;
};

extern struct cerebro_clusterlist_module_info default_clusterlist_module_info;

/* 
 * _clusterlist_module_cb
 *
 * Check and store module
 *
 * Return 1 is module is stored, 0 if not, -1 on fatal error
 */
static int
_clusterlist_module_cb(void *handle, void *dl_handle, void *module_info)
{
  clusterlist_module_t clusterlist_handle;
  struct cerebro_clusterlist_module_info *clusterlist_module_info;
  lt_dlhandle clusterlist_dl_handle;

  if (!handle || !dl_handle || !module_info)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  clusterlist_handle = handle;
  clusterlist_module_info = module_info;
  clusterlist_dl_handle = dl_handle;
    
  if (clusterlist_handle->magic != CLUSTERLIST_MODULE_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("invalid handle"));
      return -1;
    }

  if (!clusterlist_module_info->clusterlist_module_name
      || !clusterlist_module_info->setup
      || !clusterlist_module_info->cleanup
      || !clusterlist_module_info->numnodes
      || !clusterlist_module_info->get_all_nodes
      || !clusterlist_module_info->node_in_cluster
      || !clusterlist_module_info->get_nodename)
    {
      CEREBRO_DBG(("invalid module info"));
      return 0;
    }

  clusterlist_handle->dl_handle = clusterlist_dl_handle;
  clusterlist_handle->module_info = clusterlist_module_info;
  return 1;
}

clusterlist_module_t 
clusterlist_module_load(void)
{
  struct clusterlist_module *clusterlist_handle = NULL;
  int rv;
  
  if (module_setup() < 0)
    return NULL;

  if (!(clusterlist_handle = (struct clusterlist_module *)malloc(sizeof(struct clusterlist_module))))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      return NULL;
    }
  memset(clusterlist_handle, '\0', sizeof(struct clusterlist_module));
  clusterlist_handle->magic = CLUSTERLIST_MODULE_MAGIC_NUMBER;
      
  if ((rv = find_and_load_modules(CLUSTERLIST_MODULE_DIR,
                                  clusterlist_modules,
                                  clusterlist_modules_len,
                                  CLUSTERLIST_FILENAME_SIGNATURE,
                                  _clusterlist_module_cb,
                                  CLUSTERLIST_MODULE_INFO_SYM,
                                  clusterlist_handle,
                                  1)) < 0)
    goto cleanup;
  
  if (rv)
    goto out;

  clusterlist_handle->dl_handle = NULL;
  clusterlist_handle->module_info = &default_clusterlist_module_info;
 out:
  return clusterlist_handle;

 cleanup:
  if (clusterlist_handle)
    {
      if (clusterlist_handle->dl_handle)
        lt_dlclose(clusterlist_handle->dl_handle);
      free(clusterlist_handle);
    }
  module_cleanup();
  return NULL;
}

/* 
 * clusterlist_module_handle_check
 *
 * Check for proper clusterlist module handle
 *
 * Returns 0 on success, -1 on error
 */
static int
clusterlist_module_handle_check(clusterlist_module_t clusterlist_handle)
{
  if (!clusterlist_handle 
      || clusterlist_handle->magic != CLUSTERLIST_MODULE_MAGIC_NUMBER
      || !clusterlist_handle->module_info)
    {
      CEREBRO_DBG(("invalid clusterlist_handle"));
      return -1;
    }

  return 0;
}

int
clusterlist_module_unload(clusterlist_module_t clusterlist_handle)
{
  if (clusterlist_module_handle_check(clusterlist_handle) < 0)
    return -1;

  clusterlist_module_cleanup(clusterlist_handle);

  clusterlist_handle->magic = ~CLUSTERLIST_MODULE_MAGIC_NUMBER;
  if (clusterlist_handle->dl_handle)
    lt_dlclose(clusterlist_handle->dl_handle);
  clusterlist_handle->module_info = NULL;
  free(clusterlist_handle);

  module_cleanup();
  return 0;
}

char *
clusterlist_module_name(clusterlist_module_t clusterlist_handle)
{
  if (clusterlist_module_handle_check(clusterlist_handle) < 0)
    return NULL;

  return (clusterlist_handle->module_info)->clusterlist_module_name;
}

int
clusterlist_module_setup(clusterlist_module_t clusterlist_handle)
{
  if (clusterlist_module_handle_check(clusterlist_handle) < 0)
    return -1;
  
  return ((*(clusterlist_handle->module_info)->setup)());
}

int
clusterlist_module_cleanup(clusterlist_module_t clusterlist_handle)
{
  if (clusterlist_module_handle_check(clusterlist_handle) < 0)
    return -1;
  
  return ((*(clusterlist_handle->module_info)->cleanup)());
}

int
clusterlist_module_numnodes(clusterlist_module_t clusterlist_handle)
{
  if (clusterlist_module_handle_check(clusterlist_handle) < 0)
    return -1;
  
  return ((*(clusterlist_handle->module_info)->numnodes)());
}

int
clusterlist_module_get_all_nodes(clusterlist_module_t clusterlist_handle, 
				 char ***nodes)
{
  if (clusterlist_module_handle_check(clusterlist_handle) < 0)
    return -1;
  
  return ((*(clusterlist_handle->module_info)->get_all_nodes)(nodes));
}

int
clusterlist_module_node_in_cluster(clusterlist_module_t clusterlist_handle, 
				   const char *node)
{
  if (clusterlist_module_handle_check(clusterlist_handle) < 0)
    return -1;
  
  return ((*(clusterlist_handle->module_info)->node_in_cluster)(node));
}

int
clusterlist_module_get_nodename(clusterlist_module_t clusterlist_handle,
				const char *node, 
				char *buf, 
				unsigned int buflen)
{
  if (clusterlist_module_handle_check(clusterlist_handle) < 0)
    return -1;
  
  return ((*(clusterlist_handle->module_info)->get_nodename)(node, buf, buflen));
}
