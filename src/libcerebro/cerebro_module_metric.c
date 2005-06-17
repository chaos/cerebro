/*****************************************************************************\
 *  $Id: cerebro_module_metric.c,v 1.1 2005-06-17 17:45:39 achu Exp $
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
#include "cerebro_module_metric.h"
#include "cerebro_module_util.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_error.h"
#include "cerebro/cerebro_metric_module.h"

#include "ltdl.h"

#define CEREBRO_METRIC_FILENAME_SIGNATURE      "cerebro_metric_"

#define CEREBRO_METRIC_MODULE_DIR      CEREBRO_METRIC_MODULE_BUILDDIR "/.libs"

#define CEREBRO_METRIC_MODULE_MAGIC_NUMBER      0x33882222

/* 
 * struct cerebro_metric_module
 *
 * metric module handle
 */
struct cerebro_metric_module
{
  int32_t magic;
  unsigned int modules_max;
  unsigned int modules_count;
  lt_dlhandle *dl_handle;
  struct cerebro_metric_module_info **module_info;
};

extern int cerebro_module_library_setup_count;

/* 
 * _load_metric_module
 *
 * If compiled statically, attempt to load the module specified by the
 * module name.
 *
 * If compiled dynamically, attempt to load the module specified by
 * the module_path.
 *
 * Return 1 if module is loaded, 0 if not, -1 on fatal error
 */
