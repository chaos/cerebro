/*****************************************************************************\
 *  $Id: cerebro_module_util.c,v 1.1 2005-06-17 16:59:30 achu Exp $
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
#include "cerebro_module_util.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_error.h"

#include "ltdl.h"

/*  
 * cerebro_module_library_setup_count
 *
 * indicates the number of times module setup function has been called
 */
int cerebro_module_library_setup_count = 0;

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

  if (!handle)
    {
      cerebro_err_debug("%s(%s:%d): handle null", 
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

int 
_cerebro_module_find_modules(char *search_dir,
                             char *signature,
                             Cerebro_load_module load_module,
                             void *handle,
                             unsigned modules_max)
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

  if (!handle)
    {
      cerebro_err_debug("%s(%s:%d): handle null", 
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!modules_max)
    {
      cerebro_err_debug("%s(%s:%d): modules_max null", 
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
           * Don't bother trying to load unless its a shared object
           * file
           */
          ptr = strchr(dirent->d_name, '.');
          if (!ptr || strcmp(ptr, ".so"))
            continue;

          memset(filebuf, '\0', CEREBRO_MAXPATHLEN+1);
          snprintf(filebuf, CEREBRO_MAXPATHLEN, "%s/%s",
                   search_dir, dirent->d_name);

          if ((flag = load_module(handle, filebuf)) < 0)
	    return -1;

          if (flag)
            found++;

          if (found >= modules_max)
            goto out;
        }
    }

 out:
  closedir(dir);
  return (found) ? 1 : 0;
}

int 
_cerebro_module_setup(void)
{
  if (cerebro_module_library_setup_count)
    goto out;

  if (lt_dlinit() != 0)
    {
      cerebro_err_debug("%s(%s:%d): lt_dlinit: %s", 
			__FILE__, __FUNCTION__, __LINE__, 
			lt_dlerror());
      return -1;
    }

 out:
  cerebro_module_library_setup_count++;
  return 0;
}

int 
_cerebro_module_cleanup(void)
{
  if (cerebro_module_library_setup_count)
    cerebro_module_library_setup_count--;

  if (!cerebro_module_library_setup_count)
    {
      if (lt_dlexit() != 0)
        {
          cerebro_err_debug("%s(%s:%d): lt_dlexit: %s", 
			    __FILE__, __FUNCTION__, __LINE__, 
			    lt_dlerror());
          return -1;
        }
    }

  return 0;
}
