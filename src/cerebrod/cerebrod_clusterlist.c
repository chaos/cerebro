/*****************************************************************************\
 *  $Id: cerebrod_clusterlist.c,v 1.28 2005-04-29 17:12:04 achu Exp $
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
#include "cerebro_module.h"

#include "cerebrod.h"
#include "cerebrod_clusterlist.h"
#include "cerebrod_config.h"
#if WITH_STATIC_MODULES
#include "cerebrod_static_modules.h"
#endif /* WITH_STATIC_MODULES */
#include "cerebrod_util.h"
#include "wrappers.h"

extern struct cerebrod_config conf;

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
#endif /* WITH_STATIC_MODULES */

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
  if (conf.clusterlist_module_file)
    {
      int rv;

      if ((rv = cerebro_load_clusterlist_module(conf.clusterlist_module_file)) < 0)
        cerebro_err_exit("%s(%s:%d): cerebro_load_clusterlist_module: %s",
                         __FILE__, __FUNCTION__, __LINE__, strerror(errno));

	if (!rv)
	  cerebro_err_exit("clusterlist module '%s' could not be loaded", 
			   conf.clusterlist_module_file);
    }
  else
    {
      int rv;

      if ((rv = cerebro_find_clusterlist_module()) < 0)
	cerebro_err_exit("%s(%s:%d): cerebro_find_clusterlist_module: %s",
                         __FILE__, __FUNCTION__, __LINE__, strerror(errno));
      if (!rv)
        cerebro_err_exit("no loadable clusterlist modules found");
    }

#ifndef NDEBUG
  if (conf.debug)
    {
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Cerebro Clusterlist Configuration:\n");
      fprintf(stderr, "* -----------------------\n");
      fprintf(stderr, "* Loaded clusterlist module: %s\n", 
	      cerebro_clusterlist_module_name());
      fprintf(stderr, "**************************************\n");
    }
#endif /* NDEBUG */

#endif /* !WITH_STATIC_MODULES */
  return 0;
}
