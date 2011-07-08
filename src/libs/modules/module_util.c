/*****************************************************************************\
 *  $Id: module_util.c,v 1.22 2010-02-02 01:01:20 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2010 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2005-2007 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <http://www.llnl.gov/linux/cerebro/>.
 *
 *  Cerebro is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  Cerebro is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with Cerebro. If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#if !WITH_STATIC_MODULES
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <dirent.h>
#endif /* !WITH_STATIC_MODULES */

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"

#include "debug.h"
#if !WITH_STATIC_MODULES
#include "ltdl.h"
#endif /* !WITH_STATIC_MODULES */
#include "module_util.h"

/*  
 * module_setup_count
 *
 * indicates the number of times module setup function has been called
 */
int module_setup_count = 0;

#if WITH_STATIC_MODULES
int 
find_and_load_modules(void **modules_list,
                      Module_callback module_cb,
                      void *handle,
                      unsigned int modules_max)
{
  int found = 0;
 
  /* modules_list need not be passed in */
  if (!module_cb
      || !handle
      || !modules_max)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  if (modules_list)
    {
      void *module_info;
      int i = 0;

      while ((module_info = modules_list[i]))
        {
          int flag;
          
          if ((flag = module_cb(handle, module_info)) < 0)
            return -1;

          if (flag)
            found++;
          
          if (found >= modules_max)
            goto out;

          i++;
        }
    }

 out:
  return (found) ? 1 : 0;
}
#else /* !WITH_STATIC_MODULES */
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
  struct stat buf;
  lt_dlhandle dl_handle;
  void *module_info;
  int flag;

  if (!search_dir || !filename || !module_cb || !module_info_sym || !handle)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  memset(filebuf, '\0', CEREBRO_MAX_PATH_LEN+1);
  snprintf(filebuf, CEREBRO_MAX_PATH_LEN, "%s/%s", search_dir, filename);

  if (stat(filebuf, &buf) < 0)
    return 0;

  if (!(dl_handle = lt_dlopen(filebuf)))
    {
      CEREBRO_ERR(("lt_dlopen: %s, %s", filebuf, lt_dlerror()));
      goto cleanup;
    }
              
  /* clear lt_dlerror */
  lt_dlerror();
  
  if (!(module_info = lt_dlsym(dl_handle, module_info_sym)))
    {
      lt_dlclose(dl_handle);
      return 0;
    }
  
  if ((flag = module_cb(handle, dl_handle, module_info)) < 0)
    goto cleanup;

  if (!flag)
    lt_dlclose(dl_handle);

  return flag;

 cleanup:
  if (dl_handle)
    lt_dlclose(dl_handle);
  return -1;
}

/*
 * _find_known_module
 *
 * Try to find a known module from the modules list in the search
 * directory.
 *
 * - search_dir - directory to search
 * - modules_list - list of modules to search for
 * - module_cb - function to call when a possible module is found
 * - handle - pointer to module handle
 *
 * Returns 1 if module is loaded, 0 if it isn't, -1 on fatal error
 */
static int
_find_known_module(char *search_dir,
                   char **modules_list,
                   Module_callback module_cb,
                   char *module_info_sym,
                   void *handle)
{
  DIR *dir;
  int i = 0, found = 0;

  if (!search_dir
      || !modules_list
      || !module_cb
      || !module_info_sym
      || !handle)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  if (!(dir = opendir(search_dir)))
    {
      if (errno != ENOENT)
        {
          CEREBRO_ERR(("opendir: %s: %s", search_dir, strerror(errno)));
          return -1;
        }
      return 0;
    }

  while (modules_list[i])
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
      i++;
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
  DIR *dir = NULL;
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
      if (errno != ENOENT)
        {
          CEREBRO_ERR(("opendir: %s: %s", search_dir, strerror(errno)));
          return -1;
        }
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
  closedir(dir);
  return -1;
}

int 
find_and_load_modules(char *module_dir,
                      char **modules_list,
                      char *signature,
                      Module_callback module_cb,
                      char *module_info_sym,
                      void *handle,
                      unsigned int modules_max)
{
  int rv;

  /* modules_list need not be passed in */
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

  if (modules_list)
    {
#if CEREBRO_DEBUG
      if ((rv = _find_known_module(module_dir,
                                   modules_list,
                                   module_cb,
                                   module_info_sym,
                                   handle)) < 0)
        return -1;
      
      if (rv)
        return 1;
#endif /* CEREBRO_DEBUG */
      
      if ((rv = _find_known_module(CEREBRO_MODULE_DIR,
                                   modules_list,
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
#endif /* !WITH_STATIC_MODULES */

int 
module_setup(void)
{
  if (module_setup_count)
    goto out;

#if !WITH_STATIC_MODULES
  if (lt_dlinit() != 0)
    {
      CEREBRO_ERR(("lt_dlinit: %s", lt_dlerror()));
      return -1;
    }
#endif /* !WITH_STATIC_MODULES */

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
#if !WITH_STATIC_MODULES
      if (lt_dlexit() != 0)
        {
          CEREBRO_DBG(("lt_dlexit: %s", lt_dlerror()));
          return -1;
        }
#endif /* !WITH_STATIC_MODULES */
    }

  return 0;
}

