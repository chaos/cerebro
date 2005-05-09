/*****************************************************************************\
 *  $Id: cerebrod_clusterlist.c,v 1.38 2005-05-09 16:02:11 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>

#include "cerebro_module.h"
#include "cerebro/cerebro_error.h"

#include "cerebrod.h"
#include "cerebrod_clusterlist.h"
#include "cerebrod_config.h"

extern struct cerebrod_config conf;

int
cerebrod_clusterlist_module_setup(void)
{
  if (cerebro_module_load_clusterlist_module() < 0)
    cerebro_err_exit("%s(%s:%d): cerebro_module_load_clusterlist_module",
		     __FILE__, __FUNCTION__, __LINE__);

  if (cerebro_clusterlist_module_setup() < 0)
    cerebro_err_exit("%s(%s:%d): cerebro_clusterlist_module_setup",
		     __FILE__, __FUNCTION__, __LINE__);

#if CEREBRO_DEBUG
  if (conf.debug)
    {
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Cerebro Clusterlist Configuration:\n");
      fprintf(stderr, "* -----------------------\n");
      fprintf(stderr, "* Loaded clusterlist module: %s\n", 
	      cerebro_clusterlist_module_name());
      fprintf(stderr, "**************************************\n");
    }
#endif /* CEREBRO_DEBUG */
  return 0;
}
