/*****************************************************************************\
 *  $Id: cerebro_updown.c,v 1.2 2005-04-26 00:09:13 achu Exp $
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
#include "cerebro_defs.h"
#include "cerebro_util.h"
#include "hostlist.h"

#define CEREBRO_UPDOWN_MAGIC 0xF00F5678

struct cerebro_updown_data {
  int32_t magic;
  hostlist_t up_nodes;
  hostlist_t down_nodes;
};

static int
_cerebro_updown_loaded_handle_err_check(cerebro_t handle)
{
  if (cerebro_handle_err_check(handle) < 0)
    return -1;

  if (!(handle->loaded_state & CEREBRO_UPDOWN_DATA_LOADED))
    {
      handle->errnum = CEREBRO_ERR_LOADED_DATA;
      return -1;
    }

  return 0;
}

int 
cerebro_updown_load_data(cerebro_t handle, 
                         const char *hostname, 
                         unsigned int port, 
                         unsigned int timeout_len)
{
  struct cerebro_updown_data *updown_data;
  hostlist_t up_nodes;
  hostlist_t down_nodes;
  
  if (cerebro_handle_err_check(handle) < 0)
    goto cleanup;

  return 0;
  
 cleanup:
  return -1;
}

int 
cerebro_updown_get_up_nodes(cerebro_t handle, char *buf, unsigned int buflen)
{
  if (_cerebro_updown_loaded_handle_err_check(handle) < 0)
    return -1;

  return 0;
}
 
int 
cerebro_updown_get_down_nodes(cerebro_t handle, char *buf, unsigned int buflen)
{
  if (_cerebro_updown_loaded_handle_err_check(handle) < 0)
    return -1;

  return 0;
}
 
int 
cerebro_updown_is_node_up(cerebro_t handle, const char *node)
{
  if (_cerebro_updown_loaded_handle_err_check(handle) < 0)
    return -1;

  return 0;
}
 
int 
cerebro_updown_is_node_down(cerebro_t handle, const char *node)
{
  if (_cerebro_updown_loaded_handle_err_check(handle) < 0)
    return -1;

  return 0;
}
 
int 
cerebro_updown_up_count(cerebro_t handle)
{
  if (_cerebro_updown_loaded_handle_err_check(handle) < 0)
    return -1;

  return 0;
}

int 
cerebro_updown_down_count(cerebro_t handle)
{
  if (_cerebro_updown_loaded_handle_err_check(handle) < 0)
    return -1;

  return 0;
}
