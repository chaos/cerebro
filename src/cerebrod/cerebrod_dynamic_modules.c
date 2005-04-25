/*****************************************************************************\
 *  $Id: cerebrod_dynamic_modules.c,v 1.8 2005-04-25 15:33:05 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if !WITH_STATIC_MODULES

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <errno.h>
#include <assert.h>

#include <sys/param.h>

#include "cerebro_error.h"

#include "cerebrod_dynamic_modules.h"
#include "cerebrod_config.h"
#include "cerebrod_clusterlist.h"
#include "wrappers.h"

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

int
cerebrod_lookup_module(char *search_dir,
		       char **modules_list,
		       int modules_list_len,
		       Cerebrod_load_module load_module)
{
  DIR *dir;
  int i = 0, found = 0;

  assert(search_dir);
  assert(modules_list);
  assert(modules_list_len > 0);
  assert(load_module);

  if (!(dir = opendir(search_dir)))
    return 0;

  for (i = 0; i < modules_list_len; i++)
    {
      struct dirent *dirent;

      while ((dirent = readdir(dir)))
        {
          if (!strcmp(dirent->d_name, modules_list[i]))
            {
              char filebuf[MAXPATHLEN+1];
              int ret;

              memset(filebuf, '\0', MAXPATHLEN+1);
              snprintf(filebuf, MAXPATHLEN, "%s/%s",
                       search_dir, modules_list[i]);

              if ((ret = load_module(filebuf)) < 0)
                cerebro_err_exit("%s(%s:%d): load_module: %s",
                                 __FILE__, __FUNCTION__, __LINE__,
                                 strerror(errno));

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
  Closedir(dir);

  return (found) ? 1 : 0;
}

int
cerebrod_search_for_module(char *search_dir,
			   char *signature,
			   Cerebrod_load_module load_module)
{
  DIR *dir;
  struct dirent *dirent;
  int found = 0;

  assert(search_dir);
  assert(signature);
  assert(load_module);

  if (!(dir = opendir(search_dir)))
    return 0;

  while ((dirent = readdir(dir)))
    {
      if (strstr(dirent->d_name, signature))
        {
          char filebuf[MAXPATHLEN+1];
          char *ptr;
          int ret;

	  /* 
	   * Don't bother trying to load this file unless its a shared
	   * object file or libtool file.
	   */
          ptr = strchr(dirent->d_name, '.');
          if (!(!strcmp(ptr, ".la") || !strcmp(ptr, ".so")))
            continue;

          memset(filebuf, '\0', MAXPATHLEN+1);
          snprintf(filebuf, MAXPATHLEN, "%s/%s",
                   search_dir, dirent->d_name);
          
          if ((ret = load_module(filebuf)) < 0)
            cerebro_err_exit("%s(%s:%d): load_module: %s",
                             __FILE__, __FUNCTION__, __LINE__,
                             strerror(errno));

          if (ret)
            {
              found++;
              goto done;
            }
        }
    }
  
 done:
  Closedir(dir);

  return (found) ? 1 : 0;
}

#endif /* !WITH_STATIC_MODULES */
