/*****************************************************************************\
 *  $Id: metric_module.c,v 1.9 2005-06-27 23:27:06 achu Exp $
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
#include "cerebro/cerebro_metric_module.h"

#include "metric_module.h"
#include "module_util.h"

#include "debug.h"
#include "ltdl.h"

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
  unsigned int modules_max;
  unsigned int modules_count;
  lt_dlhandle *dl_handle;
  struct cerebro_metric_module_info **module_info;
};

/*
 * _metric_module_cb
 *
 * Check and store module
 *
 * Return 1 is module is stored, 0 if not, -1 on fatal error
 */
static int 
_metric_module_cb(void *handle, void *dl_handle, void *module_info)
{
  metric_modules_t metric_handle;
  struct cerebro_metric_module_info *metric_module_info;
  lt_dlhandle metric_dl_handle;
  
  if (!handle || !dl_handle || !module_info)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }
  
  metric_handle = handle;
  metric_module_info = module_info;
  metric_dl_handle = dl_handle;
  
  if (metric_handle->magic != METRIC_MODULE_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("invalid handle"));
      return -1;
    }
  
  if (!metric_module_info->metric_module_name
      || !metric_module_info->setup
      || !metric_module_info->cleanup
      || !metric_module_info->get_metric_name
      || !metric_module_info->get_metric_period
      || !metric_module_info->get_metric_value
      || !metric_module_info->destroy_metric_value
      || !metric_module_info->get_metric_thread)
    {
      CEREBRO_DBG(("invalid module info"));
      return 0;
    }

  if (metric_handle->modules_count < metric_handle->modules_max)
    {
      metric_handle->dl_handle[metric_handle->modules_count] = metric_dl_handle;
      metric_handle->module_info[metric_handle->modules_count] = metric_module_info;
      metric_handle->modules_count++;
      return 1;
    }
  else
    return 0;
}

metric_modules_t 
metric_modules_load(unsigned int modules_max)
{
  struct metric_module *metric_handle = NULL;
  int rv;

  if (!modules_max)
    {
      CEREBRO_DBG(("modules_max invalid"));
      return NULL;
    }

  if (module_setup() < 0)
    return NULL;
                                                                                    
  if (!(metric_handle = (struct metric_module *)malloc(sizeof(struct metric_module))))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      return NULL;
    }
  memset(metric_handle, '\0', sizeof(struct metric_module));
  metric_handle->magic = METRIC_MODULE_MAGIC_NUMBER;
  metric_handle->modules_max = modules_max;
  metric_handle->modules_count = 0;

  if (!(metric_handle->dl_handle = (lt_dlhandle *)malloc(sizeof(lt_dlhandle)*metric_handle->modules_max)))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      goto cleanup;
    }
  memset(metric_handle->dl_handle, '\0', sizeof(lt_dlhandle)*metric_handle->modules_max);
  
  if (!(metric_handle->module_info = (struct cerebro_metric_module_info **)malloc(sizeof(struct cerebro_metric_module_info *)*metric_handle->modules_max)))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      goto cleanup;
    }
  memset(metric_handle->module_info, '\0', sizeof(struct cerebro_metric_module_info *)*metric_handle->modules_max);

  if ((rv = find_and_load_modules(METRIC_MODULE_DIR,
                                  NULL,
                                  0,
                                  METRIC_FILENAME_SIGNATURE,
                                  _metric_module_cb,
                                  METRIC_MODULE_INFO_SYM,
                                  metric_handle,
                                  metric_handle->modules_max)) < 0)
    goto cleanup;

  if (rv)
    goto out;

  /* Responsibility of caller to call count to see no modules were loaded */

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
  module_cleanup();
  return NULL;
}

/*
 * metric_module_handle_check
 *
 * Check for proper metric module handle
 *
 * Returns 0 on success, -1 on error
 */
static int
metric_module_handle_check(metric_modules_t metric_handle)
{
  if (!metric_handle || metric_handle->magic != METRIC_MODULE_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("invalid metric_handle"));
      return -1;
    }

  return 0;
}

