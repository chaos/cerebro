/*****************************************************************************\
 *  $Id: cerebro_module.c,v 1.34 2005-06-09 20:17:09 achu Exp $
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
#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <dirent.h>

#include "cerebro.h"
#include "cerebro_module.h"
#include "cerebro/cerebro_clusterlist_module.h"
#include "cerebro/cerebro_config_module.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_error.h"

#if !WITH_STATIC_MODULES
#include "ltdl.h"
#endif /* !WITH_STATIC_MODULES */

/*  
 * cerebro_module_library_setup_count
 *
 * indicates the number of times module setup function has been called
 */
static int cerebro_module_library_setup_count = 0;

#if WITH_STATIC_MODULES

#if WITH_GENDERSLLNL
extern struct cerebro_config_module_info gendersllnl_config_module_info;
extern struct cerebro_clusterlist_module_info gendersllnl_clusterlist_module_info;
#endif /* WITH_GENDERSLLNL */

#if WITH_GENDERS
extern struct cerebro_clusterlist_module_info genders_clusterlist_module_info;
#endif /* WITH_GENDERS */

#if WITH_HOSTSFILE
extern struct cerebro_clusterlist_module_info hostsfile_clusterlist_module_info;
#endif /* WITH_HOSTSFILE */

/*
 * static_config_modules
 * static_config_modules_names
 *
 * configuration modules statically compiled in
 */
struct cerebro_config_module_info *static_config_modules[] =
  {
#if WITH_GENDERSLLNL
    &gendersllnl_config_module_info,
#endif /* WITH_GENDERSLLNL */
    NULL
  };

/*
 * static_clusterlist_modules
 *
 * clusterlist modules statically compiled in
 */
struct cerebro_clusterlist_module_info *static_clusterlist_modules[] =
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
 * dynamic_clusterlist_modules
 * dynamic_clusterlist_modules_len
 *
 * dynamic clusterlist modules to search for by default
 */
char *dynamic_clusterlist_modules[] = {
  "cerebro_clusterlist_gendersllnl.la",
  "cerebro_clusterlist_genders.la",
  "cerebro_clusterlist_hostsfile.la",
  NULL
};
int dynamic_clusterlist_modules_len = 3;

/*
 * dynamic_config_modules
 * dynamic_config_modules_len
 *
 * dynamic configuration modules to search for by default
 */
char *dynamic_config_modules[] = {
  "cerebro_config_gendersllnl.la",
  NULL
};
int dynamic_config_modules_len = 1;

#define CEREBRO_CLUSTERLIST_FILENAME_SIGNATURE "cerebro_clusterlist_"
#define CEREBRO_CONFIG_FILENAME_SIGNATURE      "cerebro_config_"

#endif /* !WITH_STATIC_MODULES */

#define CEREBRO_CLUSTERLIST_MODULE_MAGIC_NUMBER 0x33882299
#define CEREBRO_CONFIG_MODULE_MAGIC_NUMBER      0x33882200

/* 
 * struct cerebro_clusterlist_module
 *
 * clusterlist module handle
 */
struct cerebro_clusterlist_module
{
  int32_t magic;
#if !WITH_STATIC_MODULES
  lt_dlhandle dl_handle;
#endif /* !WITH_STATIC_MODULES */
  struct cerebro_clusterlist_module_info *module_info;
};

/* 
 * struct cerebro_config_module
 *
 * config module handle
 */
struct cerebro_config_module
{
  int32_t magic;
#if !WITH_STATIC_MODULES
  lt_dlhandle dl_handle;
#endif /* !WITH_STATIC_MODULES */
  struct cerebro_config_module_info *module_info;
};

extern struct cerebro_clusterlist_module_info default_clusterlist_module_info;
extern struct cerebro_config_module_info default_config_module_info;

#if !WITH_STATIC_MODULES 
/*
 * Cerebro_load_module
 *
 * function prototype for loading a module. Passed a module handle and
 * file/module to load.
 *
 * Returns 1 on loading success, 0 on loading failure, -1 on fatal error
 */
typedef int (*Cerebro_load_module)(void *, char *);

