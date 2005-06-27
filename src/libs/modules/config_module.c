/*****************************************************************************\
 *  $Id: config_module.c,v 1.8 2005-06-27 23:27:06 achu Exp $
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
  "cerebro_config_gendersllnl.so",
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
      || !config_module_info->load_default)
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
  struct config_module *config_handle = NULL;
  int rv;

  if (module_setup() < 0)
    return NULL;

  if (!(config_handle = (struct config_module *)malloc(sizeof(struct config_module))))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      return NULL;
    }
  memset(config_handle, '\0', sizeof(struct config_module));
  config_handle->magic = CONFIG_MODULE_MAGIC_NUMBER;

  if ((rv = find_and_load_modules(CONFIG_MODULE_DIR,
                                  config_modules,
                                  config_modules_len,
                                  CONFIG_FILENAME_SIGNATURE,
                                  _config_module_cb,
                                  CONFIG_MODULE_INFO_SYM,
                                  config_handle,
                                  1)) < 0)

  if (rv)
    goto out;

  config_handle->dl_handle = NULL;
  config_handle->module_info = &default_config_module_info;
 out:
  return config_handle;

 cleanup:
  if (config_handle)
    {
      if (config_handle->dl_handle)
        lt_dlclose(config_handle->dl_handle);
      free(config_handle);
    }
  module_cleanup();
  return NULL;
}

/*
 * config_module_handle_check
 *
 * Check for proper config module handle
 *
 * Returns 0 on success, -1 on error
 */
static int
config_module_handle_check(config_module_t config_handle)
{
  if (!config_handle 
      || config_handle->magic != CONFIG_MODULE_MAGIC_NUMBER
      || !config_handle->module_info)
    {
      CEREBRO_DBG(("invalid config_handle"));
      return -1;
    }

  return 0;
}

int
config_module_unload(config_module_t config_handle)
{
  if (config_module_handle_check(config_handle) < 0)
    return -1;
  
  config_module_cleanup(config_handle);

  config_handle->magic = ~CONFIG_MODULE_MAGIC_NUMBER;
  if (config_handle->dl_handle)
    lt_dlclose(config_handle->dl_handle);
  config_handle->module_info = NULL;
  free(config_handle);

  module_cleanup();
  return 0;
}

char *
config_module_name(config_module_t config_handle)
{
  if (config_module_handle_check(config_handle) < 0)
    return NULL;

  return (config_handle->module_info)->config_module_name;
}

int
config_module_setup(config_module_t config_handle)
{
  if (config_module_handle_check(config_handle) < 0)
    return -1;
  
  return ((*(config_handle->module_info)->setup)());
}

int
config_module_cleanup(config_module_t config_handle)
{
  if (config_module_handle_check(config_handle) < 0)
    return -1;
  
  return ((*(config_handle->module_info)->cleanup)());
}

int
config_module_load_default(config_module_t config_handle,
			   struct cerebro_config *conf)
{
  if (config_module_handle_check(config_handle) < 0)
    return -1;
  
  return ((*(config_handle->module_info)->load_default)(conf));
}
