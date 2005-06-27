/*****************************************************************************\
 *  $Id: monitor_module.c,v 1.7 2005-06-27 20:23:38 achu Exp $
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
#include "ltdl.h"

#define MONITOR_FILENAME_SIGNATURE  "cerebro_monitor_"

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
  lt_dlhandle *dl_handle;
  struct cerebro_monitor_module_info **module_info;
};

extern int module_setup_count;

/* 
 * _monitor_module_loader
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
_monitor_module_loader(void *handle, char *module)
{
  lt_dlhandle dl_handle = NULL;
  struct cerebro_monitor_module_info *module_info = NULL;
  monitor_modules_t monitor_handle = (monitor_modules_t)handle;

  if (!module_setup_count)
    {
      CEREBRO_DBG(("cerebro_module_library uninitialized"));
      return -1;
    }

  if (!monitor_handle)
    {
      CEREBRO_DBG(("monitor_handle null"));
      return -1;
    }

  if (monitor_handle->magic != MONITOR_MODULE_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("monitor_handle magic number invalid"));
      return -1;
    }

  if (!module)
    {
      CEREBRO_DBG(("module null"));
      return -1;
    }
  
  if (!(dl_handle = lt_dlopen(module)))
    {
      CEREBRO_DBG(("lt_dlopen: module=%s, %s", module, lt_dlerror()));
      goto cleanup;
    }

  /* clear lt_dlerror */
  lt_dlerror();

  if (!(module_info = lt_dlsym(dl_handle, "monitor_module_info")))
    {
      const char *err = lt_dlerror();
      if (err)
	CEREBRO_DBG(("lt_dlsym: module=%s, %s", module, err));
      goto cleanup;
    }

  if (!module_info->monitor_module_name
      || !module_info->setup
      || !module_info->cleanup
      || !module_info->metric_name
      || !module_info->metric_update)
    {
      CEREBRO_DBG(("invalid module info"));
      goto cleanup;
    }

  if (monitor_handle->modules_count < monitor_handle->modules_max)
    {
      monitor_handle->dl_handle[monitor_handle->modules_count] = dl_handle;
      monitor_handle->module_info[monitor_handle->modules_count] = module_info;
      monitor_handle->modules_count++;
    }
  return 1;

 cleanup:
  if (dl_handle)
    lt_dlclose(dl_handle);
  return 0;
}

monitor_modules_t 
monitor_modules_load(unsigned int modules_max)
{
  struct monitor_module *monitor_handle = NULL;
  int rv;

  if (!modules_max)
    {
      CEREBRO_DBG(("modules_max invalid"));
      return NULL;
    }

  if (module_setup() < 0)
    return NULL;
                                                                                    
  if (!(monitor_handle = (struct monitor_module *)malloc(sizeof(struct monitor_module))))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      return NULL;
    }
  memset(monitor_handle, '\0', sizeof(struct monitor_module));
  monitor_handle->magic = MONITOR_MODULE_MAGIC_NUMBER;
  monitor_handle->modules_max = modules_max;
  monitor_handle->modules_count = 0;
  if (!(monitor_handle->dl_handle = (lt_dlhandle *)malloc(sizeof(lt_dlhandle)*monitor_handle->modules_max)))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      goto cleanup;
    }
  memset(monitor_handle->dl_handle, '\0', sizeof(lt_dlhandle)*monitor_handle->modules_max);
  
  if (!(monitor_handle->module_info = (struct cerebro_monitor_module_info * *)malloc(sizeof(struct cerebro_monitor_module_info *)*monitor_handle->modules_max)))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      goto cleanup;
    }
  memset(monitor_handle->module_info, '\0', sizeof(struct cerebro_monitor_module_info *)*monitor_handle->modules_max);

#if CEREBRO_DEBUG
  if ((rv = find_modules(MONITOR_MODULE_DIR,
			 MONITOR_FILENAME_SIGNATURE,
			 _monitor_module_loader,
			 monitor_handle,
			 monitor_handle->modules_max)) < 0)
    goto cleanup;

  if (rv)
    goto out;
