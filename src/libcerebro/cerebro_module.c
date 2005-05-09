/*****************************************************************************\
 *  $Id: cerebro_module.c,v 1.27 2005-05-09 16:02:11 achu Exp $
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
 * cerebro_module_library_is_setup
 *
 * indicates if the module library has been initialized
 */
static int cerebro_module_library_is_setup = 0;

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
 * clusterlist_module_dl_handle
 *
 * clusterlist module dynamically loaded module handle
 */
static lt_dlhandle clusterlist_module_dl_handle = NULL;

/*
 * config_module_dl_handle
 *
 * config module dynamically loaded module handle
 */
static lt_dlhandle config_module_dl_handle = NULL;

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

/*
 * clusterlist_module_info
 *
 * clusterlist module info and operations
 */
static struct cerebro_clusterlist_module_info *clusterlist_module_info = NULL;

/* 
 * clusterlist_module_found
 *
 * Flag indicates if a clusterlist module was found, or if the default
 * is used
 */
static int clusterlist_module_found = 0;

extern struct cerebro_clusterlist_module_info default_clusterlist_module_info;

/*
 * config_module_info
 *
 * config module info and operations
 */
static struct cerebro_config_module_info *config_module_info = NULL;

/* 
 * config_module_found
 *
 * Flag indicates if a config module was found, or if the default
 * is used
 */
static int config_module_found = 0;

extern struct cerebro_config_module_info default_config_module_info;

#if !WITH_STATIC_MODULES 
/*
 * Cerebro_load_module
 *
 * function prototype for loading a module. Passed a module
 * file to load.
 *
 * Returns 1 on loading success, 0 on loading failure, -1 on fatal error
 */
typedef int (*Cerebro_load_module)(char *);

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
 *
 * Returns 1 if module is loaded, 0 if it isn't, -1 on fatal error
 */
int
_cerebro_module_find_known_module(char *search_dir,
				  char **modules_list,
				  int modules_list_len,
				  Cerebro_load_module load_module)
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

              if ((flag = load_module(filebuf)) < 0)
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
 *
 * Returns 1 when a module is found, 0 when one is not, -1 on fatal error
 */
int 
_cerebro_module_find_unknown_module(char *search_dir,
				    char *signature,
				    Cerebro_load_module load_module)
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

          if ((flag = load_module(filebuf)) < 0)
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

int 
cerebro_module_setup(void)
{
#if !WITH_STATIC_MODULES
  if (lt_dlinit() != 0)
    {
      cerebro_err_debug_lib("%s(%s:%d): lt_dlinit: %s", 
			    __FILE__, __FUNCTION__, __LINE__, 
			    lt_dlerror());
      return -1;
    }
#endif /* !WITH_STATIC_MODULES */

  cerebro_module_library_is_setup = 1;
  return 0;
}

int 
cerebro_module_is_setup(void)
{
  if (cerebro_module_library_is_setup)
    return 1;
  return 0;
}

int 
cerebro_module_cleanup(void)
{
  cerebro_module_unload_clusterlist_module();
  cerebro_module_unload_config_module();

#if !WITH_STATIC_MODULES
  if (lt_dlexit() != 0)
    {
      cerebro_err_debug_lib("%s(%s:%d): lt_dlexit: %s", 
			    __FILE__, __FUNCTION__, __LINE__, 
			    lt_dlerror());
      return -1;
    }
#endif /* !WITH_STATIC_MODULES */

  cerebro_module_library_is_setup = 0;
  return 0;
}

/* 
 * _cerebro_module_load_clusterlist_module
 *
 * If compiled statically, attempt to load the module specified by the
 * module name.
 *
 * If compiled dynamically, attempt to load the module specifiec by
 * the module_path.
 *
 * Return 1 is module is loaded, 0 if not, -1 on fatal error
 */