static int 
_load_metric_module(void *handle, char *module)
{
  lt_dlhandle dl_handle = NULL;
  struct cerebro_metric_module_info *module_info = NULL;
  cerebro_metric_modules_t metric_handle = (cerebro_metric_modules_t)handle;

  if (!cerebro_module_library_setup_count)
    {
      cerebro_err_debug("%s(%s:%d): cerebro_module_library uninitialized", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!metric_handle)
    {
      cerebro_err_debug("%s(%s:%d): metric_handle null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (metric_handle->magic != CEREBRO_METRIC_MODULE_MAGIC_NUMBER)
    {
      cerebro_err_debug("%s(%s:%d): metric_handle magic number invalid",
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

  if (!(module_info = lt_dlsym(dl_handle, "metric_module_info")))
    {
      const char *err = lt_dlerror();
      if (err)
	cerebro_err_debug("%s(%s:%d): lt_dlsym: module=%s, %s",
			  __FILE__, __FUNCTION__, __LINE__,
			  module, err);
      goto cleanup;
    }

  if (!module_info->metric_module_name)
    {
      cerebro_err_debug("metric module '%s': metric_module_name null",
			module_info->metric_module_name);
      goto cleanup;
    }

  if (!module_info->setup)
    {
      cerebro_err_debug("metric module '%s': setup null",
			module_info->metric_module_name);
      goto cleanup;
    }

  if (!module_info->cleanup)
    {
      cerebro_err_debug("metric module '%s': cleanup null",
			module_info->metric_module_name);
      goto cleanup;
    }

  if (!module_info->get_metric_name)
    {
      cerebro_err_debug("metric module '%s': get_metric_name null",
			module_info->metric_module_name);
      goto cleanup;
    }

  if (!module_info->get_metric_value)
    {
      cerebro_err_debug("metric module '%s': get_metric_value null",
			module_info->metric_module_name);
      goto cleanup;
    }

  if (!module_info->destroy_metric_value)
    {
      cerebro_err_debug("metric module '%s': destroy_metric_value null",
			module_info->metric_module_name);
      goto cleanup;
    }

  if (metric_handle->modules_count < metric_handle->modules_max)
    {
      metric_handle->dl_handle[metric_handle->modules_count] = dl_handle;
      metric_handle->module_info[metric_handle->modules_count] = module_info;
      metric_handle->modules_count++;
    }
  return 1;

 cleanup:
  if (dl_handle)
    lt_dlclose(dl_handle);
  return 0;
}

cerebro_metric_modules_t 
_cerebro_module_load_metric_modules(unsigned int modules_max)
{
  struct cerebro_metric_module *metric_handle = NULL;
  int rv;
                                                                                      
  if (!modules_max)
    {
      cerebro_err_debug("%s(%s:%d): modules_max invalid",
			__FILE__, __FUNCTION__, __LINE__);
      return NULL;
    }

  if (_cerebro_module_setup() < 0)
    return NULL;
                                                                                    
  if (!(metric_handle = (struct cerebro_metric_module *)malloc(sizeof(struct cerebro_metric_module))))
    return NULL;
  memset(metric_handle, '\0', sizeof(struct cerebro_metric_module));
  metric_handle->magic = CEREBRO_METRIC_MODULE_MAGIC_NUMBER;
  metric_handle->modules_max = modules_max;
  metric_handle->modules_count = 0;
  if (!(metric_handle->dl_handle = (lt_dlhandle *)malloc(sizeof(lt_dlhandle)*metric_handle->modules_max)))
    {
      cerebro_err_debug("%s(%s:%d): out of memory",
			__FILE__, __FUNCTION__, __LINE__);
      goto cleanup;
    }
  memset(metric_handle->dl_handle, '\0', sizeof(lt_dlhandle)*metric_handle->modules_max);
  
  if (!(metric_handle->module_info = (struct cerebro_metric_module_info * *)malloc(sizeof(struct cerebro_metric_module_info *)*metric_handle->modules_max)))
    {
      cerebro_err_debug("%s(%s:%d): out of memory",
			__FILE__, __FUNCTION__, __LINE__);
      goto cleanup;
    }
  memset(metric_handle->module_info, '\0', sizeof(struct cerebro_metric_module_info *)*metric_handle->modules_max);

#if CEREBRO_DEBUG
  if ((rv = _cerebro_module_find_modules(CEREBRO_METRIC_MODULE_DIR,
                                         CEREBRO_METRIC_FILENAME_SIGNATURE,
                                         _load_metric_module,
                                         metric_handle,
                                         metric_handle->modules_max)) < 0)
    goto cleanup;

  if (rv)
    goto out;
#endif /* CEREBRO_DEBUG */

  if ((rv = _cerebro_module_find_modules(CEREBRO_MODULE_DIR,
                                         CEREBRO_METRIC_FILENAME_SIGNATURE,
                                         _load_metric_module,
                                         metric_handle,
                                         metric_handle->modules_max)) < 0)
    goto cleanup;
                                                                                      
  if (rv)
    goto out;

 out:
  return metric_handle;

 cleanup:
  if (metric_handle)
    {
      if (metric_handle->dl_handle)
        {
          int i;
          for (i = 0; i < metric_handle->modules_count; i++)
            lt_dlclose(metric_handle->dl_handle[i]);
          free(metric_handle->dl_handle);
        }
      if (metric_handle->module_info)
        free(metric_handle->module_info);
      free(metric_handle);
    }
  _cerebro_module_cleanup();
  return NULL;
}
                                                                                      
/*
 * _cerebro_module_metric_module_check
 *
 * Check for proper metric module handle
 *
 * Returns 0 on success, -1 on error
 */
static int
_cerebro_module_metric_module_check(cerebro_metric_modules_t metric_handle)
{
  if (!metric_handle 
      || metric_handle->magic != CEREBRO_METRIC_MODULE_MAGIC_NUMBER)
    {
      cerebro_err_debug("%s(%s:%d): cerebro metric_handle invalid",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  return 0;
}

int 
_cerebro_module_destroy_metric_handle(cerebro_metric_modules_t metric_handle)
{
  if (_cerebro_module_metric_module_check(metric_handle) < 0)
    return -1;

  metric_handle->magic = ~CEREBRO_METRIC_MODULE_MAGIC_NUMBER;
  metric_handle->modules_max = 0;
  metric_handle->modules_count = 0;
  if (metric_handle->dl_handle)
    {
      int i;
      for (i = 0; i < metric_handle->modules_count; i++)
        lt_dlclose(metric_handle->dl_handle[i]);
      free(metric_handle->dl_handle);
    }
  if (metric_handle->module_info)
    free(metric_handle->module_info);
  free(metric_handle);

  _cerebro_module_cleanup();
  return 0;
  
}

int 
_cerebro_metric_module_count(cerebro_metric_modules_t metric_handle)
{
  if (_cerebro_module_metric_module_check(metric_handle) < 0)
    return -1;

  return metric_handle->modules_count;
}

char *
_cerebro_metric_module_name(cerebro_metric_modules_t metric_handle,
                            unsigned int index)
{
  if (_cerebro_module_metric_module_check(metric_handle) < 0)
    return NULL;

  if (!(index < metric_handle->modules_count))
    return NULL;

  return (metric_handle->module_info[index])->metric_module_name;
}

int 
_cerebro_metric_module_setup(cerebro_metric_modules_t metric_handle,
                             unsigned int index)
{
  if (_cerebro_module_metric_module_check(metric_handle) < 0)
    return -1;
  
  if (!(index < metric_handle->modules_count))
    return -1;

  return ((*(metric_handle->module_info[index])->setup)());
}

int 
_cerebro_metric_module_cleanup(cerebro_metric_modules_t metric_handle,
                               unsigned int index)
{
  if (_cerebro_module_metric_module_check(metric_handle) < 0)
    return -1;
  
  if (!(index < metric_handle->modules_count))
    return -1;

  return ((*(metric_handle->module_info[index])->cleanup)());
}

char *
_cerebro_metric_module_get_metric_name(cerebro_metric_modules_t metric_handle,
                                       unsigned int index)
{
  if (_cerebro_module_metric_module_check(metric_handle) < 0)
    return NULL;
  
  if (!(index < metric_handle->modules_count))
    return NULL;

  return ((*(metric_handle->module_info[index])->get_metric_name)());
}

int 
_cerebro_metric_module_get_metric_value(cerebro_metric_modules_t metric_handle,
                                        unsigned int index,
                                        unsigned int *metric_value_type,
                                        unsigned int *metric_value_len,
                                        void **metric_value)
{
  if (_cerebro_module_metric_module_check(metric_handle) < 0)
    return -1;
  
  if (!(index < metric_handle->modules_count))
    return -1;
  
  return ((*(metric_handle->module_info[index])->get_metric_value)(metric_value_type,
                                                                   metric_value_len,
                                                                   metric_value));
}

int 
_cerebro_metric_module_destroy_metric_value(cerebro_metric_modules_t metric_handle,
                                            unsigned int index,
                                            void *metric_value)
{
  if (_cerebro_module_metric_module_check(metric_handle) < 0)
    return -1;
  
  if (!(index < metric_handle->modules_count))
    return -1;
  
  return ((*(metric_handle->module_info[index])->destroy_metric_value)(metric_value));
}