#endif /* CEREBRO_DEBUG */

  if ((rv = find_modules(CEREBRO_MODULE_DIR,
			 MONITOR_FILENAME_SIGNATURE,
			 _monitor_module_loader,
			 monitor_handle,
			 monitor_handle->modules_max)) < 0)
    goto cleanup;

  if (rv)
    goto out;

 out:
  return monitor_handle;

 cleanup:
  if (monitor_handle)
    {
      if (monitor_handle->dl_handle)
        {
          int i;
          for (i = 0; i < monitor_handle->modules_count; i++)
            lt_dlclose(monitor_handle->dl_handle[i]);
          free(monitor_handle->dl_handle);
        }
      if (monitor_handle->module_info)
        free(monitor_handle->module_info);
      free(monitor_handle);
    }
  module_cleanup();
  return NULL;
}

/*
 * monitor_module_handle_check
 *
 * Check for proper monitor module handle
 *
 * Returns 0 on success, -1 on error
 */
static int
monitor_module_handle_check(monitor_modules_t monitor_handle)
{
  if (!monitor_handle || monitor_handle->magic != MONITOR_MODULE_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("invalid monitor_handle"));
      return -1;
    }

  return 0;
}

int 
monitor_modules_unload(monitor_modules_t monitor_handle)
{
  int i;

  if (monitor_module_handle_check(monitor_handle) < 0)
    return -1;

  for (i = 0; i < monitor_handle->modules_count; i++)
    monitor_module_cleanup(monitor_handle, i);

  if (monitor_handle->dl_handle)
    {
      for (i = 0; i < monitor_handle->modules_count; i++)
        lt_dlclose(monitor_handle->dl_handle[i]);
      free(monitor_handle->dl_handle);
      monitor_handle->dl_handle = NULL;
    }
  if (monitor_handle->module_info)
    {
      free(monitor_handle->module_info);
      monitor_handle->module_info = NULL;
    }

  monitor_handle->magic = ~MONITOR_MODULE_MAGIC_NUMBER;
  monitor_handle->modules_max = 0;
  monitor_handle->modules_count = 0;

  free(monitor_handle);

  module_cleanup();
  return 0;
  
}

int 
monitor_modules_count(monitor_modules_t monitor_handle)
{
  if (monitor_module_handle_check(monitor_handle) < 0)
    return -1;

  return monitor_handle->modules_count;
}

char *
monitor_module_name(monitor_modules_t monitor_handle,
		    unsigned int index)
{
  if (monitor_module_handle_check(monitor_handle) < 0)
    return NULL;

  if (!(index < monitor_handle->modules_count))
    return NULL;

  return (monitor_handle->module_info[index])->monitor_module_name;
}

int 
monitor_module_setup(monitor_modules_t monitor_handle,
		     unsigned int index)
{
  if (monitor_module_handle_check(monitor_handle) < 0)
    return -1;
  
  if (!(index < monitor_handle->modules_count))
    return -1;

  return ((*(monitor_handle->module_info[index])->setup)());
}

int 
monitor_module_cleanup(monitor_modules_t monitor_handle,
		       unsigned int index)
{
  if (monitor_module_handle_check(monitor_handle) < 0)
    return -1;
  
  if (!(index < monitor_handle->modules_count))
    return -1;

  return ((*(monitor_handle->module_info[index])->cleanup)());
}

char *
monitor_module_metric_name(monitor_modules_t monitor_handle,
			   unsigned int index)
{
  if (monitor_module_handle_check(monitor_handle) < 0)
    return NULL;
  
  if (!(index < monitor_handle->modules_count))
    return NULL;

  return ((*(monitor_handle->module_info[index])->metric_name)());
}

int 
monitor_module_metric_update(monitor_modules_t monitor_handle,
			     unsigned int index,
			     const char *nodename,
			     unsigned int metric_value_type,
			     unsigned int metric_value_len,
			     void *metric_value)
{
  if (monitor_module_handle_check(monitor_handle) < 0)
    return -1;
  
  if (!(index < monitor_handle->modules_count))
    return -1;

  return ((*(monitor_handle->module_info[index])->metric_update)(nodename,
                                                                 metric_value_type,
                                                                 metric_value_len,
                                                                 metric_value));
}
