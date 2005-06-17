/*****************************************************************************\
 *  $Id: cerebrod_clusterlist.c,v 1.42 2005-06-17 17:45:39 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>

#include "cerebro_module_clusterlist.h"
#include "cerebro/cerebro_error.h"

#include "cerebrod.h"
#include "cerebrod_clusterlist.h"
#include "cerebrod_config.h"

extern struct cerebrod_config conf;

/* 
 * clusterlist_handle
 *
 * Handle for clusterlist module
 */
cerebro_clusterlist_module_t clusterlist_handle;

int
cerebrod_clusterlist_module_setup(void)
{
  if (!(clusterlist_handle = _cerebro_module_load_clusterlist_module()))
    cerebro_err_exit("%s(%s:%d): _cerebro_module_load_clusterlist_module",
		     __FILE__, __FUNCTION__, __LINE__);
  
  if (_cerebro_clusterlist_module_setup(clusterlist_handle) < 0)
    cerebro_err_exit("%s(%s:%d): _cerebro_clusterlist_module_setup",
		     __FILE__, __FUNCTION__, __LINE__);

#if CEREBRO_DEBUG
  if (conf.debug)
    {
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Cerebro Clusterlist Configuration:\n");
      fprintf(stderr, "* -----------------------\n");
      fprintf(stderr, "* Loaded clusterlist module: %s\n", 
	      _cerebro_clusterlist_module_name(clusterlist_handle));
      fprintf(stderr, "**************************************\n");
    }
#endif /* CEREBRO_DEBUG */
  return 0;
}
