/*****************************************************************************\
 *  $Id: cerebro_clusterlist_util.c,v 1.1 2005-06-17 20:54:08 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>

#include "cerebro.h"
#include "cerebro_api.h"
#include "cerebro_clusterlist_util.h"
#include "cerebro_module_clusterlist.h"
#include "cerebro_util.h"

int 
_cerebro_load_clusterlist_module(cerebro_t handle)
{
  if (_cerebro_handle_check(handle) < 0)
    return -1;

  if (handle->loaded_state & CEREBRO_CLUSTERLIST_MODULE_LOADED)
    return 0;

  if (!(handle->clusterlist_handle = _cerebro_module_load_clusterlist_module()))
    {
      handle->errnum = CEREBRO_ERR_CLUSTERLIST_MODULE;
      goto cleanup;
    }
  
  if (_cerebro_clusterlist_module_setup(handle->clusterlist_handle) < 0)
    {
      handle->errnum = CEREBRO_ERR_CLUSTERLIST_MODULE;
      goto cleanup;
    }
  
  handle->loaded_state |= CEREBRO_CLUSTERLIST_MODULE_LOADED;
  return 0;

 cleanup:
  _cerebro_module_destroy_clusterlist_handle(handle->clusterlist_handle);
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
      if (_cerebro_clusterlist_module_cleanup(handle->clusterlist_handle) < 0)
        {
	  handle->errnum = CEREBRO_ERR_CLUSTERLIST_MODULE;
	  return -1;
        }
      
      if (_cerebro_module_destroy_clusterlist_handle(handle->clusterlist_handle) < 0)
	{
	  handle->errnum = CEREBRO_ERR_CLUSTERLIST_MODULE;
	  return -1;
	}
    }

  handle->loaded_state &= ~CEREBRO_CLUSTERLIST_MODULE_LOADED;
  handle->clusterlist_handle = NULL;
  handle->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}
