/*****************************************************************************\
 *  $Id: cerebro_module.c,v 1.3 2005-04-29 06:53:35 achu Exp $
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

/*
 * clusterlist_module_dl_handle
 *
 * clusterlist module dynamically loaded module handle
 */
static lt_dlhandle clusterlist_module_dl_handle = NULL;

/*
 * clusterlist_module_info
 *
 * clusterlist module info and operations
 */
static struct cerebro_clusterlist_module_info *clusterlist_module_info = NULL;

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

/*
 * config_module_dl_handle
 *
 * config module dynamically loaded module handle
 */
static lt_dlhandle config_module_dl_handle = NULL;

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
 * Returns 1 when a module is found, 0 when one is not.
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


int 
cerebro_module_setup(void)
{
  if (lt_dlinit() != 0)
    {
      cerebro_err_debug("%s(%s:%d): lt_dlinit: %s", 
			__FILE__, __FUNCTION__, __LINE__, lt_dlerror());
      return -1;
    }

  cerebro_module_library_initialized = 1;
  return 0;
}

int 
cerebro_module_cleanup(void)
{
  if (lt_dlexit() != 0)
    {
      cerebro_err_debug("%s(%s:%d): lt_dlexit: %s", 
			__FILE__, __FUNCTION__, __LINE__, lt_dlerror());
      return -1;
    }

  cerebro_module_library_initialized = 0;
  return 0;
}

int
cerebro_load_clusterlist_module(char *module_path)
{
  lt_dlhandle clusterlist_module_dl_handle_l = NULL;
  struct cerebro_clusterlist_module_info *clusterlist_module_info_l = NULL;

  if (!cerebro_module_library_initialized)
    {
      cerebro_err_debug("%s(%s:%d): cerebro_module_library uninitialized", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!module_path)
    {
      cerebro_err_debug("%s(%s:%d): module_path null", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (clusterlist_module_info || clusterlist_module_dl_handle)
    {
      cerebro_err_debug("%s(%s:%d): clusterlist module already loaded", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  if (!(clusterlist_module_dl_handle_l = lt_dlopen(module_path)))
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

  clusterlist_module_dl_handle = clusterlist_module_dl_handle_l;
  clusterlist_module_info = clusterlist_module_info_l;
  return 1;

 cleanup:
  if (clusterlist_module_dl_handle)
    lt_dlclose(clusterlist_module_dl_handle);
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
  
  if (clusterlist_module_dl_handle)
    lt_dlclose(clusterlist_module_dl_handle);

  clusterlist_module_dl_handle = NULL;
  clusterlist_module_info = NULL;
  return 0;
}

int 
cerebro_find_clusterlist_module(void)
{
  if (!cerebro_module_library_initialized)
    {
      cerebro_err_debug("%s(%s:%d): cerebro_module_library uninitialized", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (_cerebro_find_known_module(CEREBRO_CLUSTERLIST_MODULE_BUILDDIR,
				 dynamic_clusterlist_modules,
				 dynamic_clusterlist_modules_len,
				 cerebro_load_clusterlist_module))
    goto done;

  if (_cerebro_find_known_module(CEREBRO_MODULE_DIR,
				 dynamic_clusterlist_modules,
				 dynamic_clusterlist_modules_len,
				 cerebro_load_clusterlist_module))
    goto done;

  
  if (_cerebro_find_unknown_module(CEREBRO_MODULE_DIR,
				   "cerebro_clusterlist_",
				   cerebro_load_clusterlist_module))
    goto done;

  return 0;

 done:
  return 1;
}

int 
cerebro_load_config_module(char *module_path)
{
  lt_dlhandle config_module_dl_handle_l = NULL;
  struct cerebro_config_module_info *config_module_info_l = NULL;

  if (!cerebro_module_library_initialized)
    {
      cerebro_err_debug("%s(%s:%d): cerebro_module_library uninitialized", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!module_path)
    {
      cerebro_err_debug("%s(%s:%d): module_path null", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (config_module_info || config_module_dl_handle)
    {
      cerebro_err_debug("%s(%s:%d): config module already loaded", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  if (!(config_module_dl_handle_l = lt_dlopen(module_path)))
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

  config_module_dl_handle = config_module_dl_handle_l;
  config_module_info = config_module_info_l;
  return 1;

 cleanup:
  if (config_module_dl_handle)
    lt_dlclose(config_module_dl_handle);
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

  if (config_module_dl_handle)
    lt_dlclose(config_module_dl_handle);

  config_module_dl_handle = NULL;
  config_module_info = NULL;
  return 0;
}

int 
cerebro_find_config_module(void)
{
  if (!cerebro_module_library_initialized)
    {
      cerebro_err_debug("%s(%s:%d): cerebro_module_library uninitialized", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (_cerebro_find_known_module(CEREBRO_CONFIG_MODULE_BUILDDIR,
				 dynamic_config_modules,
				 dynamic_config_modules_len,
				 cerebro_load_config_module))
    goto done;

  if (_cerebro_find_known_module(CEREBRO_MODULE_DIR,
				 dynamic_config_modules,
				 dynamic_config_modules_len,
				 cerebro_load_config_module))
    goto done;

  
  if (_cerebro_find_unknown_module(CEREBRO_MODULE_DIR,
				   "cerebro_config_",
				   cerebro_load_config_module))
    goto done;

  return 0;

 done:
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

char *
cerebrod_clusterlist_module_name(void)
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
cerebrod_clusterlist_parse_options(char **options)
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
cerebrod_clusterlist_setup(void)
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
cerebrod_clusterlist_cleanup(void)
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
cerebrod_clusterlist_get_all_nodes(char **nodes, unsigned int nodeslen)
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
cerebrod_clusterlist_numnodes(void)
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
cerebrod_clusterlist_node_in_cluster(char *node)
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
cerebrod_clusterlist_get_nodename(char *node, char *buf, unsigned int buflen)
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
cerebrod_config_module_name(void)
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
cerebrod_config_parse_options(char **options)
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
cerebrod_config_setup(void)
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
cerebrod_config_cleanup(void)
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
cerebrod_config_load_cerebrod_default(struct cerebrod_module_config *conf)
{
  if (!config_module_info)
    {
      cerebro_err_debug("%s(%s:%d): config_module_info not loaded",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  return ((*config_module_info->load_cerebrod_default)(conf));
}
