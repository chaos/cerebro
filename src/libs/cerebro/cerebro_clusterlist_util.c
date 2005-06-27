/*****************************************************************************\
 *  $Id: cerebro_clusterlist_util.c,v 1.3 2005-06-27 17:59:45 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>

#include "cerebro.h"
#include "cerebro_api.h"
#include "cerebro_clusterlist_util.h"
#include "cerebro_util.h"

#include "clusterlist_module.h"

int 
_cerebro_load_clusterlist_module(cerebro_t handle)
{
  if (_cerebro_handle_check(handle) < 0)
    return -1;

  if (handle->loaded_state & CEREBRO_CLUSTERLIST_MODULE_LOADED)
    return 0;

  if (!(handle->clusterlist_handle = clusterlist_module_load()))
    {
      handle->errnum = CEREBRO_ERR_CLUSTERLIST_MODULE;
      goto cleanup;
    }
  
  if (clusterlist_module_setup(handle->clusterlist_handle) < 0)
    {
      handle->errnum = CEREBRO_ERR_CLUSTERLIST_MODULE;
      goto cleanup;
    }
  
  handle->loaded_state |= CEREBRO_CLUSTERLIST_MODULE_LOADED;
  return 0;

 cleanup:
  clusterlist_module_unload(handle->clusterlist_handle);
  handle->clusterlist_handle = NULL;
  return -1;
}

int 
_cerebro_unload_clusterlist_module(cerebro_t handle)
{
  if (_cerebro_handle_check(handle) < 0)
    return -1;

  if (handle->loaded_state & CEREBRO_CLUSTERLIST_MODULE_LOADED)
    {
      if (clusterlist_module_cleanup(handle->clusterlist_handle) < 0)
        {
	  handle->errnum = CEREBRO_ERR_CLUSTERLIST_MODULE;
	  return -1;
        }
      
      if (clusterlist_module_unload(handle->clusterlist_handle) < 0)
	{
	  handle->errnum = CEREBRO_ERR_CLUSTERLIST_MODULE;
	  return -1;
	}
    }

  handle->loaded_state &= ~CEREBRO_CLUSTERLIST_MODULE_LOADED;
  handle->clusterlist_handle = NULL;
  return 0;
}
