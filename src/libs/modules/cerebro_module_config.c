/*****************************************************************************\
 *  $Id: cerebro_module_config.c,v 1.1 2005-06-18 06:47:06 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */

#include "cerebro.h"
#include "cerebro_module_config.h"
#include "cerebro_module_util.h"
#include "cerebro/cerebro_config_module.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_error.h"

#include "ltdl.h"

/*
 * dynamic_config_modules
 * dynamic_config_modules_len
 *
 * dynamic configuration modules to search for by default
 */
char *dynamic_config_modules[] = {
  "cerebro_config_gendersllnl.so",
  NULL
};
int dynamic_config_modules_len = 1;

#define CEREBRO_CONFIG_FILENAME_SIGNATURE      "cerebro_config_"

#define CEREBRO_CONFIG_MODULE_DIR      CEREBRO_CONFIG_MODULE_BUILDDIR "/.libs"

#define CEREBRO_CONFIG_MODULE_MAGIC_NUMBER      0x33882211

/* 
 * struct cerebro_config_module
 *
 * config module handle
 */
struct cerebro_config_module
{
  int32_t magic;
  lt_dlhandle dl_handle;
  struct cerebro_config_module_info *module_info;
};

extern struct cerebro_config_module_info default_config_module_info;
extern int cerebro_module_library_setup_count;

/* 
 * _load_config_module
 *
 * If compiled statically, attempt to load the module specified by the
 * module name.
 *
 * If compiled dynamically, attempt to load the module specified by
 * the module_path.
 *
 * Return 1 is module is loaded, 0 if not, -1 on fatal error
 */
static int 
_load_config_module(void *handle, char *module)
{
  lt_dlhandle dl_handle = NULL;
  struct cerebro_config_module_info *module_info = NULL;
  cerebro_config_module_t config_handle = (cerebro_config_module_t)handle;

  if (!cerebro_module_library_setup_count)
    {
      cerebro_err_debug("%s(%s:%d): cerebro_module_library uninitialized", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!config_handle)
    {
      cerebro_err_debug("%s(%s:%d): config_handle null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
                                                                                      
  if (config_handle->magic != CEREBRO_CONFIG_MODULE_MAGIC_NUMBER)
    {
      cerebro_err_debug("%s(%s:%d): config_handle magic number invalid",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!module)
    {
      cerebro_err_debug("%s(%s:%d): module null", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  if (!(dl_handle = lt_dlopen(module)))
    {
      cerebro_err_debug("%s(%s:%d): lt_dlopen: module=%s, %s",
			__FILE__, __FUNCTION__, __LINE__,
			module, lt_dlerror());
      goto cleanup;
    }

  /* clear lt_dlerror */
  lt_dlerror();

  if (!(module_info = lt_dlsym(dl_handle, "config_module_info")))
    {
      const char *err = lt_dlerror();
      if (err)
	cerebro_err_debug("%s(%s:%d): lt_dlsym: module=%s, %s",
			  __FILE__, __FUNCTION__, __LINE__,
			  module, err);
      goto cleanup;
    }

  if (!module_info->config_module_name)
    {
      cerebro_err_debug("config module '%s': config_module_name null",
			module_info->config_module_name);
      goto cleanup;
    }

  if (!module_info->setup)
    {
      cerebro_err_debug("config module '%s': setup null",
			module_info->config_module_name);
      goto cleanup;
    }

  if (!module_info->cleanup)
    {
      cerebro_err_debug("config module '%s': cleanup null",
			module_info->config_module_name);
      goto cleanup;
    }

  if (!module_info->load_default)
    {
      cerebro_err_debug("config module '%s': load_default null",
			module_info->config_module_name);
      goto cleanup;
    }

  config_handle->dl_handle = dl_handle;
  config_handle->module_info = module_info;
  return 1;

 cleanup:
  if (dl_handle)
    lt_dlclose(dl_handle);
  return 0;
}

cerebro_config_module_t
_cerebro_module_load_config_module(void)
{
  struct cerebro_config_module *config_handle = NULL;
  int rv;

  if (_cerebro_module_setup() < 0)
    return NULL;
                                                                                      
  if (!(config_handle = (struct cerebro_config_module *)malloc(sizeof(struct cerebro_config_module))))
    return NULL;
  memset(config_handle, '\0', sizeof(struct cerebro_config_module));
  config_handle->magic = CEREBRO_CONFIG_MODULE_MAGIC_NUMBER;

#if CEREBRO_DEBUG
  if ((rv = _cerebro_module_find_known_module(CEREBRO_CONFIG_MODULE_DIR,
					      dynamic_config_modules,
					      dynamic_config_modules_len,
					      _load_config_module,
                                              config_handle)) < 0)
    goto cleanup;

  if (rv)
    goto out;
#endif /* CEREBRO_DEBUG */

  if ((rv = _cerebro_module_find_known_module(CEREBRO_MODULE_DIR,
					      dynamic_config_modules,
					      dynamic_config_modules_len,
					      _load_config_module,
                                              config_handle)) < 0)
    goto cleanup;

  if (rv)
    goto out;

  if ((rv = _cerebro_module_find_modules(CEREBRO_MODULE_DIR,
                                         CEREBRO_CONFIG_FILENAME_SIGNATURE,
                                         _load_config_module,
                                         config_handle,
                                         1)) < 0)
    goto cleanup;

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
  _cerebro_module_cleanup();
  return NULL;
}

/*
 * _cerebro_module_config_module_check
 *
 * Check for proper config module handle
 *
 * Returns 0 on success, -1 on error
 */
static int
_cerebro_module_config_module_check(cerebro_config_module_t config_handle)
{
  if (!config_handle 
      || config_handle->magic != CEREBRO_CONFIG_MODULE_MAGIC_NUMBER
      || !config_handle->module_info)
    {
      cerebro_err_debug("%s(%s:%d): cerebro config_handle invalid",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  return 0;
}

int
_cerebro_module_destroy_config_handle(cerebro_config_module_t config_handle)
{
  if (_cerebro_module_config_module_check(config_handle) < 0)
    return -1;
  
  config_handle->magic = ~CEREBRO_CONFIG_MODULE_MAGIC_NUMBER;
  if (config_handle->dl_handle)
    lt_dlclose(config_handle->dl_handle);
  config_handle->module_info = NULL;
  free(config_handle);

  _cerebro_module_cleanup();
  return 0;
}

char *
_cerebro_config_module_name(cerebro_config_module_t config_handle)
{
  if (_cerebro_module_config_module_check(config_handle) < 0)
    return NULL;

  return (config_handle->module_info)->config_module_name;
}

int
_cerebro_config_module_setup(cerebro_config_module_t config_handle)
{
  if (_cerebro_module_config_module_check(config_handle) < 0)
    return -1;
  
  return ((*(config_handle->module_info)->setup)());
}

int
_cerebro_config_module_cleanup(cerebro_config_module_t config_handle)
{
  if (_cerebro_module_config_module_check(config_handle) < 0)
    return -1;
  
  return ((*(config_handle->module_info)->cleanup)());
}

int
_cerebro_config_module_load_default(cerebro_config_module_t config_handle,
                                    struct cerebro_config *conf)
{
  if (_cerebro_module_config_module_check(config_handle) < 0)
    return -1;
  
  return ((*(config_handle->module_info)->load_default)(conf));
}
