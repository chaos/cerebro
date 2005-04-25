/*****************************************************************************\
 *  $Id: cerebrod_clusterlist.c,v 1.27 2005-04-25 15:33:05 achu Exp $
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

#include "cerebro_error.h"
#include "cerebro_clusterlist_module.h"

#include "cerebrod.h"
#include "cerebrod_clusterlist.h"
#include "cerebrod_config.h"
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
static struct cerebro_clusterlist_module_info *clusterlist_module_info = NULL;

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
_clusterlist_check_module_data(struct cerebro_clusterlist_module_info *clusterlist_module_info_l)
{
  assert(clusterlist_module_info_l);

  if (!clusterlist_module_info_l->parse_options)
    {
      cerebro_err_debug("clusterlist module '%s' does not contain "
                        "valid parse_options function",
                        clusterlist_module_info_l->clusterlist_module_name);
      return 0;
    }

  if (!clusterlist_module_info_l->setup)
    {
      cerebro_err_debug("clusterlist module '%s' does not contain "
                        "valid setup function",
                        clusterlist_module_info_l->clusterlist_module_name);
      return 0;
    }
  
  if (!clusterlist_module_info_l->cleanup)
    {
      cerebro_err_debug("clusterlist module '%s' does not contain "
                        "valid cleanup function",
                        clusterlist_module_info_l->clusterlist_module_name);
      return 0;
    }

  if (!clusterlist_module_info_l->get_all_nodes)
    {
      cerebro_err_debug("clusterlist module '%s' does not contain "
                        "valid get_all_nodes function",
                        clusterlist_module_info_l->clusterlist_module_name);
      return 0;
    }

  if (!clusterlist_module_info_l->numnodes)
    {
      cerebro_err_debug("clusterlist module '%s' does not contain "
                        "valid numnodes function",
                        clusterlist_module_info_l->clusterlist_module_name);
      return 0;
    }
  
  if (!clusterlist_module_info_l->node_in_cluster)
    {
      cerebro_err_debug("clusterlist module '%s' does not contain "
                        "valid node_in_cluster function",
                        clusterlist_module_info_l->clusterlist_module_name);
      return 0;
    }
  
  if (!clusterlist_module_info_l->get_nodename)
    {
      cerebro_err_debug("clusterlist module '%s' does not contain "
                        "valid get_nodename function",
                        clusterlist_module_info_l->clusterlist_module_name);
      return 0;
    }

#ifndef NDEBUG
  if (conf.debug)
    {
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Cerebro Clusterlist Configuration:\n");
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
    cerebro_err_exit("clusterlist module '%s' not found", name);

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
  int rv;

  assert(module_path);

  if (!(clusterlist_module_dl_handle = lt_dlopen(module_path)))
    {
      cerebro_err_debug("clusterlist module '%s': lt_dlopen failure: %s", 
                        lt_dlerror());
      return 0;
    }

  /* clear lt_dlerror */
  lt_dlerror();

  if (!(clusterlist_module_info = (struct cerebro_clusterlist_module_info *)lt_dlsym(clusterlist_module_dl_handle, "clusterlist_module_info")))
    {
      const char *err = lt_dlerror();
      if (err != NULL)
        cerebro_err_debug("clusterlist module '%s': lt_dlsym failure:: %s", err);
      return 0;
    }

  if (!clusterlist_module_info->clusterlist_module_name)
    {
      cerebro_err_debug("clusterlist module '%s' does not contain a valid name", 
                        module_path);
      return 0;
    }

  if ((rv = _clusterlist_check_module_data(clusterlist_module_info)) <= 0)
    {
      lt_dlclose(clusterlist_module_dl_handle);
      clusterlist_module_dl_handle = NULL;
      clusterlist_module_info = NULL;
    }

  return rv;
}
#endif /* !WITH_STATIC_MODULES */

int
cerebrod_clusterlist_module_setup(void)
{
  assert(!clusterlist_module_info);

#if WITH_STATIC_MODULES
  if (conf.clusterlist_module)
    {
      if (_clusterlist_load_static_module(conf.clusterlist_module) != 1)
        cerebro_err_exit("clusterlist module '%s' could not be loaded",
                         conf.clusterlist_module);
    }
  else
    {
      struct cerebro_clusterlist_module_info **ptr;
      int i = 0;

      ptr = &static_clusterlist_modules[0];
      while (ptr[i] != NULL)
        {
          int rv;

          if (!ptr[i]->clusterlist_module_name)
            {
              cerebro_err_debug("static clusterlist module index '%d' "
                                "does not contain name", i);
              continue;
            }

          if ((rv = _clusterlist_check_module_data(ptr[i])) < 0)
            cerebro_err_exit("clusterlist module '%s' could not be loaded",
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
        cerebro_err_exit("clusterlist module '%s' could not be loaded", 
                         conf.clusterlist_module_file);
    }
  else
    {
      if (cerebrod_lookup_module(CEREBRO_CLUSTERLIST_MODULE_BUILDDIR,
				 dynamic_clusterlist_modules,
				 dynamic_clusterlist_modules_len,
				 _clusterlist_load_dynamic_module))
        goto done;

      if (cerebrod_lookup_module(CEREBRO_MODULE_DIR,
				 dynamic_clusterlist_modules,
				 dynamic_clusterlist_modules_len,
				 _clusterlist_load_dynamic_module))
        goto done;

      if (cerebrod_search_for_module(CEREBRO_MODULE_DIR,
				     "cerebro_clusterlist_",
				     _clusterlist_load_dynamic_module))
        goto done;

      if (!clusterlist_module_dl_handle)
        cerebro_err_exit("no valid cluster list modules found");
    }

 done:
#endif /* !WITH_STATIC_MODULES */
  return 0;
}

int 
cerebrod_clusterlist_module_cleanup(void)
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
cerebrod_clusterlist_setup(void)
{
  assert(clusterlist_module_info);

  return ((*clusterlist_module_info->setup)());
}

int 
cerebrod_clusterlist_cleanup(void)
{
  assert(clusterlist_module_info);

  return ((*clusterlist_module_info->cleanup)());
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
