/*****************************************************************************\
 *  $Id: cerebrod_clusterlist.c,v 1.22 2005-03-30 18:26:02 achu Exp $
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

#include "cerebrod_clusterlist_module.h"

#include "cerebrod.h"
#include "cerebrod_clusterlist.h"
#include "cerebrod_config.h"
#include "cerebrod_error.h"
#if WITH_STATIC_MODULES
#include "cerebrod_static_modules.h"
#else /* !WITH_STATIC_MODULES */
#include "cerebrod_dynamic_modules.h"
#endif /* !WITH_STATIC_MODULES */
#include "cerebrod_util.h"
#include "wrappers.h"

extern struct cerebrod_config conf;

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
  assert(clusterlist_module_info_l);

  if (!clusterlist_module_info_l->parse_options)
    cerebrod_err_exit("clusterlist module '%s' does not contain "
		      "valid parse_options function",
		      clusterlist_module_info_l->clusterlist_module_name);

  if (!clusterlist_module_info_l->init)
    cerebrod_err_exit("clusterlist module '%s' does not contain "
		      "valid init function",
		      clusterlist_module_info_l->clusterlist_module_name);
  
  if (!clusterlist_module_info_l->finish)
    cerebrod_err_exit("clusterlist module '%s' does not contain "
		      "valid finish function",
		      clusterlist_module_info_l->clusterlist_module_name);

  if (!clusterlist_module_info_l->get_all_nodes)
    cerebrod_err_exit("clusterlist module '%s' does not contain "
		      "valid get_all_nodes function",
		      clusterlist_module_info_l->clusterlist_module_name);

  if (!clusterlist_module_info_l->numnodes)
    cerebrod_err_exit("clusterlist module '%s' does not contain "
		      "valid numnodes function",
		      clusterlist_module_info_l->clusterlist_module_name);

  if (!clusterlist_module_info_l->node_in_cluster)
    cerebrod_err_exit("clusterlist module '%s' does not contain "
		      "valid node_in_cluster function",
		      clusterlist_module_info_l->clusterlist_module_name);
  
  if (!clusterlist_module_info_l->get_nodename)
    cerebrod_err_exit("clusterlist module '%s' does not contain "
		      "valid get_nodename function",
		      clusterlist_module_info_l->clusterlist_module_name);

#ifndef NDEBUG
  if (conf.debug)
    {
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Cerebrod Clusterlist Configuration:\n");
      fprintf(stderr, "* -----------------------\n");
      fprintf(stderr, "* Loaded clusterlist module: %s\n", 
	      clusterlist_module_info_l->clusterlist_module_name);
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
  assert(name);

  if (!(clusterlist_module_info = cerebrod_find_static_clusterlist_module(name)))
    cerebrod_err_exit("clusterlist module '%s' not found", name);

  return _clusterlist_check_module_data(clusterlist_module_info);
}
#else  /* !WITH_STATIC_MODULES */
/*
 * _clusterlist_load_dynamic_module
 *
 * load a clusterlist module and all appropriate symbols
 *
 * - module_path - full path to clusterlist module to load
 *
 * Returns 1 on loading success, 0 on loading failure, -1 on fatal error
 */
static int
_clusterlist_load_dynamic_module(char *module_path)
{
  assert(module_path);

  clusterlist_module_dl_handle = Lt_dlopen(module_path);

  clusterlist_module_info = (struct cerebrod_clusterlist_module_info *)Lt_dlsym(clusterlist_module_dl_handle, "clusterlist_module_info");

  if (!clusterlist_module_info->clusterlist_module_name)
    cerebrod_err_exit("clusterlist module '%s' does not contain a valid name", 
		      module_path);

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
        cerebrod_err_exit("clusterlist module '%s' could not be loaded",
			  conf.clusterlist_module);
    }
  else
    {
      struct cerebrod_clusterlist_module_info **ptr;
      int i = 0;

      ptr = &static_clusterlist_modules[0];
      while (ptr[i] != NULL)
        {
          int rv;

          if (!ptr[i]->clusterlist_module_name)
            {
              cerebrod_err_debug("static clusterlist module index '%d' "
				 "does not contain name", i);
              continue;
            }

          if ((rv = _clusterlist_check_module_data(ptr[i])) < 0)
            cerebrod_err_exit("clusterlist module '%s' could not be loaded",
			      ptr[i]->clusterlist_module_name);
          if (rv) 
            {
              clusterlist_module_info = ptr[i];
              break;
            }
          i++;
        }
    }
#else /* !WITH_STATIC_MODULES */
  Lt_dlinit();

  if (conf.clusterlist_module_file)
    {
      if (_clusterlist_load_dynamic_module(conf.clusterlist_module_file) != 1)
        cerebrod_err_exit("clusterlist module '%s' could not be loaded", 
			  conf.clusterlist_module_file);
    }
  else
    {
      if (cerebrod_search_dir_for_module(CEREBROD_MODULE_DIR,
                                         dynamic_clusterlist_modules,
					 dynamic_clusterlist_modules_len,
                                         _clusterlist_load_dynamic_module))
        goto done;

      if (cerebrod_search_dir_for_module(".",
                                         dynamic_clusterlist_modules,
					 dynamic_clusterlist_modules_len,
                                         _clusterlist_load_dynamic_module))
        goto done;

      if (!clusterlist_module_dl_handle)
        cerebrod_err_exit("no valid cluster list modules found");
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
  Lt_dlexit();
  clusterlist_module_dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */

  clusterlist_module_info = NULL;

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
