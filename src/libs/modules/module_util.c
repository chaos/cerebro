/*****************************************************************************\
 *  $Id: module_util.c,v 1.10 2005-07-02 13:49:36 achu Exp $
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
#include "cerebro/cerebro_constants.h"

#include "debug.h"
#include "ltdl.h"
#include "module_util.h"

/*  
 * module_setup_count
 *
 * indicates the number of times module setup function has been called
 */
int module_setup_count = 0;

/* 
 * _load_module
 * 
 * Load module indicated by parameters and call module callback
 *
 * Returns 1 if module loaded and stored, 0 if not, -1 on error
 */
static int
_load_module(char *search_dir,
             char *filename,
             Module_callback module_cb,
             char *module_info_sym,
             void *handle)
{
  char filebuf[CEREBRO_MAX_PATH_LEN+1];
  lt_dlhandle dl_handle;
  void *module_info;
  int flag, rv = -1;

  if (!search_dir || !filename || !module_cb || !module_info_sym || !handle)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  memset(filebuf, '\0', CEREBRO_MAX_PATH_LEN+1);
  snprintf(filebuf, CEREBRO_MAX_PATH_LEN, "%s/%s", search_dir, filename);

  if (!(dl_handle = lt_dlopen(filebuf)))
    {
      CEREBRO_DBG(("lt_dlopen: %s, %s", filebuf, lt_dlerror()));
      goto cleanup;
    }
              
  /* clear lt_dlerror */
  lt_dlerror();
  
  if (!(module_info = lt_dlsym(dl_handle, module_info_sym)))
    {
      const char *err = lt_dlerror();
      if (err)
        CEREBRO_DBG(("lt_dlsym: %s, %s", filebuf, err));
      goto cleanup;
    }
  
  if ((flag = module_cb(handle, dl_handle, module_info)) < 0)
    goto cleanup;

  if (!flag)
    lt_dlclose(dl_handle);

  return flag;

 cleanup:
  if (dl_handle)
    lt_dlclose(dl_handle);
  return rv;
}

/*
 * _find_known_module
 *
 * Try to find a known module from the modules list in the search
 * directory.
 *
 * - search_dir - directory to search
 * - modules_list - list of modules to search for
 * - modules_list_len - length of list
 * - module_cb - function to call when a possible module is found
 * - handle - pointer to module handle
 *
 * Returns 1 if module is loaded, 0 if it isn't, -1 on fatal error
 */
static int
_find_known_module(char *search_dir,
                   char **modules_list,
                   int modules_list_len,
                   Module_callback module_cb,
                   char *module_info_sym,
                   void *handle)
{
  DIR *dir;
  int i = 0, found = 0;

  if (!search_dir
      || !modules_list
      || !(modules_list_len > 0)
      || !module_cb
      || !module_info_sym
      || !handle)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  if (!(dir = opendir(search_dir)))
    {
      /* Return 0, since dir may simply not exist */
      CEREBRO_DBG(("opendir: %s: %s", search_dir, strerror(errno)));
      return 0;
    }

  for (i = 0; i < modules_list_len; i++)
    {
      struct dirent *dirent;

      while ((dirent = readdir(dir)))
        {
          if (!strcmp(dirent->d_name, modules_list[i]))
            {
              int flag;

              if ((flag = _load_module(search_dir,
                                       modules_list[i],
                                       module_cb,
                                       module_info_sym,
                                       handle)) < 0)
                goto cleanup;

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

 cleanup:
  closedir(dir);
  return -1;
}

/*
 * _find_unknown_modules
 *
 * Search a directory for modules
 *
 * - search_dir - directory to search
 * - signature - filename signature indicating if the filename is a
 *               module we want to try and load
 * - module_cb - function to call when a possible module is found
 * - handle - pointer to module handle
 * - modules_max - maximum modules that can be found
 *
 * Returns 1 when a module(s) are found, 0 if not, -1 on fatal error
 */
static int 
_find_unknown_modules(char *search_dir,
                      char *signature,
                      Module_callback module_cb,
                      char *module_info_sym,
                      void *handle,
                      unsigned int modules_max)
{
  DIR *dir;
  struct dirent *dirent;
  int found = 0;

  if (!search_dir
      || !signature
      || !module_cb
      || !module_info_sym
      || !handle
      || !modules_max)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  if (!(dir = opendir(search_dir)))
    {
      /* Return 0, since dir may simply not exist */
      CEREBRO_DBG(("opendir: %s: %s", search_dir, strerror(errno)));
      return 0;
    }

  while ((dirent = readdir(dir)))
    {
      char *ptr;

      /* Don't bother unless its a shared object */
      ptr = strchr(dirent->d_name, '.');
      if (!ptr || strcmp(ptr, ".so"))
        continue;

      ptr = strstr(dirent->d_name, signature);
      if (ptr && ptr == &dirent->d_name[0])
        {
          int flag;

          if ((flag = _load_module(search_dir,
                                   dirent->d_name,
                                   module_cb,
                                   module_info_sym,
                                   handle)) < 0)
            goto cleanup;
          
          if (flag)
            found++;
          
          if (found >= modules_max)
            goto out;
        }
    }

 out:
  closedir(dir);
  return (found) ? 1 : 0;

 cleanup:
  return -1;
}

int 
find_and_load_modules(char *module_dir,
                      char **modules_list,
                      int modules_list_len,
                      char *signature,
                      Module_callback module_cb,
                      char *module_info_sym,
                      void *handle,
                      unsigned int modules_max)
{
  int rv;

  /* modules_list and modules_list_len need not be passed in */
  if (!module_dir
      || !signature
      || !module_cb
      || !module_info_sym
      || !handle
      || !modules_max)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  if (modules_list && modules_list_len)
    {
#if CEREBRO_DEBUG
      if ((rv = _find_known_module(module_dir,
                                   modules_list,
                                   modules_list_len,
                                   module_cb,
                                   module_info_sym,
                                   handle)) < 0)
        return -1;
      
      if (rv)
        return 1;
#endif /* CEREBRO_DEBUG */
      
      if ((rv = _find_known_module(CEREBRO_MODULE_DIR,
                                   modules_list,
                                   modules_list_len,
                                   module_cb,
                                   module_info_sym,
                                   handle)) < 0)
        return -1;
      
      if (rv)
        return 1;
    }

#if CEREBRO_DEBUG
  if ((rv = _find_unknown_modules(module_dir,
                                  signature,
                                  module_cb,
                                  module_info_sym,
                                  handle,
                                  modules_max)) < 0)
    return -1;
  
  if (rv)
    return 1;
#endif /* CEREBRO_DEBUG */

  if ((rv = _find_unknown_modules(CEREBRO_MODULE_DIR,
                                  signature,
                                  module_cb,
                                  module_info_sym,
                                  handle,
                                  modules_max)) < 0)
    return -1;

  if (rv)
    return 1;

  return 0;
}

int 
module_setup(void)
{
  if (module_setup_count)
    goto out;

  if (lt_dlinit() != 0)
    {
      CEREBRO_DBG(("lt_dlinit: %s", lt_dlerror()));
      return -1;
    }

 out:
  module_setup_count++;
  return 0;
}

int 
module_cleanup(void)
{
  if (module_setup_count)
    module_setup_count--;

  if (!module_setup_count)
    {
      if (lt_dlexit() != 0)
        {
          CEREBRO_DBG(("lt_dlexit: %s", lt_dlerror()));
          return -1;
        }
    }

  return 0;
}
