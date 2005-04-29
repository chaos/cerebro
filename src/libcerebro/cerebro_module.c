/*****************************************************************************\
 *  $Id: cerebro_module.c,v 1.9 2005-04-29 20:52:16 achu Exp $
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
#include "cerebro_defs.h"
#include "cerebro_error.h"
#include "cerebro_module.h"
#include "ltdl.h"

/*  
 * cerebro_module_library_initialized
 *
 * indicates if the module library has been initialized
 */
static int cerebro_module_library_initialized = 0;

#if WITH_STATIC_MODULES

#if WITH_GENDERSLLNL
extern struct cerebro_config_module_info gendersllnl_config_module_info;
extern struct cerebro_clusterlist_module_info gendersllnl_clusterlist_module_info;
#endif /* WITH_GENDERSLLNL */

#if WITH_GENDERS
extern struct cerebro_clusterlist_module_info genders_clusterlist_module_info;
#endif /* WITH_GENDERS */

extern struct cerebro_clusterlist_module_info hostsfile_clusterlist_module_info;
extern struct cerebro_clusterlist_module_info none_clusterlist_module_info;

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
    &hostsfile_clusterlist_module_info,
    &none_clusterlist_module_info,
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
  "cerebro_clusterlist_none.la",
  "cerebro_clusterlist_hostsfile.la",
  NULL
};
int dynamic_clusterlist_modules_len = 4;

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
 * config_module_info
 *
 * config module info and operations
 */