/*
 * metric_module_handle_and_index_check
 *
 * Check for proper monitor module handle and index
 *
 * Returns 0 on success, -1 on error
 */
static int
metric_module_handle_and_index_check(metric_modules_t metric_handle,
                                     unsigned int index)
{
  if (metric_module_handle_check(metric_handle) < 0)
    return -1;
  
  if (!(index < metric_handle->modules_count))
    {
      CEREBRO_DBG(("invalid index"));
      return -1;
    }
  
  return 0;
}

int 
metric_modules_unload(metric_modules_t metric_handle)
{
  int i;

  if (metric_module_handle_check(metric_handle) < 0)
    return -1;

  for (i = 0; i < metric_handle->modules_count; i++)
    metric_module_cleanup(metric_handle, i);

  if (metric_handle->dl_handle)
    {
      for (i = 0; i < metric_handle->modules_count; i++)
        lt_dlclose(metric_handle->dl_handle[i]);
      free(metric_handle->dl_handle);
      metric_handle->dl_handle = NULL;
    }
  if (metric_handle->module_info)
    {
      free(metric_handle->module_info);
      metric_handle->module_info = NULL;
    }

  metric_handle->magic = ~METRIC_MODULE_MAGIC_NUMBER;
  metric_handle->modules_max = 0;
  metric_handle->modules_count = 0;

  free(metric_handle);

  module_cleanup();
  return 0;
  
}

int 
metric_modules_count(metric_modules_t metric_handle)
{
  if (metric_module_handle_check(metric_handle) < 0)
    return -1;

  return metric_handle->modules_count;
}

char *
metric_module_name(metric_modules_t metric_handle,
		   unsigned int index)
{
  if (metric_module_handle_and_index_check(metric_handle, index) < 0)
    return NULL;

  return (metric_handle->module_info[index])->metric_module_name;
}

int 
metric_module_setup(metric_modules_t metric_handle,
		    unsigned int index)
{
  if (metric_module_handle_and_index_check(metric_handle, index) < 0)
    return -1;
  
  return ((*(metric_handle->module_info[index])->setup)());
}

int 
metric_module_cleanup(metric_modules_t metric_handle,
		      unsigned int index)
{
  if (metric_module_handle_and_index_check(metric_handle, index) < 0)
    return -1;
  
  return ((*(metric_handle->module_info[index])->cleanup)());
}

char *
metric_module_get_metric_name(metric_modules_t metric_handle,
			      unsigned int index)
{
  if (metric_module_handle_and_index_check(metric_handle, index) < 0)
    return NULL;
  
  return ((*(metric_handle->module_info[index])->get_metric_name)());
}

int
metric_module_get_metric_period(metric_modules_t metric_handle,
                                unsigned int index)
{
  if (metric_module_handle_and_index_check(metric_handle, index) < 0)
    return -1;
  
  return ((*(metric_handle->module_info[index])->get_metric_period)());
}

int 
metric_module_get_metric_value(metric_modules_t metric_handle,
			       unsigned int index,
			       unsigned int *metric_value_type,
			       unsigned int *metric_value_len,
			       void **metric_value)
{
  if (metric_module_handle_and_index_check(metric_handle, index) < 0)
    return -1;
  
  return ((*(metric_handle->module_info[index])->get_metric_value)(metric_value_type,
                                                                   metric_value_len,
                                                                   metric_value));
}

int 
metric_module_destroy_metric_value(metric_modules_t metric_handle,
				   unsigned int index,
				   void *metric_value)
{
  if (metric_module_handle_and_index_check(metric_handle, index) < 0)
    return -1;
  
  return ((*(metric_handle->module_info[index])->destroy_metric_value)(metric_value));
}

Cerebro_metric_thread_pointer 
metric_module_get_metric_thread(metric_modules_t metric_handle,
                                unsigned int index)
{
  if (metric_module_handle_and_index_check(metric_handle, index) < 0)
    return NULL;
  
  return ((*(metric_handle->module_info[index])->get_metric_thread)());
}