/* 
 * _cerebro_module_find_known_module
 *
 * Try to find a known module from the modules list in the search
 * directory.
 *
 * - search_dir - directory to search
 * - modules_list - list of modules to search for
 * - modules_list_len - length of list
 * - load_module - function to call when a module is found
 * - handle - pointer to module handle
 *
 * Returns 1 if module is loaded, 0 if it isn't, -1 on fatal error
 */
int
_cerebro_module_find_known_module(char *search_dir,
				  char **modules_list,
				  int modules_list_len,
				  Cerebro_load_module load_module,
                                  void *handle)
{
  DIR *dir;
  int i = 0, found = 0;

  if (!search_dir)
    {
      cerebro_err_debug_lib("%s(%s:%d): search_dir null", 
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!modules_list)
    {
      cerebro_err_debug_lib("%s(%s:%d): modules_list null", 
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!(modules_list_len > 0))
    {
      cerebro_err_debug_lib("%s(%s:%d): modules_list_len not valid",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  if (!load_module)
    {
      cerebro_err_debug_lib("%s(%s:%d): load_module null", 
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!handle)
    {
      cerebro_err_debug_lib("%s(%s:%d): handle null", 
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!(dir = opendir(search_dir)))
    return 0;

  for (i = 0; i < modules_list_len; i++)
    {
      struct dirent *dirent;

      while ((dirent = readdir(dir)))
        {
          if (!strcmp(dirent->d_name, modules_list[i]))
            {
              char filebuf[CEREBRO_MAXPATHLEN+1];
              int flag;

              memset(filebuf, '\0', CEREBRO_MAXPATHLEN+1);
              snprintf(filebuf, CEREBRO_MAXPATHLEN, "%s/%s",
                       search_dir, modules_list[i]);

              if ((flag = load_module(handle, filebuf)) < 0)
		return -1;

              if (flag)
                {
                  found++;
                  goto out;
                }
            }
        }
      rewinddir(dir);
    }

 out:
  closedir(dir);
  return (found) ? 1 : 0;
}

/*
 * _cerebro_module_find_unknown_module
 *
 * Search a directory for a module currently unknown.
 *
 * - search_dir - directory to search
 * - signature - filename signature indicating if the filename is a
 *               module we want to try and load
 * - load_module - function to call when a module is found
 * - handle - pointer to module handle
 *
 * Returns 1 when a module is found, 0 when one is not, -1 on fatal error
 */
int 
_cerebro_module_find_unknown_module(char *search_dir,
				    char *signature,
				    Cerebro_load_module load_module,
                                    void *handle)
{
  DIR *dir;
  struct dirent *dirent;
  int found = 0;

  if (!search_dir)
    {
      cerebro_err_debug_lib("%s(%s:%d): search_dir null", 
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!signature)
    {
      cerebro_err_debug_lib("%s(%s:%d): signature null", 
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
 
  if (!load_module)
    {
      cerebro_err_debug_lib("%s(%s:%d): load_module null", 
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!handle)
    {
      cerebro_err_debug_lib("%s(%s:%d): handle null", 
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!(dir = opendir(search_dir)))
    return 0;

  while ((dirent = readdir(dir)))
    {
      char *ptr = strstr(dirent->d_name, signature);

      if (ptr && ptr == &dirent->d_name[0])
        {
          char filebuf[CEREBRO_MAXPATHLEN+1];
          int flag;

          /*
           * Don't bother trying to load this file unless its a shared
           * object file or libtool file.
           */
          ptr = strchr(dirent->d_name, '.');
          if (!ptr || !(!strcmp(ptr, ".la") || !strcmp(ptr, ".so")))
            continue;

          memset(filebuf, '\0', CEREBRO_MAXPATHLEN+1);
          snprintf(filebuf, CEREBRO_MAXPATHLEN, "%s/%s",
                   search_dir, dirent->d_name);

          if ((flag = load_module(handle, filebuf)) < 0)
	    return -1;

          if (flag)
            {
              found++;
              goto out;
            }
        }
    }

 out:
  closedir(dir);
  return (found) ? 1 : 0;
}
#endif /* !WITH_STATIC_MODULES */

static int 
_cerebro_module_setup(void)
{
  if (cerebro_module_library_setup_count)
    goto out;

#if !WITH_STATIC_MODULES
  if (lt_dlinit() != 0)
    {
      cerebro_err_debug_lib("%s(%s:%d): lt_dlinit: %s", 
			    __FILE__, __FUNCTION__, __LINE__, 
			    lt_dlerror());
      return -1;
    }
#endif /* !WITH_STATIC_MODULES */

 out:
  cerebro_module_library_setup_count++;
  return 0;
}

static int 
_cerebro_module_cleanup(void)
{
  if (cerebro_module_library_setup_count)
    cerebro_module_library_setup_count--;

  if (!cerebro_module_library_setup_count)
    {
#if !WITH_STATIC_MODULES
      if (lt_dlexit() != 0)
        {
          cerebro_err_debug_lib("%s(%s:%d): lt_dlexit: %s", 
                                __FILE__, __FUNCTION__, __LINE__, 
                                lt_dlerror());
          return -1;
        }
#endif /* !WITH_STATIC_MODULES */
    }

  return 0;
}

/* 
 * _load_clusterlist_module
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
_load_clusterlist_module(void *handle, char *module)
{
#if WITH_STATIC_MODULES
  struct cerebro_clusterlist_module_info **ptr;
  int i = 0;
#else  /* !WITH_STATIC_MODULES */
  lt_dlhandle dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */
  struct cerebro_clusterlist_module_info *module_info = NULL;
  cerebro_clusterlist_module_t clusterlist_handle = (cerebro_clusterlist_module_t)handle;

  if (!cerebro_module_library_setup_count)
    {
      cerebro_err_debug_lib("%s(%s:%d): cerebro_module_library uninitialized", 
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!clusterlist_handle)
    {
      cerebro_err_debug_lib("%s(%s:%d): clusterlist_handle null", 
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (clusterlist_handle->magic != CEREBRO_CLUSTERLIST_MODULE_MAGIC_NUMBER)
    {
      cerebro_err_debug_lib("%s(%s:%d): clusterlist_handle magic number invalid", 
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!module)
    {
      cerebro_err_debug_lib("%s(%s:%d): module null", 
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

#if WITH_STATIC_MODULES
  ptr = &static_clusterlist_modules[0];
  while (ptr[i])
    {      
      if (!ptr[i]->clusterlist_module_name)
	{
	  cerebro_err_debug_lib("%s(%s:%d): clusterlist_module_name null: %d",
				__FILE__, __FUNCTION__, __LINE__, i);
	  continue;
	}
      if (!strcmp(ptr[i]->clusterlist_module_name, module))
	{
	  module_info = ptr[i];
	  break;
	}
      i++;
    }

  if (!module_info)
    goto cleanup;

#else  /* !WITH_STATIC_MODULES */
  if (!(dl_handle = lt_dlopen(module)))
    {
      cerebro_err_debug_lib("%s(%s:%d): lt_dlopen: module=%s, %s",
			    __FILE__, __FUNCTION__, __LINE__,
			    module, lt_dlerror());
      goto cleanup;
    }
  
  /* clear lt_dlerror */
  lt_dlerror();

  if (!(module_info = lt_dlsym(dl_handle, "clusterlist_module_info")))
    {
      const char *err = lt_dlerror();
      if (err)
	cerebro_err_debug_lib("%s(%s:%d): lt_dlsym: module=%s, %s",
			      __FILE__, __FUNCTION__, __LINE__,
			      module, err);
      goto cleanup;
    }
#endif /* !WITH_STATIC_MODULES */

  if (!module_info->clusterlist_module_name)
    {
      cerebro_err_debug_lib("clusterlist module '%s': name null",
			    module_info->clusterlist_module_name);
      goto cleanup;
    }

  if (!module_info->setup)
    {
      cerebro_err_debug_lib("clusterlist module '%s': setup null",
			    module_info->clusterlist_module_name);
      goto cleanup;
    }
  
  if (!module_info->cleanup)
    {
      cerebro_err_debug_lib("clusterlist module '%s': cleanup null",
			    module_info->clusterlist_module_name);
      goto cleanup;
    }
  
  if (!module_info->numnodes)
    {
      cerebro_err_debug_lib("clusterlist module '%s': numnodes null",
			    module_info->clusterlist_module_name);
      goto cleanup;
    }
  
  if (!module_info->get_all_nodes)
    {
      cerebro_err_debug_lib("clusterlist module '%s': get_all_nodes null",
			    module_info->clusterlist_module_name);
      goto cleanup;
    }

  if (!module_info->node_in_cluster)
    {
      cerebro_err_debug_lib("clusterlist module '%s': node_in_cluster null",
			    module_info->clusterlist_module_name);
      goto cleanup;
    }

  if (!module_info->get_nodename)
    {
      cerebro_err_debug_lib("clusterlist module '%s': get_nodename null",
			    module_info->clusterlist_module_name);
      goto cleanup;
    }

#if !WITH_STATIC_MODULES
  clusterlist_handle->dl_handle = dl_handle;
#endif /* !WITH_STATIC_MODULES */
  clusterlist_handle->module_info = module_info;
  return 1;

 cleanup:
#if !WITH_STATIC_MODULES
  if (dl_handle)
    lt_dlclose(dl_handle);
#endif /* !WITH_STATIC_MODULES */
  return 0;
}

cerebro_clusterlist_module_t 
_cerebro_module_load_clusterlist_module(void)
{
#if WITH_STATIC_MODULES
  struct cerebro_clusterlist_module_info **ptr;
  int i = 0;
#endif /* WITH_STATIC_MODULES */
  struct cerebro_clusterlist_module *handle = NULL;
  int rv;
  
  if (_cerebro_module_setup() < 0)
    return NULL;

  if (!(handle = (struct cerebro_clusterlist_module *)malloc(sizeof(struct cerebro_clusterlist_module))))
    return NULL;
  memset(handle, '\0', sizeof(struct cerebro_clusterlist_module));
  handle->magic = CEREBRO_CLUSTERLIST_MODULE_MAGIC_NUMBER;
      
#if WITH_STATIC_MODULES
  ptr = &static_clusterlist_modules[0];
  while (ptr[i])
    {
      char *name = ptr[i]->clusterlist_module_name;

      if (!name)
	{
	  cerebro_err_debug_lib("%s(%s:%d): clusterlist_module_name null: %d",
				__FILE__, __FUNCTION__, __LINE__, i);
	  continue;
	}

      if ((rv = _load_clusterlist_module(handle, name)) < 0)
        goto cleanup;

      if (rv)
	goto out;
      i++;
    }
#else  /* !WITH_STATIC_MODULES */
  if ((rv = _cerebro_module_find_known_module(CEREBRO_CLUSTERLIST_MODULE_BUILDDIR,
					      dynamic_clusterlist_modules,
					      dynamic_clusterlist_modules_len,
					      _load_clusterlist_module,
                                              handle)) < 0)
    goto cleanup;

  if (rv)
    goto out;

  if ((rv = _cerebro_module_find_known_module(CEREBRO_MODULE_DIR,
					      dynamic_clusterlist_modules,
					      dynamic_clusterlist_modules_len,
					      _load_clusterlist_module,
                                              handle)) < 0)
    goto cleanup;

  if (rv)
    goto out;

  
  if ((rv = _cerebro_module_find_unknown_module(CEREBRO_MODULE_DIR,
						CEREBRO_CLUSTERLIST_FILENAME_SIGNATURE,
						_load_clusterlist_module,
                                                handle)) < 0)
    goto cleanup;
  
  if (rv)
    goto out;
#endif /* !WITH_STATIC_MODULES */

#if !WITH_STATIC_MODULES
  handle->dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */
  handle->module_info = &default_clusterlist_module_info;
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
  _cerebro_module_cleanup();
  return NULL;
}

/* 
 * _cerebro_module_clusterlist_module_check
 *
 * Check for proper clusterlist module handle
 *
 * Returns 0 on success, -1 on error
 */
static int
_cerebro_module_clusterlist_module_check(cerebro_clusterlist_module_t clusterlist_handle)
{
  if (!clusterlist_handle 
      || clusterlist_handle->magic != CEREBRO_CLUSTERLIST_MODULE_MAGIC_NUMBER
      || !clusterlist_handle->module_info)
    {
      cerebro_err_debug_lib("%s(%s:%d): cerebro handle invalid", 
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  return 0;
}

int
_cerebro_module_destroy_clusterlist_handle(cerebro_clusterlist_module_t clusterlist_handle)
{
  if (_cerebro_module_clusterlist_module_check(clusterlist_handle) < 0)
    return -1;

  clusterlist_handle->magic = ~CEREBRO_CLUSTERLIST_MODULE_MAGIC_NUMBER;
#if !WITH_STATIC_MODULES
  if (clusterlist_handle->dl_handle)
    lt_dlclose(clusterlist_handle->dl_handle);
#endif /* !WITH_STATIC_MODULES */
  clusterlist_handle->module_info = NULL;

  _cerebro_module_cleanup();
  return 0;
}

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
#if WITH_STATIC_MODULES
  struct cerebro_config_module_info **ptr;
  int i = 0;
#else  /* !WITH_STATIC_MODULES */
  lt_dlhandle dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */
  struct cerebro_config_module_info *module_info = NULL;
  cerebro_config_module_t config_handle = (cerebro_config_module_t)handle;

  if (!cerebro_module_library_setup_count)
    {
      cerebro_err_debug_lib("%s(%s:%d): cerebro_module_library uninitialized", 
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!config_handle)
    {
      cerebro_err_debug_lib("%s(%s:%d): config_handle null",
                            __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
                                                                                      
  if (config_handle->magic != CEREBRO_CONFIG_MODULE_MAGIC_NUMBER)
    {
      cerebro_err_debug_lib("%s(%s:%d): config_handle magic number invalid",
                            __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!module)
    {
      cerebro_err_debug_lib("%s(%s:%d): module null", 
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
#if WITH_STATIC_MODULES
  ptr = &static_config_modules[0];
  while (ptr[i])
    {      
      if (!ptr[i]->config_module_name)
	{
	  cerebro_err_debug_lib("%s(%s:%d): config_module_name null",
				__FILE__, __FUNCTION__, __LINE__);
	  continue;
	}

      if (!strcmp(ptr[i]->config_module_name, module))
	{
	  module_info = ptr[i];
	  break;
	}
      i++;
    }

  if (!module_info)
    goto cleanup;
#else  /* !WITH_STATIC_MODULES */
  if (!(dl_handle = lt_dlopen(module)))
    {
      cerebro_err_debug_lib("%s(%s:%d): lt_dlopen: module=%s, %s",
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
	cerebro_err_debug_lib("%s(%s:%d): lt_dlsym: module=%s, %s",
			      __FILE__, __FUNCTION__, __LINE__,
			      module, err);
      goto cleanup;
    }
#endif /* !WITH_STATIC_MODULES */

  if (!module_info->config_module_name)
    {
      cerebro_err_debug_lib("config module '%s': config_module_name null",
			    module_info->config_module_name);
      goto cleanup;
    }

  if (!module_info->setup)
    {
      cerebro_err_debug_lib("config module '%s': setup null",
			    module_info->config_module_name);
      goto cleanup;
    }

  if (!module_info->cleanup)
    {
      cerebro_err_debug_lib("config module '%s': cleanup null",
			    module_info->config_module_name);
      goto cleanup;
    }

  if (!module_info->load_default)
    {
      cerebro_err_debug_lib("config module '%s': load_default null",
			    module_info->config_module_name);
      goto cleanup;
    }

#if !WITH_STATIC_MODULES
  config_handle->dl_handle = dl_handle;
#endif /* !WITH_STATIC_MODULES */
  config_handle->module_info = module_info;
  return 1;

 cleanup:
#if !WITH_STATIC_MODULES
  if (dl_handle)
    lt_dlclose(dl_handle);
#endif /* !WITH_STATIC_MODULES */
  return 0;
}

cerebro_config_module_t
_cerebro_module_load_config_module(void)
{
#if WITH_STATIC_MODULES
  struct cerebro_config_module_info **ptr;
  int i = 0;
#endif /* WITH_STATIC_MODULES */
  struct cerebro_config_module *handle = NULL;
  int rv;

  if (_cerebro_module_setup() < 0)
    return NULL;
                                                                                      
  if (!(handle = (struct cerebro_config_module *)malloc(sizeof(struct cerebro_config_module))))
    return NULL;
  memset(handle, '\0', sizeof(struct cerebro_config_module));
  handle->magic = CEREBRO_CONFIG_MODULE_MAGIC_NUMBER;

#if WITH_STATIC_MODULES
  ptr = &static_config_modules[0];
  while (ptr[i])
    {
      char *name = ptr[i]->config_module_name;

      if (!name)
	{
	  cerebro_err_debug_lib("%s(%s:%d): config_module_name null",
				__FILE__, __FUNCTION__, __LINE__);
	  continue;
	}

      if ((rv = _load_config_module(name)) < 0)
	  return -1;

      if (rv)
	goto out;
      i++;
    }
#else  /* !WITH_STATIC_MODULES */
  if ((rv = _cerebro_module_find_known_module(CEREBRO_CONFIG_MODULE_BUILDDIR,
					      dynamic_config_modules,
					      dynamic_config_modules_len,
					      _load_config_module,
                                              handle)) < 0)
    goto cleanup;

  if (rv)
    goto out;

  if ((rv = _cerebro_module_find_known_module(CEREBRO_MODULE_DIR,
					      dynamic_config_modules,
					      dynamic_config_modules_len,
					      _load_config_module,
                                              handle)) < 0)
    goto cleanup;

  if (rv)
    goto out;

  if ((rv = _cerebro_module_find_unknown_module(CEREBRO_MODULE_DIR,
						CEREBRO_CONFIG_FILENAME_SIGNATURE,
						_load_config_module,
                                                handle)) < 0)
    goto cleanup;

  if (rv)
    goto out;
#endif /* !WITH_STATIC_MODULES */

#if !WITH_STATIC_MODULES
  handle->dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */
  handle->module_info = &default_config_module_info;
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
      cerebro_err_debug_lib("%s(%s:%d): cerebro config_handle invalid",
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
#if !WITH_STATIC_MODULES
  if (config_handle->dl_handle)
    lt_dlclose(config_handle->dl_handle);
#endif /* !WITH_STATIC_MODULES */
  config_handle->module_info = NULL;

  return 0;
}

char *
_cerebro_clusterlist_module_name(cerebro_clusterlist_module_t clusterlist_handle)
{
  if (_cerebro_module_clusterlist_module_check(clusterlist_handle) < 0)
    return NULL;

  return (clusterlist_handle->module_info)->clusterlist_module_name;
}

int
_cerebro_clusterlist_module_setup(cerebro_clusterlist_module_t clusterlist_handle)
{
  if (_cerebro_module_clusterlist_module_check(clusterlist_handle) < 0)
    return -1;
  
  return ((*(clusterlist_handle->module_info)->setup)());
}

int
_cerebro_clusterlist_module_cleanup(cerebro_clusterlist_module_t clusterlist_handle)
{
  if (_cerebro_module_clusterlist_module_check(clusterlist_handle) < 0)
    return -1;
  
  return ((*(clusterlist_handle->module_info)->cleanup)());
}

int
_cerebro_clusterlist_module_numnodes(cerebro_clusterlist_module_t clusterlist_handle)
{
  if (_cerebro_module_clusterlist_module_check(clusterlist_handle) < 0)
    return -1;
  
  return ((*(clusterlist_handle->module_info)->numnodes)());
}

int
_cerebro_clusterlist_module_get_all_nodes(cerebro_clusterlist_module_t clusterlist_handle, 
                                          char ***nodes)
{
  if (_cerebro_module_clusterlist_module_check(clusterlist_handle) < 0)
    return -1;
  
  return ((*(clusterlist_handle->module_info)->get_all_nodes)(nodes));
}

int
_cerebro_clusterlist_module_node_in_cluster(cerebro_clusterlist_module_t clusterlist_handle, 
                                            const char *node)
{
  if (_cerebro_module_clusterlist_module_check(clusterlist_handle) < 0)
    return -1;
  
  return ((*(clusterlist_handle->module_info)->node_in_cluster)(node));
}

int
_cerebro_clusterlist_module_get_nodename(cerebro_clusterlist_module_t clusterlist_handle,
                                         const char *node, 
                                         char *buf, 
                                         unsigned int buflen)
{
  if (_cerebro_module_clusterlist_module_check(clusterlist_handle) < 0)
    return -1;
  
  return ((*(clusterlist_handle->module_info)->get_nodename)(node, buf, buflen));
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
