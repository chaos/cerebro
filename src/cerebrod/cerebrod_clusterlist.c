/*****************************************************************************\
 *  $Id: cerebrod_clusterlist.c,v 1.43 2005-06-18 18:48:30 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>

#include "cerebro/cerebro_error.h"

#include "cerebrod.h"
#include "cerebrod_clusterlist.h"
#include "cerebrod_config.h"

#include "clusterlist_module.h"

extern struct cerebrod_config conf;

/* 
 * clusterlist_handle
 *
 * Handle for clusterlist module
 */
clusterlist_module_t clusterlist_handle;

int
cerebrod_clusterlist_module_setup(void)
{
  if (!(clusterlist_handle = clusterlist_module_load()))
    cerebro_err_exit("%s(%s:%d): clusterlist_module_load",
		     __FILE__, __FUNCTION__, __LINE__);
  
  if (clusterlist_module_setup(clusterlist_handle) < 0)
    cerebro_err_exit("%s(%s:%d): clusterlist_module_setup",
		     __FILE__, __FUNCTION__, __LINE__);

#if CEREBRO_DEBUG
  if (conf.debug)
    {
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Cerebro Clusterlist Configuration:\n");
      fprintf(stderr, "* -----------------------\n");
      fprintf(stderr, "* Loaded clusterlist module: %s\n", 
	      clusterlist_module_name(clusterlist_handle));
      fprintf(stderr, "**************************************\n");
    }
#endif /* CEREBRO_DEBUG */
  return 0;
}
