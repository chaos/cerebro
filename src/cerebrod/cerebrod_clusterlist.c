/*****************************************************************************\
 *  $Id: cerebrod_clusterlist.c,v 1.4 2005-03-17 05:46:57 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <assert.h>
#include <errno.h>

#include <sys/param.h>

#include "cerebrod.h"
#include "cerebrod_clusterlist.h"
#include "cerebrod_config.h"
#include "error.h"
#include "wrappers.h"

char *clusterlist_modules[] = {
  "cerebrod_clusterlist_gendersllnl.la",
  "cerebrod_clusterlist_genders.la",
  "cerebrod_clusterlist_hostfile.la",
  "cerebrod_clusterlist_none.la",
  NULL
};

extern struct cerebrod_config conf;
static struct cerebrod_clusterlist_ops *clusterlist_ops = NULL;
lt_dlhandle clusterlist_dl_handle = NULL;

static void
_load_module(char *module_path)
{
  char module_path_buf[MAXPATHLEN+1];
  
  assert(module_path);

  if (module_path[0] != '/')
    {
      memset(module_path_buf, '\0', MAXPATHLEN+1);
      snprintf(module_path_buf, MAXPATHLEN, 
	       "%s/%s", 
	       CEREBROD_MODULE_DIR,
	       module_path);
      module_path = &module_path_buf[0];
    }

  clusterlist_dl_handle = Lt_dlopen(module_path);
  clusterlist_ops = (struct cerebrod_clusterlist_ops *)Lt_dlsym(clusterlist_dl_handle, "clusterlist_ops");
}

static void
_find_clusterlist_module(void)
{
  DIR *dir;
  int i = 0;

  assert(!clusterlist_ops && !clusterlist_dl_handle);

  dir = Opendir(CEREBROD_MODULE_DIR);
  while (clusterlist_modules[i] != NULL)
    {
      struct dirent *dirent;

      while ((dirent = readdir(dir)))
	{
	  if (!strcmp(dirent->d_name, clusterlist_modules[i]))
	    {
	      _load_module(clusterlist_modules[i]);
	      Closedir(dir);
	      return;
	    }
	}
      
      rewinddir(dir);
      i++;
    }
  Closedir(dir);

  dir = Opendir(".");
  while (clusterlist_modules[i] != NULL)
    {
      struct dirent *dirent;

      while ((dirent = readdir(dir)))
	{
	  if (!strcmp(dirent->d_name, clusterlist_modules[i]))
	    {
	      _load_module(clusterlist_modules[i]);
	      Closedir(dir);
	      return;
	    }
	}
      
      rewinddir(dir);
      i++;
    }
  Closedir(dir);

  if (!clusterlist_dl_handle)
    err_exit("no valid cluster list modules found");
}

int 
cerebrod_clusterlist_setup(void)
{
  assert(!clusterlist_ops && !clusterlist_dl_handle);

  Lt_dlinit();

  if (conf.clusterlist_module_file)
    _load_module(conf.clusterlist_module);
  else
    _find_clusterlist_module();
}

int 
cerebrod_clusterlist_cleanup(void)
{
  assert(!clusterlist_ops && !clusterlist_dl_handle);

  Lt_dlclose(clusterlist_dl_handle);
  clusterlist_dl_handle = NULL;
  clusterlist_ops = NULL;

  Lt_dlexit();
}

int 
cerebrod_clusterlist_init(void)
{
  assert(clusterlist_ops && clusterlist_dl_handle);

  return ((*clusterlist_ops->init)(conf.clusterlist_module_options));
}

int 
cerebrod_clusterlist_finish(void)
{
  assert(clusterlist_ops && clusterlist_dl_handle);

  return ((*clusterlist_ops->finish)());
}

int 
cerebrod_clusterlist_get_all_nodes(char **nodes, unsigned int nodeslen)
{
  assert(clusterlist_ops && clusterlist_dl_handle && nodes);

  return ((*clusterlist_ops->get_all_nodes)(nodes, nodeslen));
}

int 
cerebrod_clusterlist_numnodes(void)
{
  assert(clusterlist_ops && clusterlist_dl_handle);

  return ((*clusterlist_ops->numnodes)());
}

int 
cerebrod_clusterlist_node_in_cluster(char *node)
{
  assert(clusterlist_ops && clusterlist_dl_handle);

  return ((*clusterlist_ops->node_in_cluster)(node));
}

int 
cerebrod_clusterlist_get_nodename(char *node, char *buf, int buflen)
{
  assert(clusterlist_ops && clusterlist_dl_handle);

  return ((*clusterlist_ops->get_nodename)(node, buf, buflen));
}
