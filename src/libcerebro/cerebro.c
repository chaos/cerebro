/*****************************************************************************\
 *  $Id: cerebro.c,v 1.29 2005-06-03 21:26:04 achu Exp $
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
#include "cerebro_clusterlist_util.h"
#include "cerebro_config_util.h"
#include "cerebro_module.h"
#include "cerebro_util.h"
#include "cerebro/cerebro_config.h"
#include "cerebro/cerebro_metric_protocol.h"

static char *cerebro_error_messages[] =
  {
    "success",
    "null cerebro_t handle",
    "null cerebro_nodelist_t nodelist",
    "null cerebro_nodelist_iterator_t iterator",
    "invalid magic number found",
    "invalid parameters",
    "invalid hostname",
    "connection error",
    "connection timeout",
    "protocol error",
    "protocol timeout",
    "version incompatible",
    "buffer overflow",
    "node not found",
    "end of list reached",
    "metric value not found",
    "config file error",
    "config module error",
    "config input error",
    "clusterlist module error",
    "out of memory",
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
  memset(&(handle->hostname), '\0', CEREBRO_MAXHOSTNAMELEN+1);
  handle->port = 0;
  handle->timeout_len = 0;
  handle->flags = 0;
  handle->loaded_state = 0;
  memset(&(handle->config_data), '\0', sizeof(struct cerebro_config));
  
  if (!(handle->nodelists = list_create((ListDelF)cerebro_nodelist_destroy)))
    goto cleanup;
  
  return handle;

 cleanup:
  if (handle)
    free(handle);
  return NULL;
}

int
cerebro_handle_destroy(cerebro_t handle)
{
  if (_cerebro_handle_check(handle) < 0)
    return -1;

  if (handle->loaded_state & CEREBRO_CONFIG_LOADED)
    {
      if (_cerebro_unload_config(handle) < 0)
	return -1;

      if (handle->loaded_state & CEREBRO_CONFIG_LOADED)
        {
          handle->errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }

  if (handle->loaded_state & CEREBRO_CLUSTERLIST_MODULE_LOADED)
    {
      if (_cerebro_unload_clusterlist_module(handle) < 0)
        return -1;
      
      if (handle->loaded_state & CEREBRO_CLUSTERLIST_MODULE_LOADED)
        {
          handle->errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }

  list_destroy(handle->nodelists);
  handle->nodelists = NULL;

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


int 
cerebro_nodelist_errnum(cerebro_nodelist_t nodelist)
{
  if (!nodelist)
    return CEREBRO_ERR_NULLNODELIST;
  else if (nodelist->magic != CEREBRO_NODELIST_MAGIC_NUMBER)
    return CEREBRO_ERR_MAGIC_NUMBER;
  else
    return nodelist->errnum;
}

int 
cerebro_nodelist_iterator_errnum(cerebro_nodelist_iterator_t nodelistItr)
{
  if (!nodelistItr)
    return CEREBRO_ERR_NULLITERATOR;
  else if (nodelistItr->magic != CEREBRO_NODELIST_ITERATOR_MAGIC_NUMBER)
    return CEREBRO_ERR_MAGIC_NUMBER;
  else
    return nodelistItr->errnum;
}

char *
cerebro_strerror(int errnum)
{
  if (errnum >= CEREBRO_ERR_SUCCESS && errnum <= CEREBRO_ERR_ERRNUMRANGE)
    return cerebro_error_messages[errnum];
  else
    return cerebro_error_messages[CEREBRO_ERR_ERRNUMRANGE];
}

int 
cerebro_set_hostname(cerebro_t handle, const char *hostname)
{
  if (_cerebro_handle_check(handle) < 0)
    return -1;

  if (hostname && strlen(hostname) > CEREBRO_MAXHOSTNAMELEN)
    {
      handle->errnum = CEREBRO_ERR_OVERFLOW;
      return -1;
    }

  if (hostname)
    strcpy(handle->hostname, hostname);
  else
    memset(handle->hostname, '\0', CEREBRO_MAXHOSTNAMELEN+1);

  return 0;
}

int 
cerebro_set_port(cerebro_t handle, unsigned int port)
{
  if (_cerebro_handle_check(handle) < 0)
    return -1;

  handle->port = port;
  return 0;
}

int 
cerebro_set_timeout_len(cerebro_t handle, unsigned int timeout_len)
{
  if (_cerebro_handle_check(handle) < 0)
    return -1;

  handle->timeout_len = timeout_len;
  return 0;
}

int 
cerebro_set_flags(cerebro_t handle, unsigned int flags)
{
  if (_cerebro_handle_check(handle) < 0)
    return -1;

  /* XXX should check for valid flags */

  handle->flags = flags;
  return 0;
}

