/*****************************************************************************\
 *  $Id: cerebro_config_util.c,v 1.6 2005-06-28 21:26:52 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */

#include "cerebro_api.h"
#include "cerebro_config_util.h"
#include "cerebro_util.h"
#include "cerebro/cerebro_config.h"

#include "conffile.h"
#include "config_module.h"
#include "config_util.h"
#include "debug.h"

int 
_cerebro_load_config(cerebro_t handle)
{
  struct cerebro_config module_conf;
  struct cerebro_config config_file_conf;
  
  if (_cerebro_handle_check(handle) < 0)
    return -1;

  if (handle->loaded_state & CEREBRO_CONFIG_LOADED)
    return 0;
  
  memset(&module_conf, '\0', sizeof(struct cerebro_config));
  if (load_config_module(&module_conf) < 0)
    {
      handle->errnum = CEREBRO_ERR_CONFIG_MODULE;
      return -1;
    }

  memset(&config_file_conf, '\0', sizeof(struct cerebro_config));
  if (load_config_file(&config_file_conf) < 0)
    {
      handle->errnum = CEREBRO_ERR_CONFIG_FILE;
      return -1;
    }

  memset(&(handle->config_data), '\0', sizeof(struct cerebro_config));
  if (merge_cerebro_configs(&(handle->config_data), 
			    &module_conf, 
			    &config_file_conf) < 0)
    {
      CEREBRO_DBG(("merge_cerebro_configs"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  
  handle->loaded_state |= CEREBRO_CONFIG_LOADED;
  return 0;
}

int 
_cerebro_unload_config(cerebro_t handle)
{
  if (_cerebro_handle_check(handle) < 0)
    return -1;

  memset(&(handle->config_data), '\0', sizeof(struct cerebro_config));
  
  handle->loaded_state &= ~CEREBRO_CONFIG_LOADED;
  return 0;
}
