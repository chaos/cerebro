/*****************************************************************************\
 *  $Id: cerebro_api.c,v 1.10 2005-05-03 22:46:34 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */

#include "cerebro.h"
#include "cerebro_api.h"
#include "cerebro_config.h"
#include "cerebro_module.h"
#include "cerebro_util.h"
#include "cerebro_updown.h"

#include "conffile.h"
#include "ltdl.h"

char *cerebro_error_messages[] =
  {
    "success",
    "null cerebro_t handle",
    "invalid magic number",
    "invalid parameters",
    "server data not loaded",
    "cerebro version incompatabile",
    "invalid hostname",
    "invalid address",
    "connection error",
    "connection timeout",
    "network communication error",
    "protocol error",
    "protocol timeout",
    "buffer overflow",
    "node not found",
    "out of memory",
    "config error",
    "clusterlist module error",
    "config module error",
    "internal error",
    "errnum out of range",
  };

cerebro_t
cerebro_handle_create(void)
{
  cerebro_t handle = NULL;

  if (!(handle = (cerebro_t)malloc(sizeof(struct cerebro))))
    goto cleanup;
                 
  memset(handle, '\0', sizeof(struct cerebro));
  handle->magic = CEREBRO_MAGIC_NUMBER;
  handle->errnum = CEREBRO_ERR_SUCCESS;
  handle->loaded_state = 0;
  memset(&(handle->config_data), '\0', sizeof(struct cerebro_config));
  handle->updown_data = NULL;

 cleanup:
  return handle;
}

int
cerebro_handle_destroy(cerebro_t handle)
{
  if (cerebro_handle_check(handle) < 0)
    return -1;

  if (handle->loaded_state & CEREBRO_CONFIG_LOADED)
    {
      if (cerebro_unload_config(handle) < 0)
	return -1;

      if (handle->loaded_state & CEREBRO_CONFIG_LOADED)
        {
          handle->errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }

  if (handle->loaded_state & CEREBRO_CLUSTERLIST_MODULE_LOADED)
    {
      if (cerebro_unload_clusterlist_module(handle) < 0)
        return -1;
      
      if (handle->loaded_state & CEREBRO_CLUSTERLIST_MODULE_LOADED)
        {
          handle->errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }

  if (handle->loaded_state & CEREBRO_UPDOWN_DATA_LOADED)
    {
      if (cerebro_updown_unload_data(handle) < 0)
        return -1;

      if (handle->loaded_state & CEREBRO_UPDOWN_DATA_LOADED)
        {
          handle->errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }

  if (handle->loaded_state & CEREBRO_MODULE_SETUP_CALLED)
    {
      cerebro_module_cleanup();
      handle->loaded_state &= ~CEREBRO_MODULE_SETUP_CALLED;
    }

  handle->errnum = CEREBRO_ERR_SUCCESS;
  handle->magic = ~CEREBRO_MAGIC_NUMBER;
  free(handle);
  return 0;
}

int
cerebro_errnum(cerebro_t handle)
{
  if (!handle)
    return CEREBRO_ERR_NULLHANDLE;
  else if (handle->magic != CEREBRO_MAGIC_NUMBER)
    return CEREBRO_ERR_MAGIC_NUMBER;
  else
    return handle->errnum;
}

char *
cerebro_strerror(int errnum)
{
  if (errnum >= CEREBRO_ERR_SUCCESS && errnum <= CEREBRO_ERR_ERRNUMRANGE)
    return cerebro_error_messages[errnum];
  else
    return cerebro_error_messages[CEREBRO_ERR_ERRNUMRANGE];
}

char *
cerebro_errormsg(cerebro_t handle)
{
  return cerebro_strerror(cerebro_errnum(handle));
}

void
cerebro_perror(cerebro_t handle, const char *msg)
{
  char *errormsg = cerebro_strerror(cerebro_errnum(handle));
 
  if (!msg)
    fprintf(stderr, "%s\n", errormsg);
  else
    fprintf(stderr, "%s: %s\n", msg, errormsg);
}

int 
cerebro_load_config(cerebro_t handle)
{
  if (cerebro_handle_check(handle) < 0)
    return -1;

  if (handle->loaded_state & CEREBRO_CONFIG_LOADED)
    {
      handle->errnum = CEREBRO_ERR_SUCCESS;
      return 0;
    }
  
  if (cerebro_config_load(&(handle->config_data)) < 0)
    {
      handle->errnum = CEREBRO_ERR_CONFIG_FILE;
      return -1;
    }
  
  handle->loaded_state |= CEREBRO_CONFIG_LOADED;
  handle->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}

int 
cerebro_unload_config(cerebro_t handle)
{
  if (cerebro_handle_check(handle) < 0)
    return -1;

  memset(&(handle->config_data), '\0', sizeof(struct cerebro_config));
  
  handle->loaded_state &= ~CEREBRO_CONFIG_LOADED;
  handle->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}

int 
cerebro_load_clusterlist_module(cerebro_t handle)
{
  int module_setup_called = 0;
  int rv;

  if (cerebro_handle_check(handle) < 0)
    return -1;

  if (!(handle->loaded_state & CEREBRO_MODULE_SETUP_CALLED))
    {
      if (cerebro_module_setup() < 0)
	{
	  handle->errnum = CEREBRO_ERR_CLUSTERLIST_MODULE;
	  goto cleanup;
	}
    }

  if (cerebro_module_load_clusterlist_module() < 0)
    {
      handle->errnum = CEREBRO_ERR_CLUSTERLIST_MODULE;
      goto cleanup;
    }
  
  if (cerebro_clusterlist_module_setup() < 0)
    {
      handle->errnum = CEREBRO_ERR_CLUSTERLIST_MODULE;
      goto cleanup;
    }
  
  if (module_setup_called)
    handle->loaded_state |= CEREBRO_MODULE_SETUP_CALLED;
  handle->loaded_state |= CEREBRO_CLUSTERLIST_MODULE_LOADED;
  
  return rv;
 cleanup:
  if (module_setup_called)
    cerebro_module_cleanup();
  
  return -1;
}

int 
cerebro_unload_clusterlist_module(cerebro_t handle)
{
  if (cerebro_handle_check(handle) < 0)
    return -1;

  if (handle->loaded_state & CEREBRO_CLUSTERLIST_MODULE_LOADED)
    {
      if (cerebro_clusterlist_module_cleanup() < 0)
        {
	  handle->errnum = CEREBRO_ERR_CLUSTERLIST_MODULE;
	  return -1;
        }
      
      if (cerebro_module_unload_clusterlist_module() < 0)
	{
	  handle->errnum = CEREBRO_ERR_CLUSTERLIST_MODULE;
	  return -1;
	}
    }

  handle->loaded_state &= ~CEREBRO_CLUSTERLIST_MODULE_LOADED;
  handle->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}
