/*****************************************************************************\
 *  $Id: cerebrod_clusterlist.c,v 1.12 2005-03-20 21:50:40 achu Exp $
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
#include "cerebrod_util.h"
#include "error.h"
#include "wrappers.h"

extern struct cerebrod_config conf;

/*
 * cluster_modules
 * cluster_modules_len
 *
 * clusterlist modules to search for by default
 */
char *clusterlist_modules[] = {
  "cerebrod_clusterlist_gendersllnl.la",
  "cerebrod_clusterlist_genders.la",
  "cerebrod_clusterlist_none.la",
  "cerebrod_clusterlist_hostfile.la",
  NULL
};
int clusterlist_modules_len = 4;

/* 
 * clusterlist_module_info
 * clusterlist_module_ops
 *
 * clusterlist module info and operations pointers
 */
static struct cerebrod_clusterlist_module_info *clusterlist_module_info = NULL;
static struct cerebrod_clusterlist_module_ops *clusterlist_module_ops = NULL;

/* 
 * clusterlist_module_dl_handle
 *
 * clusterlist module dynamically loaded module handle
 */
static lt_dlhandle clusterlist_module_dl_handle = NULL;


/*
 * _clusterlist_load_module
 *
 * load a clusterlist module and all appropriate symbols
 *
 * - module_path - full path to clusterlist module to load
 *
 * Returns 1 on loading success, 0 on loading failure, -1 on fatal error
 */
static int
_clusterlist_load_module(char *module_path)
{
  assert(module_path);

  clusterlist_module_dl_handle = Lt_dlopen(module_path);
  clusterlist_module_info = (struct cerebrod_clusterlist_module_info *)Lt_dlsym(clusterlist_module_dl_handle, "clusterlist_module_info");
  clusterlist_module_ops = (struct cerebrod_clusterlist_module_ops *)Lt_dlsym(clusterlist_module_dl_handle, "clusterlist_module_ops");

  if (!clusterlist_module_info->clusterlist_module_name)
    err_exit("clusterlist module '%s' does not contain a valid name");

  if (!clusterlist_module_ops->parse_options)
    err_exit("clusterlist module '%s' does not contain valid parse_options function");

  if (!clusterlist_module_ops->init)
    err_exit("clusterlist module '%s' does not contain valid init function");

  if (!clusterlist_module_ops->finish)
    err_exit("clusterlist module '%s' does not contain valid finish function");

  if (!clusterlist_module_ops->get_all_nodes)
    err_exit("clusterlist module '%s' does not contain valid get_all_nodes function");

  if (!clusterlist_module_ops->numnodes)
    err_exit("clusterlist module '%s' does not contain valid numnodes function");

  if (!clusterlist_module_ops->node_in_cluster)
    err_exit("clusterlist module '%s' does not contain valid node_in_cluster function");

  if (!clusterlist_module_ops->get_nodename)
    err_exit("clusterlist module '%s' does not contain valid get_nodename function");
  
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

  return 1;
}

int
cerebrod_clusterlist_setup(void)
{
  assert(!clusterlist_module_dl_handle);

  Lt_dlinit();

  if (conf.clusterlist_module_file)
    {
      if (_clusterlist_load_module(conf.clusterlist_module_file) != 1)
        err_exit("clusterlist module '%s' could not be loaded", 
                 conf.clusterlist_module_file);
    }
  else
    {
      if (cerebrod_search_dir_for_module(CEREBROD_MODULE_DIR,
                                         clusterlist_modules,
					 clusterlist_modules_len,
                                         _clusterlist_load_module))
        goto done;

      if (cerebrod_search_dir_for_module(".",
                                         clusterlist_modules,
					 clusterlist_modules_len,
                                         _clusterlist_load_module))
        goto done;

      if (!clusterlist_module_dl_handle)
        err_exit("no valid cluster list modules found");
    }

 done:
  return 0;
}

int 
cerebrod_clusterlist_cleanup(void)
{
  assert(!clusterlist_module_dl_handle);

  Lt_dlclose(clusterlist_module_dl_handle);
  clusterlist_module_dl_handle = NULL;
  clusterlist_module_ops = NULL;

  Lt_dlexit();

  return 0;
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
