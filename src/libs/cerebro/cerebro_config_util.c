/*****************************************************************************\
 *  $Id: cerebro_config_util.c,v 1.7 2005-06-29 22:43:47 achu Exp $
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
  unsigned int errnum;

  if (_cerebro_handle_check(handle) < 0)
    return -1;

  if (handle->loaded_state & CEREBRO_CONFIG_LOADED)
    return 0;
  
  memset(&(handle->config_data), '\0', sizeof(struct cerebro_config));
  if (load_config(&(handle->config_data), &errnum) < 0)
    {
      CEREBRO_DBG(("merge_cerebro_configs"));
      handle->errnum = errnum;
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
