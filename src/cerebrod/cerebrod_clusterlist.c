/*****************************************************************************\
 *  $Id: cerebrod_clusterlist.c,v 1.8 2005-03-18 01:36:30 achu Exp $
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
static struct cerebrod_clusterlist_module_info *clusterlist_module_info = NULL;
static struct cerebrod_clusterlist_module_ops *clusterlist_module_ops = NULL;
static lt_dlhandle clusterlist_module_dl_handle = NULL;

static void
_load_module(char *module_path)
{
  assert(module_path);

  clusterlist_module_dl_handle = Lt_dlopen(module_path);
  clusterlist_module_info = (struct cerebrod_clusterlist_module_info *)Lt_dlsym(clusterlist_module_dl_handle, "clusterlist_module_info");
  clusterlist_module_ops = (struct cerebrod_clusterlist_module_ops *)Lt_dlsym(clusterlist_module_dl_handle, "clusterlist_module_ops");

  if (!clusterlist_module_info->clusterlist_module_name)
    err_exit("clusterlist module '%s' does not contain a valid name");
  
#ifndef NDEBUG
  if (conf.debug)
    {
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Cerebrod Clusterlist Configuration:\n");
      fprintf(stderr, "* -----------------------\n");
      fprintf(stderr, "* Loaded clusterlist module: %s\n", module_path); 
      fprintf(stderr, "**************************************\n");
    }
#endif /* NDEBUG */
}

static int
_search_dir(char *search_dir)
{
  DIR *dir;
  int i = 0;

  assert(!clusterlist_module_dl_handle);
  assert(search_dir);

  if (!(dir = opendir(search_dir)))
    return 0;

  while (clusterlist_modules[i] != NULL)
    {
      struct dirent *dirent;

      while ((dirent = readdir(dir)))
	{
	  if (!strcmp(dirent->d_name, clusterlist_modules[i]))
	    {
              char filebuf[MAXPATHLEN+1];

              memset(filebuf, '\0', MAXPATHLEN+1);
              snprintf(filebuf, MAXPATHLEN, "%s/%s",
                       search_dir, clusterlist_modules[i]);

	      _load_module(filebuf);
	      goto found_dir;
	    }
	}
      
      rewinddir(dir);
      i++;
    }
 found_dir:
  Closedir(dir);
  
  return (clusterlist_module_dl_handle) ? 1 : 0;
}

static void
_find_clusterlist_module(void)
{
  assert(!clusterlist_module_dl_handle);

  if (_search_dir(CEREBROD_MODULE_DIR))
    return;

  if (_search_dir("."))
    return;

  if (!clusterlist_module_dl_handle)
    err_exit("no valid cluster list modules found");
}

int 
cerebrod_clusterlist_setup(void)
{
  assert(!clusterlist_module_dl_handle);

  Lt_dlinit();

  if (conf.clusterlist_module_file)
    _load_module(conf.clusterlist_module);
  else
    _find_clusterlist_module();
}

int 
cerebrod_clusterlist_cleanup(void)
{
  assert(!clusterlist_module_dl_handle);

  Lt_dlclose(clusterlist_module_dl_handle);
  clusterlist_module_dl_handle = NULL;
  clusterlist_module_ops = NULL;

  Lt_dlexit();
}

int 
cerebrod_clusterlist_parse_options(void)
{
  assert(clusterlist_module_dl_handle);

  return ((*clusterlist_module_ops->parse_options)(conf.clusterlist_module_options));
}

int 
cerebrod_clusterlist_init(void)
{
  assert(clusterlist_module_dl_handle);

  return ((*clusterlist_module_ops->init)());
}

int 
cerebrod_clusterlist_finish(void)
{
  assert(clusterlist_module_dl_handle);

  return ((*clusterlist_module_ops->finish)());
}

int 
cerebrod_clusterlist_get_all_nodes(char **nodes, unsigned int nodeslen)
{
  assert(clusterlist_module_dl_handle && nodes);

  return ((*clusterlist_module_ops->get_all_nodes)(nodes, nodeslen));
}

int 
cerebrod_clusterlist_numnodes(void)
{
  assert(clusterlist_module_dl_handle);

  return ((*clusterlist_module_ops->numnodes)());
}

int 
cerebrod_clusterlist_node_in_cluster(char *node)
{
  assert(clusterlist_module_dl_handle);

  return ((*clusterlist_module_ops->node_in_cluster)(node));
}

int 
cerebrod_clusterlist_get_nodename(char *node, char *buf, int buflen)
{
  assert(clusterlist_module_dl_handle);

  return ((*clusterlist_module_ops->get_nodename)(node, buf, buflen));
}

char *
cerebrod_clusterlist_module_name(void)
{
  assert(clusterlist_module_dl_handle);

  return clusterlist_module_info->clusterlist_module_name;
}
