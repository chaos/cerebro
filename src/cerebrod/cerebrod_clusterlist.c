/*****************************************************************************\
 *  $Id: cerebrod_clusterlist.c,v 1.34 2005-05-04 01:00:55 achu Exp $
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
#include "wrappers.h"

extern struct cerebrod_config conf;

int
cerebrod_clusterlist_module_setup(void)
{
  if (cerebro_module_load_clusterlist_module())
    cerebro_err_exit("%s(%s:%d): cerebro_module_load_clusterlist_module",
		     __FILE__, __FUNCTION__, __LINE__);

  if (cerebro_clusterlist_module_setup() < 0)
    cerebro_err_exit("%s(%s:%d): cerebro_clusterlist_module_setup",
		     __FILE__, __FUNCTION__, __LINE__);

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
  return 0;
}
