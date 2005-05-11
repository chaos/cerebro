/*****************************************************************************\
 *  $Id: cerebro_clusterlist_util.c,v 1.1 2005-05-11 17:06:28 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>

#include "cerebro.h"
#include "cerebro_api.h"
#include "cerebro_clusterlist_util.h"
#include "cerebro_module.h"
#include "cerebro_util.h"

int 
_cerebro_load_clusterlist_module(cerebro_t handle)
{
  int module_setup_called = 0;

  if (_cerebro_handle_check(handle) < 0)
    return -1;

  if (!(handle->loaded_state & CEREBRO_MODULE_SETUP_CALLED))
    {
      if (_cerebro_module_setup() < 0)
	{
	  handle->errnum = CEREBRO_ERR_CLUSTERLIST_MODULE;
	  goto cleanup;
	}
    }

  if (_cerebro_module_load_clusterlist_module() < 0)
    {
      handle->errnum = CEREBRO_ERR_CLUSTERLIST_MODULE;
      goto cleanup;
    }
  
  if (_cerebro_clusterlist_module_setup() < 0)
    {
      handle->errnum = CEREBRO_ERR_CLUSTERLIST_MODULE;
      goto cleanup;
    }
  
  if (module_setup_called)
    handle->loaded_state |= CEREBRO_MODULE_SETUP_CALLED;
  handle->loaded_state |= CEREBRO_CLUSTERLIST_MODULE_LOADED;
  
  return 0;
 cleanup:
  if (module_setup_called)
    _cerebro_module_cleanup();
  
  return -1;
}

int 
_cerebro_unload_clusterlist_module(cerebro_t handle)
{
  if (_cerebro_handle_check(handle) < 0)
    return -1;

  if (handle->loaded_state & CEREBRO_CLUSTERLIST_MODULE_LOADED)
    {
      if (_cerebro_clusterlist_module_cleanup() < 0)
        {
	  handle->errnum = CEREBRO_ERR_CLUSTERLIST_MODULE;
	  return -1;
        }
      
      if (_cerebro_module_unload_clusterlist_module() < 0)
	{
	  handle->errnum = CEREBRO_ERR_CLUSTERLIST_MODULE;
	  return -1;
	}
    }

  handle->loaded_state &= ~CEREBRO_CLUSTERLIST_MODULE_LOADED;
  handle->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}