static int
_cerebro_module_load_clusterlist_module(char *module)
{
#if WITH_STATIC_MODULES
  struct cerebro_clusterlist_module_info **ptr;
  int i = 0;
#else  /* !WITH_STATIC_MODULES */
  lt_dlhandle clusterlist_module_dl_handle_l = NULL;
#endif /* !WITH_STATIC_MODULES */
  struct cerebro_clusterlist_module_info *clusterlist_module_info_l = NULL;

  if (!cerebro_module_library_is_setup)
    {
      cerebro_err_debug_lib("%s(%s:%d): cerebro_module_library uninitialized", 
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!module)
    {
      cerebro_err_debug_lib("%s(%s:%d): module null", 
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (clusterlist_module_info)
    {
      cerebro_err_debug_lib("%s(%s:%d): clusterlist module already loaded", 
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
	  clusterlist_module_info_l = ptr[i];
	  break;
	}
      i++;
    }

  if (!clusterlist_module_info_l)
    goto cleanup;
#else  /* !WITH_STATIC_MODULES */
  if (!(clusterlist_module_dl_handle_l = lt_dlopen(module)))
    {
      cerebro_err_debug_lib("%s(%s:%d): lt_dlopen: module=%s, %s",
			    __FILE__, __FUNCTION__, __LINE__,
			    module, lt_dlerror());
      goto cleanup;
    }

  /* clear lt_dlerror */
  lt_dlerror();

  if (!(clusterlist_module_info_l = lt_dlsym(clusterlist_module_dl_handle_l, 
					     "clusterlist_module_info")))
    {
      const char *err = lt_dlerror();
      if (err)
	cerebro_err_debug_lib("%s(%s:%d): lt_dlsym: module=%s, %s",
			      __FILE__, __FUNCTION__, __LINE__,
			      module, err);
      goto cleanup;
    }
#endif /* !WITH_STATIC_MODULES */

  if (!clusterlist_module_info_l->clusterlist_module_name)
    {
      cerebro_err_debug_lib("clusterlist module '%s': name null",
			    clusterlist_module_info_l->clusterlist_module_name);
      goto cleanup;
    }

  if (!clusterlist_module_info_l->setup)
    {
      cerebro_err_debug_lib("clusterlist module '%s': setup null",
			    clusterlist_module_info_l->clusterlist_module_name);
      goto cleanup;
    }

  if (!clusterlist_module_info_l->cleanup)
    {
      cerebro_err_debug_lib("clusterlist module '%s': cleanup null",
			    clusterlist_module_info_l->clusterlist_module_name);
      goto cleanup;
    }

  if (!clusterlist_module_info_l->get_all_nodes)
    {
      cerebro_err_debug_lib("clusterlist module '%s': get_all_nodes null",
			    clusterlist_module_info_l->clusterlist_module_name);
      goto cleanup;
    }

  if (!clusterlist_module_info_l->numnodes)
    {
      cerebro_err_debug_lib("clusterlist module '%s': numnodes null",
			    clusterlist_module_info_l->clusterlist_module_name);
      goto cleanup;
    }

  if (!clusterlist_module_info_l->node_in_cluster)
    {
      cerebro_err_debug_lib("clusterlist module '%s': node_in_cluster null",
			    clusterlist_module_info_l->clusterlist_module_name);
      goto cleanup;
    }

  if (!clusterlist_module_info_l->get_nodename)
    {
      cerebro_err_debug_lib("clusterlist module '%s': get_nodename null",
			    clusterlist_module_info_l->clusterlist_module_name);
      goto cleanup;
    }

#if !WITH_STATIC_MODULES
  clusterlist_module_dl_handle = clusterlist_module_dl_handle_l;
#endif /* !WITH_STATIC_MODULES */
  clusterlist_module_info = clusterlist_module_info_l;
  return 1;

 cleanup:
#if !WITH_STATIC_MODULES
  if (clusterlist_module_dl_handle)
    lt_dlclose(clusterlist_module_dl_handle);
#endif /* !WITH_STATIC_MODULES */
  return 0;
}

int 
cerebro_module_load_clusterlist_module(void)
{
#if WITH_STATIC_MODULES
  struct cerebro_clusterlist_module_info **ptr;
  int i = 0;
#endif /* WITH_STATIC_MODULES */
  int rv;

  if (!cerebro_module_library_is_setup)
    {
      cerebro_err_debug_lib("%s(%s:%d): cerebro_module_library uninitialized", 
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

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

      if ((rv = _cerebro_module_load_clusterlist_module(name)) < 0)
	  return -1;

      if (rv)
	goto found;
      i++;
    }
#else  /* !WITH_STATIC_MODULES */
  if ((rv = _cerebro_module_find_known_module(CEREBRO_CLUSTERLIST_MODULE_BUILDDIR,
					      dynamic_clusterlist_modules,
					      dynamic_clusterlist_modules_len,
					      _cerebro_module_load_clusterlist_module)) < 0)
      return -1;

  if (rv)
    goto found;

  if ((rv = _cerebro_module_find_known_module(CEREBRO_MODULE_DIR,
					      dynamic_clusterlist_modules,
					      dynamic_clusterlist_modules_len,
					      _cerebro_module_load_clusterlist_module)) < 0)
    return -1;

  if (rv)
    goto found;

  
  if ((rv = _cerebro_module_find_unknown_module(CEREBRO_MODULE_DIR,
						CEREBRO_CLUSTERLIST_FILENAME_SIGNATURE,
						_cerebro_module_load_clusterlist_module)) < 0)
    return -1;
  
  if (rv)
    goto found;
#endif /* !WITH_STATIC_MODULES */

  clusterlist_module_info = &default_clusterlist_module_info;
  return 0;

 found:
  clusterlist_module_found++;
  return 1;
}

int
cerebro_module_unload_clusterlist_module(void)
{
  if (!cerebro_module_library_is_setup)
    {
      cerebro_err_debug_lib("%s(%s:%d): cerebro_module_library uninitialized", 
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
#if !WITH_STATIC_MODULES
  if (clusterlist_module_dl_handle)
    lt_dlclose(clusterlist_module_dl_handle);
  clusterlist_module_dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */
  clusterlist_module_info = NULL;
  clusterlist_module_found = 0;
  return 0;
}

/* 
 * _cerebro_module_load_config_module
 *
 * If compiled statically, attempt to load the module specified by the
 * module name.
 *
 * If compiled dynamically, attempt to load the module specifiec by
 * the module_path.
 *
 * Return 1 is module is loaded, 0 if not, -1 on fatal error
 */
static int 
_cerebro_module_load_config_module(char *module)
{
#if WITH_STATIC_MODULES
  struct cerebro_config_module_info **ptr;
  int i = 0;
#else  /* !WITH_STATIC_MODULES */
  lt_dlhandle config_module_dl_handle_l = NULL;
#endif /* !WITH_STATIC_MODULES */
  struct cerebro_config_module_info *config_module_info_l = NULL;

  if (!cerebro_module_library_is_setup)
    {
      cerebro_err_debug_lib("%s(%s:%d): cerebro_module_library uninitialized", 
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!module)
    {
      cerebro_err_debug_lib("%s(%s:%d): module null", 
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (config_module_info)
    {
      cerebro_err_debug_lib("%s(%s:%d): config module already loaded", 
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
	  config_module_info_l = ptr[i];
	  break;
	}
      i++;
    }

  if (!config_module_info_l)
    goto cleanup;
#else  /* !WITH_STATIC_MODULES */
  if (!(config_module_dl_handle_l = lt_dlopen(module)))
    {
      cerebro_err_debug_lib("%s(%s:%d): lt_dlopen: module=%s, %s",
			    __FILE__, __FUNCTION__, __LINE__,
			    module, lt_dlerror());
      goto cleanup;
    }

  /* clear lt_dlerror */
  lt_dlerror();

  if (!(config_module_info_l = lt_dlsym(config_module_dl_handle_l, 
					"config_module_info")))
    {
      const char *err = lt_dlerror();
      if (err)
	cerebro_err_debug_lib("%s(%s:%d): lt_dlsym: module=%s, %s",
			      __FILE__, __FUNCTION__, __LINE__,
			      module, err);
      goto cleanup;
    }
#endif /* !WITH_STATIC_MODULES */

  if (!config_module_info_l->config_module_name)
    {
      cerebro_err_debug_lib("config module '%s': config_module_name null",
			    config_module_info_l->config_module_name);
      goto cleanup;
    }

  if (!config_module_info_l->setup)
    {
      cerebro_err_debug_lib("config module '%s': setup null",
			    config_module_info_l->config_module_name);
      goto cleanup;
    }

  if (!config_module_info_l->cleanup)
    {
      cerebro_err_debug_lib("config module '%s': cleanup null",
			    config_module_info_l->config_module_name);
      goto cleanup;
    }

  if (!config_module_info_l->load_default)
    {
      cerebro_err_debug_lib("config module '%s': load_default null",
			    config_module_info_l->config_module_name);
      goto cleanup;
    }

#if !WITH_STATIC_MODULES
  config_module_dl_handle = config_module_dl_handle_l;
#endif /* !WITH_STATIC_MODULES */
  config_module_info = config_module_info_l;
  return 1;

 cleanup:
#if !WITH_STATIC_MODULES
  if (config_module_dl_handle)
    lt_dlclose(config_module_dl_handle);
#endif /* !WITH_STATIC_MODULES */
  return 0;
}

int 
cerebro_module_load_config_module(void)
{
#if WITH_STATIC_MODULES
  struct cerebro_config_module_info **ptr;
  int i = 0;
#endif /* WITH_STATIC_MODULES */
  int rv;

  if (!cerebro_module_library_is_setup)
    {
      cerebro_err_debug_lib("%s(%s:%d): cerebro_module_library uninitialized", 
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

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

      if ((rv = _cerebro_module_load_config_module(name)) < 0)
	  return -1;

      if (rv)
	goto found;
      i++;
    }
#else  /* !WITH_STATIC_MODULES */
  if ((rv = _cerebro_module_find_known_module(CEREBRO_CONFIG_MODULE_BUILDDIR,
					      dynamic_config_modules,
					      dynamic_config_modules_len,
					      _cerebro_module_load_config_module)) < 0)
    return -1;

  if (rv)
    goto found;

  if ((rv = _cerebro_module_find_known_module(CEREBRO_MODULE_DIR,
					      dynamic_config_modules,
					      dynamic_config_modules_len,
					      _cerebro_module_load_config_module)) < 0)
    return -1;

  if (rv)
    goto found;

  if ((rv = _cerebro_module_find_unknown_module(CEREBRO_MODULE_DIR,
						CEREBRO_CONFIG_FILENAME_SIGNATURE,
						_cerebro_module_load_config_module)) < 0)
    return -1;

  if (rv)
    goto found;
#endif /* !WITH_STATIC_MODULES */

  config_module_info = &default_config_module_info;
  return 0;

 found:
  config_module_found++;
  return 1;
}

int
cerebro_module_unload_config_module(void)
{
  if (!cerebro_module_library_is_setup)
    {
      cerebro_err_debug_lib("%s(%s:%d): cerebro_module_library uninitialized", 
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

#if !WITH_STATIC_MODULES
  if (config_module_dl_handle)
    lt_dlclose(config_module_dl_handle);
  config_module_dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */
  config_module_info = NULL;
  config_module_found = 0;
  return 0;
}

int 
cerebro_module_clusterlist_module_is_loaded(void)
{
  if (clusterlist_module_info)
    return 1;
  return 0;
}

int 
cerebro_module_config_module_is_loaded(void)
{
  if (config_module_info)
    return 1;
  return 0;
}

int 
cerebro_module_clusterlist_module_found(void)
{
  if (clusterlist_module_found)
    return 1;
  return 0;
}

int 
cerebro_module_config_module_found(void)
{
  if (config_module_found)
    return 1;
  return 0;
}

char *
cerebro_clusterlist_module_name(void)
{
  if (!clusterlist_module_info)
    {
      cerebro_err_debug_lib("%s(%s:%d): clusterlist_module_info null",
			    __FILE__, __FUNCTION__, __LINE__);
      return NULL;
    }

  return clusterlist_module_info->clusterlist_module_name;
}

int
cerebro_clusterlist_module_setup(void)
{
  if (!clusterlist_module_info)
    {
      cerebro_err_debug_lib("%s(%s:%d): clusterlist_module_info null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  return ((*clusterlist_module_info->setup)());
}

int
cerebro_clusterlist_module_cleanup(void)
{
  if (!clusterlist_module_info)
    {
      cerebro_err_debug_lib("%s(%s:%d): clusterlist_module_info null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  return ((*clusterlist_module_info->cleanup)());
}

int
cerebro_clusterlist_module_get_all_nodes(char **nodes, 
					 unsigned int nodeslen)
{
  if (!clusterlist_module_info)
    {
      cerebro_err_debug_lib("%s(%s:%d): clusterlist_module_info null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  return ((*clusterlist_module_info->get_all_nodes)(nodes, nodeslen));
}

int
cerebro_clusterlist_module_numnodes(void)
{
  if (!clusterlist_module_info)
    {
      cerebro_err_debug_lib("%s(%s:%d): clusterlist_module_info null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  return ((*clusterlist_module_info->numnodes)());
}

int
cerebro_clusterlist_module_node_in_cluster(const char *node)
{
  if (!clusterlist_module_info)
    {
      cerebro_err_debug_lib("%s(%s:%d): clusterlist_module_info null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  return ((*clusterlist_module_info->node_in_cluster)(node));
}

int
cerebro_clusterlist_module_get_nodename(const char *node, 
					char *buf, 
					unsigned int buflen)
{
  if (!clusterlist_module_info)
    {
      cerebro_err_debug_lib("%s(%s:%d): clusterlist_module_info null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  return ((*clusterlist_module_info->get_nodename)(node, buf, buflen));
}

char *
cerebro_config_module_name(void)
{
  if (!config_module_info)
    {
      cerebro_err_debug_lib("%s(%s:%d): config_module_info null",
			    __FILE__, __FUNCTION__, __LINE__);
      return NULL;
    }

  return config_module_info->config_module_name;
}

int
cerebro_config_module_setup(void)
{
  if (!config_module_info)
    {
      cerebro_err_debug_lib("%s(%s:%d): config_module_info null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  return ((*config_module_info->setup)());
}

int
cerebro_config_module_cleanup(void)
{
  if (!config_module_info)
    {
      cerebro_err_debug_lib("%s(%s:%d): config_module_info null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  return ((*config_module_info->cleanup)());
}
int
cerebro_config_module_load_default(struct cerebro_config *conf)
{
  if (!config_module_info)
    {
      cerebro_err_debug_lib("%s(%s:%d): config_module_info null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  return ((*config_module_info->load_default)(conf));
}
