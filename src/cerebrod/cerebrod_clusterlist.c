/*****************************************************************************\
 *  $Id: cerebrod_clusterlist.c,v 1.17 2005-03-22 07:27:30 achu Exp $
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
#if WITH_STATIC_MODULES
#include "cerebrod_static_modules.h"
#endif /* WITH_STATIC_MODULES */
#include "cerebrod_util.h"
#include "error.h"
#include "wrappers.h"

extern struct cerebrod_config conf;

#if WITH_STATIC_MODULES
/* 
 * static_clusterlist_modules
 *
 * clusterlist modules statically compiled in
 */
struct cerebrod_clusterlist_module_info *static_clusterlist_modules[] =
  {
#if WITH_GENDERSLLNL
    &gendersllnl_clusterlist_module_info,
#endif
#if WITH_GENDERS
    &genders_clusterlist_module_info,
#endif
    &hostsfile_clusterlist_module_info,
    &none_clusterlist_module_info,
    NULL
  }; 
#else /* !WITH_STATIC_MODULES */
/*
 * clusterlist_modules
 * clusterlist_modules_len
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
#endif /* !WITH_STATIC_MODULES */

/* 
 * clusterlist_module_info
 *
 * clusterlist module info and operations
 */
static struct cerebrod_clusterlist_module_info *clusterlist_module_info = NULL;

#if !WITH_STATIC_MODULES
/* 
 * clusterlist_module_dl_handle
 *
 * clusterlist module dynamically loaded module handle
 */
static lt_dlhandle clusterlist_module_dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */

/*
 * _clusterlist_check_module_data
 *
 * Check parameters and load the actual module data
 *
 * Returns 1 on loading success, 0 on loading failure, -1 on fatal error
 */
static int
_clusterlist_check_module_data(struct cerebrod_clusterlist_module_info *clusterlist_module_info_l)
{
  assert(clusterlist_module_info);

  if (!clusterlist_module_info_l->parse_options)
    err_exit("clusterlist module '%s' does not contain valid parse_options function",
	     clusterlist_module_info->clusterlist_module_name);

  if (!clusterlist_module_info_l->init)
    err_exit("clusterlist module '%s' does not contain valid init function",
	     clusterlist_module_info->clusterlist_module_name);

  if (!clusterlist_module_info_l->finish)
    err_exit("clusterlist module '%s' does not contain valid finish function",
	     clusterlist_module_info->clusterlist_module_name);

  if (!clusterlist_module_info_l->get_all_nodes)
    err_exit("clusterlist module '%s' does not contain valid get_all_nodes function",
	     clusterlist_module_info->clusterlist_module_name);

  if (!clusterlist_module_info_l->numnodes)
    err_exit("clusterlist module '%s' does not contain valid numnodes function",
	     clusterlist_module_info->clusterlist_module_name);

  if (!clusterlist_module_info_l->node_in_cluster)
    err_exit("clusterlist module '%s' does not contain valid node_in_cluster function",
	     clusterlist_module_info->clusterlist_module_name);

  if (!clusterlist_module_info_l->get_nodename)
    err_exit("clusterlist module '%s' does not contain valid get_nodename function",
    	     clusterlist_module_info->clusterlist_module_name);
                                                                                         
#ifndef NDEBUG
  if (conf.debug)
    {
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Cerebrod Clusterlist Configuration:\n");
      fprintf(stderr, "* -----------------------\n");
      fprintf(stderr, "* Loaded clusterlist module: %s\n", name);
      fprintf(stderr, "**************************************\n");
    }
#endif /* NDEBUG */

  return 1;
}

#if WITH_STATIC_MODULES
/*
 * _clusterlist_load_static_module
 *
 * load a clusterlist module
 *
 * - type - module type
 *
 * Returns 1 on loading success, 0 on loading failure, -1 on fatal error
 */
static int
_clusterlist_load_static_module(char *name)
{
  struct cerebrod_clusterlist_module_info **ptr = &static_clusterlist_modules[0];
  int i = 0;

  assert(name);

  while (ptr[i] != NULL)
    {
      if (!ptr[i]->clusterlist_module_name)
        {
          err_debug("static clusterlist module index '%d' does not contain name", i);
          continue;
        }
      if (!strcmp(ptr[i]->clusterlist_module_name, name))
        {
          clusterlist_module_info = ptr[i];
          break;
        }
      i++;
    }
  
  return _clusterlist_check_module_data(ptr[i]);
}
#else  /* !WITH_STATIC_MODULES */
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

  if (!clusterlist_module_info->clusterlist_module_name)
    err_exit("clusterlist module '%s' does not contain a valid name");

  return _clusterlist_check_module_data(clusterlist_module_info);
}
#endif /* !WITH_STATIC_MODULES */

int
cerebrod_clusterlist_setup(void)
{
  assert(!clusterlist_module_info);

#if WITH_STATIC_MODULES
  if (conf.clusterlist_module)
    {
      if (_clusterlist_load_static_module(conf.clusterlist_module) != 1)
        err_exit("clusterlist module '%s' could not be loaded",
                 conf.clusterlist_module);
    }
  else
    {
      struct cerebrod_clusterlist_module_info **ptr = &static_clusterlist_modules[0];
      int i = 0;
                                                                                         
      while (ptr[i] != NULL)
        {
          int rv;
                                                                                         
          if ((rv = _clusterlist_load_static_module(ptr[i]->clusterlist_module_name)) < 0)
            err_exit("clusterlist module '%s' could not be loaded",
                     ptr[i]->clusterlist_module_name);
          if (rv)
            break;
          i++;
        }
    }
#else /* !WITH_STATIC_MODULES */
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
#endif /* !WITH_STATIC_MODULES */
  return 0;
}

int 
cerebrod_clusterlist_cleanup(void)
{
  assert(clusterlist_module_info);

#if !WITH_STATIC_MODULES
  Lt_dlclose(clusterlist_module_dl_handle);
  clusterlist_module_dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */
  clusterlist_module_info = NULL;

#if !WITH_STATIC_MODULES
  Lt_dlexit();
#endif /* !WITH_STATIC_MODULES */

  return 0;
}

char *
cerebrod_clusterlist_module_name(void)
{
  assert(clusterlist_module_info);

  return clusterlist_module_info->clusterlist_module_name;
}

int 
cerebrod_clusterlist_parse_options(void)
{
  assert(clusterlist_module_info);

  return ((*clusterlist_module_info->parse_options)(conf.clusterlist_module_options));
}

int 
cerebrod_clusterlist_init(void)
{
  assert(clusterlist_module_info);

  return ((*clusterlist_module_info->init)());
}

int 
cerebrod_clusterlist_finish(void)
{
  assert(clusterlist_module_info);

  return ((*clusterlist_module_info->finish)());
}

int 
cerebrod_clusterlist_get_all_nodes(char **nodes, unsigned int nodeslen)
{
  assert(clusterlist_module_info && nodes);

  return ((*clusterlist_module_info->get_all_nodes)(nodes, nodeslen));
}

int 
cerebrod_clusterlist_numnodes(void)
{
  assert(clusterlist_module_info);

  return ((*clusterlist_module_info->numnodes)());
}

int 
cerebrod_clusterlist_node_in_cluster(char *node)
{
  assert(clusterlist_module_info);

  return ((*clusterlist_module_info->node_in_cluster)(node));
}

int 
cerebrod_clusterlist_get_nodename(char *node, char *buf, unsigned int buflen)
{
  assert(clusterlist_module_info);

  return ((*clusterlist_module_info->get_nodename)(node, buf, buflen));
}