static struct cerebro_config_module_info *config_module_info = NULL;

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
 * _cerebro_find_known_modules
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
_cerebro_find_known_module(char *search_dir,
			   char **modules_list,
			   int modules_list_len,
			   Cerebro_load_module load_module)
{
  DIR *dir;
  int i = 0, found = 0;
                                                                                     
  if (!search_dir)
    {
      cerebro_err_debug("%s(%s:%d): search_dir null", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!modules_list)
    {
      cerebro_err_debug("%s(%s:%d): modules_list null", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!(modules_list_len > 0))
    {
      cerebro_err_debug("%s(%s:%d): modules_list_len not valid",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  if (!load_module)
    {
      cerebro_err_debug("%s(%s:%d): load_module null", 
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
              int ret;

              memset(filebuf, '\0', CEREBRO_MAXPATHLEN+1);
              snprintf(filebuf, CEREBRO_MAXPATHLEN, "%s/%s",
                       search_dir, modules_list[i]);

              if ((ret = load_module(filebuf)) < 0)
		{
		  cerebro_err_debug("%s(%s:%d): load_module: %s",
				    __FILE__, __FUNCTION__, __LINE__,
				    strerror(errno));
		  return -1;
		}

              if (ret)
                {
                  found++;
                  goto done;
                }
            }
        }
      rewinddir(dir);
    }

 done:
  closedir(dir);
  return (found) ? 1 : 0;
}

/*
 * _cerebro_find_unknown_module
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
_cerebro_find_unknown_module(char *search_dir,
			     char *signature,
			     Cerebro_load_module load_module)
{
  DIR *dir;
  struct dirent *dirent;
  int found = 0;

  if (!search_dir)
    {
      cerebro_err_debug("%s(%s:%d): search_dir null", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!signature)
    {
      cerebro_err_debug("%s(%s:%d): signature null", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
 
  if (!load_module)
    {
      cerebro_err_debug("%s(%s:%d): load_module null", 
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
          int ret;

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

          if ((ret = load_module(filebuf)) < 0)
	    {
	      cerebro_err_debug("%s(%s:%d): load_module: %s",
				__FILE__, __FUNCTION__, __LINE__,
				strerror(errno));
	      return -1;
	    }

          if (ret)
            {
              found++;
              goto done;
            }
        }
    }

 done:
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
      cerebro_err_debug("%s(%s:%d): lt_dlinit: %s", 
			__FILE__, __FUNCTION__, __LINE__, lt_dlerror());
      return -1;
    }
#endif /* !WITH_STATIC_MODULES */

  cerebro_module_library_initialized = 1;
  return 0;
}

int 
cerebro_module_cleanup(void)
{
  cerebro_unload_clusterlist_module();
  cerebro_unload_config_module();

#if !WITH_STATIC_MODULES
  if (lt_dlexit() != 0)
    {
      cerebro_err_debug("%s(%s:%d): lt_dlexit: %s", 
			__FILE__, __FUNCTION__, __LINE__, lt_dlerror());
      return -1;
    }
#endif /* !WITH_STATIC_MODULES */

  cerebro_module_library_initialized = 0;
  return 0;
}

int
cerebro_load_clusterlist_module(char *module)
{
#if WITH_STATIC_MODULES
  struct cerebro_clusterlist_module_info **ptr;
  int i = 0;
#else  /* !WITH_STATIC_MODULES */
  lt_dlhandle clusterlist_module_dl_handle_l = NULL;
#endif /* !WITH_STATIC_MODULES */
  struct cerebro_clusterlist_module_info *clusterlist_module_info_l = NULL;

  if (!cerebro_module_library_initialized)
    {
      cerebro_err_debug("%s(%s:%d): cerebro_module_library uninitialized", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!module)
    {
      cerebro_err_debug("%s(%s:%d): module null", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (clusterlist_module_info)
    {
      cerebro_err_debug("%s(%s:%d): clusterlist module already loaded", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
#if WITH_STATIC_MODULES
  ptr = &static_clusterlist_modules[0];
  while (ptr[i] != NULL)
    {      
      if (!ptr[i]->clusterlist_module_name)
	{
	  cerebro_err_debug("static clusterlist module index '%d' "
			    "does not contain name", i);
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
      cerebro_err_debug("clusterlist module '%s': lt_dlopen: %s",
                        lt_dlerror());
      goto cleanup;
    }

  /* clear lt_dlerror */
  lt_dlerror();

  if (!(clusterlist_module_info_l = lt_dlsym(clusterlist_module_dl_handle_l, "clusterlist_module_info")))
    {
      const char *err = lt_dlerror();
      if (err != NULL)
	cerebro_err_debug("clusterlist module '%s': lt_dlsym: %s", err);
      goto cleanup;
    }
#endif /* !WITH_STATIC_MODULES */

  if (!clusterlist_module_info_l->clusterlist_module_name)
    {
      cerebro_err_debug("clusterlist module '%s' does not contain a valid name",
                        clusterlist_module_info_l->clusterlist_module_name);
      goto cleanup;
    }

  if (!clusterlist_module_info_l->parse_options)
    {
      cerebro_err_debug("clusterlist module '%s' does not contain "
                        "valid parse_options function",
                        clusterlist_module_info_l->clusterlist_module_name);
      goto cleanup;
    }

  if (!clusterlist_module_info_l->setup)
    {
      cerebro_err_debug("clusterlist module '%s' does not contain "
                        "valid setup function",
                        clusterlist_module_info_l->clusterlist_module_name);
      goto cleanup;
    }

  if (!clusterlist_module_info_l->cleanup)
    {
      cerebro_err_debug("clusterlist module '%s' does not contain "
                        "valid cleanup function",
                        clusterlist_module_info_l->clusterlist_module_name);
      goto cleanup;
    }

  if (!clusterlist_module_info_l->get_all_nodes)
    {
      cerebro_err_debug("clusterlist module '%s' does not contain "
                        "valid get_all_nodes function",
                        clusterlist_module_info_l->clusterlist_module_name);
      goto cleanup;
    }

  if (!clusterlist_module_info_l->numnodes)
    {
      cerebro_err_debug("clusterlist module '%s' does not contain "
                        "valid numnodes function",
                        clusterlist_module_info_l->clusterlist_module_name);
      goto cleanup;
    }

  if (!clusterlist_module_info_l->node_in_cluster)
    {
      cerebro_err_debug("clusterlist module '%s' does not contain "
                        "valid node_in_cluster function",
                        clusterlist_module_info_l->clusterlist_module_name);
      goto cleanup;
    }

  if (!clusterlist_module_info_l->get_nodename)
    {
      cerebro_err_debug("clusterlist module '%s' does not contain "
                        "valid get_nodename function",
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
cerebro_unload_clusterlist_module(void)
{
  if (!cerebro_module_library_initialized)
    {
      cerebro_err_debug("%s(%s:%d): cerebro_module_library uninitialized", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
#if !WITH_STATIC_MODULES
  if (clusterlist_module_dl_handle)
    lt_dlclose(clusterlist_module_dl_handle);
  clusterlist_module_dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */
  clusterlist_module_info = NULL;
  return 0;
}

int 
cerebro_find_clusterlist_module(void)
{
#if WITH_STATIC_MODULES
  struct cerebro_clusterlist_module_info **ptr;
  int i = 0;
#endif /* WITH_STATIC_MODULES */
  int rv;

  if (!cerebro_module_library_initialized)
    {
      cerebro_err_debug("%s(%s:%d): cerebro_module_library uninitialized", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

#if WITH_STATIC_MODULES
  ptr = &static_clusterlist_modules[0];
  while (ptr[i] != NULL)
    {
      if (!ptr[i]->clusterlist_module_name)
	{
	  cerebro_err_debug("static clusterlist module index '%d' "
			    "does not contain name", i);
	  continue;
	}
                                                                                      
      if ((rv = cerebro_load_clusterlist_module(ptr[i]->clusterlist_module_name)) < 0)
	{
	  cerebro_err_debug("%s(%s:%d): cerebro_load_clusterlist_module: %s",
			    __FILE__, __FUNCTION__, __LINE__,
			    strerror(errno));
	  return -1;
	}

      if (rv)
	goto found;
      i++;
    }
#else  /* !WITH_STATIC_MODULES */
  if ((rv = _cerebro_find_known_module(CEREBRO_CLUSTERLIST_MODULE_BUILDDIR,
                                       dynamic_clusterlist_modules,
                                       dynamic_clusterlist_modules_len,
                                       cerebro_load_clusterlist_module)) < 0)
      return -1;

  if (rv)
    goto found;

  if ((rv = _cerebro_find_known_module(CEREBRO_MODULE_DIR,
                                       dynamic_clusterlist_modules,
                                       dynamic_clusterlist_modules_len,
                                       cerebro_load_clusterlist_module)) < 0)
    return -1;

  if (rv)
    goto found;

  
  if ((rv = _cerebro_find_unknown_module(CEREBRO_MODULE_DIR,
                                         CEREBRO_CLUSTERLIST_FILENAME_SIGNATURE,
                                         cerebro_load_clusterlist_module)) < 0)
    return -1;
  
  if (rv)
    goto found;
#endif /* !WITH_STATIC_MODULES */

  return 0;

 found:
  return 1;
}

int 
cerebro_load_config_module(char *module)
{
#if WITH_STATIC_MODULES
  struct cerebro_config_module_info **ptr;
  int i = 0;
#else  /* !WITH_STATIC_MODULES */
  lt_dlhandle config_module_dl_handle_l = NULL;
#endif /* !WITH_STATIC_MODULES */
  struct cerebro_config_module_info *config_module_info_l = NULL;

  if (!cerebro_module_library_initialized)
    {
      cerebro_err_debug("%s(%s:%d): cerebro_module_library uninitialized", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!module)
    {
      cerebro_err_debug("%s(%s:%d): module null", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (config_module_info)
    {
      cerebro_err_debug("%s(%s:%d): config module already loaded", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
#if WITH_STATIC_MODULES
  ptr = &static_config_modules[0];
  while (ptr[i] != NULL)
    {      
      if (!ptr[i]->config_module_name)
	{
	  cerebro_err_debug("static config module index '%d' "
			    "does not contain name", i);
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
      cerebro_err_debug("config module '%s': lt_dlopen: %s",
                        lt_dlerror());
      goto cleanup;
    }

  /* clear lt_dlerror */
  lt_dlerror();

  if (!(config_module_info_l = lt_dlsym(config_module_dl_handle_l, "config_module_info")))
    {
      const char *err = lt_dlerror();
      if (err != NULL)
	cerebro_err_debug("config module '%s': lt_dlsym: %s", err);
      goto cleanup;
    }
#endif /* !WITH_STATIC_MODULES */

  if (!config_module_info_l->config_module_name)
    {
      cerebro_err_debug("config module '%s' does not contain a valid name",
                        config_module_info_l->config_module_name);
      goto cleanup;
    }

  if (!config_module_info_l->parse_options)
    {
      cerebro_err_debug("config module '%s' does not contain "
                        "valid parse_options function",
                        config_module_info_l->config_module_name);
      goto cleanup;
    }

  if (!config_module_info_l->setup)
    {
      cerebro_err_debug("config module '%s' does not contain "
                        "valid setup function",
                        config_module_info_l->config_module_name);
      goto cleanup;
    }

  if (!config_module_info_l->cleanup)
    {
      cerebro_err_debug("config module '%s' does not contain "
                        "valid cleanup function",
                        config_module_info_l->config_module_name);
      goto cleanup;
    }

  if (!config_module_info_l->load_cerebrod_default)
    {
      cerebro_err_debug("config module '%s' does not contain "
                        "valid load_cerebrod_default function",
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
cerebro_unload_config_module(void)
{
  if (!cerebro_module_library_initialized)
    {
      cerebro_err_debug("%s(%s:%d): cerebro_module_library uninitialized", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

#if !WITH_STATIC_MODULES
  if (config_module_dl_handle)
    lt_dlclose(config_module_dl_handle);
  config_module_dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */
  config_module_info = NULL;
  return 0;
}

int 
cerebro_find_config_module(void)
{
#if WITH_STATIC_MODULES
  struct cerebro_config_module_info **ptr;
  int i = 0;
#endif /* WITH_STATIC_MODULES */
  int rv;

  if (!cerebro_module_library_initialized)
    {
      cerebro_err_debug("%s(%s:%d): cerebro_module_library uninitialized", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

#if WITH_STATIC_MODULES
  ptr = &static_config_modules[0];
  while (ptr[i] != NULL)
    {
      if (!ptr[i]->config_module_name)
	{
	  cerebro_err_debug("static config module index '%d' "
			    "does not contain name", i);
	  continue;
	}

      if ((rv = cerebro_load_config_module(ptr[i]->config_module_name)) < 0)
	{
	  cerebro_err_debug("%s(%s:%d): cerebro_load_config_module: %s",
			    __FILE__, __FUNCTION__, __LINE__,
			    strerror(errno));
	  return -1;
	}

      if (rv)
	goto found;
      i++;
    }
#else  /* !WITH_STATIC_MODULES */
  if ((rv = _cerebro_find_known_module(CEREBRO_CONFIG_MODULE_BUILDDIR,
                                       dynamic_config_modules,
                                       dynamic_config_modules_len,
                                       cerebro_load_config_module)) < 0)
    return -1;

  if (rv)
    goto found;

  if ((rv = _cerebro_find_known_module(CEREBRO_MODULE_DIR,
                                       dynamic_config_modules,
                                       dynamic_config_modules_len,
                                       cerebro_load_config_module)) < 0)
    return -1;

  if (rv)
    goto found;

  if ((rv = _cerebro_find_unknown_module(CEREBRO_MODULE_DIR,
                                         CEREBRO_CONFIG_FILENAME_SIGNATURE,
                                         cerebro_load_config_module)) < 0)
    return -1;

  if (rv)
    goto found;
#endif /* !WITH_STATIC_MODULES */

  return 0;

 found:
  return 1;
}

int 
cerebro_clusterlist_is_loaded(void)
{
  if (clusterlist_module_info)
    return 1;
  return 0;
}

int 
cerebro_config_is_loaded(void)
{
  if (config_module_info)
    return 1;
  return 0;
}

#if WITH_STATIC_MODULES
int 
cerebro_lookup_clusterlist_module(char *module)
{
  struct cerebro_clusterlist_module_info **ptr;
  int i = 0;

  if (!module)
    {
      cerebro_err_debug("%s(%s:%d): module null", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  ptr = &static_clusterlist_modules[0];
  while (ptr[i] != NULL)
    {
      if (!ptr[i]->clusterlist_module_name)
        {
          cerebro_err_debug("static clusterlist module index '%d' "
                            "does not contain name", i);
          continue;
        }
      if (!strcmp(ptr[i]->clusterlist_module_name, module))
        return 1;
      i++;
    }

  return 0;
}

int 
cerebro_lookup_config_module(char *module)
{
 struct cerebro_config_module_info **ptr;
  int i = 0;

  if (!module)
    {
      cerebro_err_debug("%s(%s:%d): module null", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  ptr = &static_config_modules[0];
  while (ptr[i] != NULL)
    {
      if (!ptr[i]->config_module_name)
        {
          cerebro_err_debug("static config module index '%d' "
                            "does not contain name", i);
          continue;
        }
      if (!strcmp(ptr[i]->config_module_name, module))
        return 1;
      i++;
    }

  return 0;
}
#else  /* !WITH_STATIC_MODULES */
/* 
 * _cerebro_lookup_module_path
 *
 * Common function for cerebro_lookup_clusterlist_module_path
 * and cerebro_lookup_config_module_path.
 *
 * Returns 1 and path in buf when the path is found, 0 if not, -1 on
 * error
 */
int
_cerebro_lookup_module_path(char *str,
			    char *buf,
			    unsigned int buflen,
			    char *signature)
{
  struct stat statbuf;

  if (!str)
    {
      cerebro_err_debug("%s(%s:%d): str null", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!buf)
    {
      cerebro_err_debug("%s(%s:%d): buf null", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!(buflen > 0))
    {
      cerebro_err_debug("%s(%s:%d): buflen not valid", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!signature)
    {
      cerebro_err_debug("%s(%s:%d): signature null", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  memset(buf, '\0', buflen);

  if (str[0] == '/')
    {
      /* Case A: User specified an absolute path to the module */
      if (stat(str, &statbuf) < 0)
	return 0;

      if (strlen(str) >= buflen)
	{
	  cerebro_err_debug("%s(%s:%d): buflen too small: path=%s, buflen=%d", 
			    __FILE__, __FUNCTION__, __LINE__, str, buflen);
	  return -1;
	}
      strcpy(buf, str);
      return 1;
    }
  else
    {
      /* Case B: Search for module.  When developing/debugging, search
       * the builddir first, b/c we may be testing a new module.
       */
      char tempbuf[CEREBRO_MAXPATHLEN+1];
                                                                                      
      /* Assume the user passed in a filename, so search in the
       * appropriate directories 
       */

#ifndef NDEBUG
      memset(tempbuf, '\0', CEREBRO_MAXPATHLEN+1);
      snprintf(tempbuf, CEREBRO_MAXPATHLEN, "%s/%s",
	       CEREBRO_CONFIG_MODULE_BUILDDIR, str);
                                                                                      
      if (!stat(tempbuf, &statbuf))
	goto found;
#endif /* NDEBUG */

      memset(tempbuf, '\0', CEREBRO_MAXPATHLEN+1);
      snprintf(tempbuf, CEREBRO_MAXPATHLEN, "%s/%s",
	       CEREBRO_MODULE_DIR, str);
                                                                                      
      if (!stat(tempbuf, &statbuf))
	goto found;

      /* Next assume the user passed in a name to a .la or .so file */

#ifndef NDEBUG
      memset(tempbuf, '\0', CEREBRO_MAXPATHLEN+1);
      snprintf(tempbuf, CEREBRO_MAXPATHLEN, "%s/%s%s.la",
	       CEREBRO_CONFIG_MODULE_BUILDDIR, signature, str);
                                                                                      
      if (!stat(tempbuf, &statbuf))
	goto found;

      memset(tempbuf, '\0', CEREBRO_MAXPATHLEN+1);
      snprintf(tempbuf, CEREBRO_MAXPATHLEN, "%s/%s%s.so",
	       CEREBRO_CONFIG_MODULE_BUILDDIR, signature, str);
                                                                                      
      if (!stat(tempbuf, &statbuf))
	goto found;
#endif /* NDEBUG */

      memset(tempbuf, '\0', CEREBRO_MAXPATHLEN+1);
      snprintf(tempbuf, CEREBRO_MAXPATHLEN, "%s/%s%s.la",
	       CEREBRO_MODULE_DIR, signature, str);
                                                                                      
      if (!stat(tempbuf, &statbuf))
	goto found;

      memset(tempbuf, '\0', CEREBRO_MAXPATHLEN+1);
      snprintf(tempbuf, CEREBRO_MAXPATHLEN, "%s/%s%s.so",
	       CEREBRO_MODULE_DIR, signature, str);
                                                                                      
      if (!stat(tempbuf, &statbuf))
	goto found;
                                                                                      
      return 0;

 found:
      if (strlen(tempbuf) >= buflen)
	{
	  cerebro_err_debug("%s(%s:%d): buflen too small: path=%s, buflen=%d", 
			    __FILE__, __FUNCTION__, __LINE__, tempbuf, buflen);
	  return -1;
	}

      strcpy(buf, tempbuf);
      return 1;
    }

  /* NOT REACHED */
  return 0;
}

int 
cerebro_lookup_clusterlist_module_path(char *str, 
				       char *buf, 
				       unsigned int buflen)
{
  return _cerebro_lookup_module_path(str,
				     buf,
				     buflen,
				     CEREBRO_CLUSTERLIST_FILENAME_SIGNATURE);
}

int 
cerebro_lookup_config_module_path(char *str, 
				  char *buf, 
				  unsigned int buflen)
{
  return _cerebro_lookup_module_path(str,
				     buf,
				     buflen,
				     CEREBRO_CONFIG_FILENAME_SIGNATURE);
}
#endif /* !WITH_STATIC_MODULES */

char *
cerebro_clusterlist_module_name(void)
{
  if (!clusterlist_module_info)
    {
      cerebro_err_debug("%s(%s:%d): clusterlist_module_info not loaded",
			__FILE__, __FUNCTION__, __LINE__);
      return NULL;
    }
                                                                                     
  return clusterlist_module_info->clusterlist_module_name;
}
                                                                                     
int
cerebro_clusterlist_parse_options(char **options)
{
  if (!clusterlist_module_info)
    {
      cerebro_err_debug("%s(%s:%d): clusterlist_module_info not loaded",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  return ((*clusterlist_module_info->parse_options)(options));
}
                                                                                     
int
cerebro_clusterlist_setup(void)
{
  if (!clusterlist_module_info)
    {
      cerebro_err_debug("%s(%s:%d): clusterlist_module_info not loaded",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  return ((*clusterlist_module_info->setup)());
}

int
cerebro_clusterlist_cleanup(void)
{
  if (!clusterlist_module_info)
    {
      cerebro_err_debug("%s(%s:%d): clusterlist_module_info not loaded",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  return ((*clusterlist_module_info->cleanup)());
}
int
cerebro_clusterlist_get_all_nodes(char **nodes, unsigned int nodeslen)
{
  if (!clusterlist_module_info)
    {
      cerebro_err_debug("%s(%s:%d): clusterlist_module_info not loaded",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  return ((*clusterlist_module_info->get_all_nodes)(nodes, nodeslen));
}
                                                                                     
int
cerebro_clusterlist_numnodes(void)
{
  if (!clusterlist_module_info)
    {
      cerebro_err_debug("%s(%s:%d): clusterlist_module_info not loaded",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  return ((*clusterlist_module_info->numnodes)());
}
                                                                                     
int
cerebro_clusterlist_node_in_cluster(char *node)
{
  if (!clusterlist_module_info)
    {
      cerebro_err_debug("%s(%s:%d): clusterlist_module_info not loaded",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  return ((*clusterlist_module_info->node_in_cluster)(node));
}

int
cerebro_clusterlist_get_nodename(char *node, char *buf, unsigned int buflen)
{
  if (!clusterlist_module_info)
    {
      cerebro_err_debug("%s(%s:%d): clusterlist_module_info not loaded",
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
      cerebro_err_debug("%s(%s:%d): config_module_info not loaded",
			__FILE__, __FUNCTION__, __LINE__);
      return NULL;
    }
                                                                                     
  return config_module_info->config_module_name;
}
                                                                                     
int
cerebro_config_parse_options(char **options)
{
  if (!config_module_info)
    {
      cerebro_err_debug("%s(%s:%d): config_module_info not loaded",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  return ((*config_module_info->parse_options)(options));
}
                                                                                     
int
cerebro_config_setup(void)
{
  if (!config_module_info)
    {
      cerebro_err_debug("%s(%s:%d): config_module_info not loaded",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  return ((*config_module_info->setup)());
}

int
cerebro_config_cleanup(void)
{
  if (!config_module_info)
    {
      cerebro_err_debug("%s(%s:%d): config_module_info not loaded",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  return ((*config_module_info->cleanup)());
}
int
cerebro_config_load_cerebrod_default(struct cerebrod_module_config *conf)
{
  if (!config_module_info)
    {
      cerebro_err_debug("%s(%s:%d): config_module_info not loaded",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  return ((*config_module_info->load_cerebrod_default)(conf));
}
