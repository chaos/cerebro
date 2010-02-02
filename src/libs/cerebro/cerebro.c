/*****************************************************************************\
 *  $Id: cerebro.c,v 1.19 2010-02-02 01:01:20 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2010 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2005-2007 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <http://www.llnl.gov/linux/cerebro/>.
 *
 *  Cerebro is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  Cerebro is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with Cerebro.  If not, see <http://www.gnu.org/licenses/>.
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
#include "cerebro_config_util.h"
#include "cerebro_util.h"
#include "cerebro/cerebro_config.h"

#include "debug.h"

static char *cerebro_error_messages[] =
  {
    "success",                                   /* 0 */
    "null cerebro_t handle",                     /* 1 */
    "null cerebro_namelist_t namelist",          /* 2 */
    "null cerebro_namelist_iterator_t iterator", /* 3 */
    "null cerebro_nodelist_t nodelist",          /* 4 */
    "null cerebro_nodelist_iterator_t iterator", /* 5 */
    "invalid magic number found",                /* 6 */
    "invalid parameters",                        /* 7 */
    "invalid hostname",                          /* 8 */
    "connection error",                          /* 9 */
    "connection timeout",                        /* 10 */
    "protocol error",                            /* 11 */
    "protocol timeout",                          /* 12 */
    "version incompatible",                      /* 13 */
    "buffer overflow",                           /* 14 */
    "node not found",                            /* 15 */
    "metric name invalid",                       /* 16 */
    "event name invalid",                        /* 17 */
    "metric max reached",                        /* 18 */
    "end of list reached",                       /* 19 */
    "event not received",                        /* 20 */
    "config file error",                         /* 21 */
    "config module error",                       /* 22 */
    "config input error",                        /* 23 */
    "out of memory",                             /* 24 */
    "internal error",                            /* 25 */
    "errnum out of range",                       /* 26 */
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
  memset(&(handle->hostname), '\0', CEREBRO_MAX_HOSTNAME_LEN+1);
  handle->port = 0;
  handle->timeout_len = 0;
  handle->flags = 0;
  handle->loaded_state = 0;
  memset(&(handle->config_data), '\0', sizeof(struct cerebro_config));
  
  if (!(handle->namelists = list_create((ListDelF)cerebro_namelist_destroy)))
    goto cleanup;

  if (!(handle->nodelists = list_create((ListDelF)cerebro_nodelist_destroy)))
    goto cleanup;

  if (!(handle->event_fds = list_create((ListDelF)free)))
    goto cleanup;

  return handle;

 cleanup:
  if (handle)
    {
      if (handle->namelists)
        list_destroy(handle->namelists);
      if (handle->nodelists)
        list_destroy(handle->nodelists);
      if (handle->event_fds)
        list_destroy(handle->event_fds);
      free(handle);
    }
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
          CEREBRO_DBG(("loaded_state invalid"));
          handle->errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }

  list_destroy(handle->namelists);
  handle->namelists = NULL;
  list_destroy(handle->nodelists);
  handle->nodelists = NULL;
  list_destroy(handle->event_fds);
  handle->event_fds = NULL;
  
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
cerebro_namelist_errnum(cerebro_namelist_t namelist)
{
  if (!namelist)
    return CEREBRO_ERR_NULLNAMELIST;
  else if (namelist->magic != CEREBRO_NAMELIST_MAGIC_NUMBER)
    return CEREBRO_ERR_MAGIC_NUMBER;
  else
    return namelist->errnum;
}

int 
cerebro_namelist_iterator_errnum(cerebro_namelist_iterator_t namelistItr)
{
  if (!namelistItr)
    return CEREBRO_ERR_NULLNAMELIST_ITERATOR;
  else if (namelistItr->magic != CEREBRO_NAMELIST_ITERATOR_MAGIC_NUMBER)
    return CEREBRO_ERR_MAGIC_NUMBER;
  else
    return namelistItr->errnum;
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
    return CEREBRO_ERR_NULLNAMELIST_ITERATOR;
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

char *
cerebro_get_hostname(cerebro_t handle)
{
  if (_cerebro_handle_check(handle) < 0)
    return NULL;

  return handle->hostname;
}

int 
cerebro_set_hostname(cerebro_t handle, const char *hostname)
{
  if (_cerebro_handle_check(handle) < 0)
    return -1;

  if (hostname && strlen(hostname) > CEREBRO_MAX_HOSTNAME_LEN)
    {
      handle->errnum = CEREBRO_ERR_OVERFLOW;
      return -1;
    }

  if (hostname)
    strcpy(handle->hostname, hostname);
  else
    memset(handle->hostname, '\0', CEREBRO_MAX_HOSTNAME_LEN+1);

  return 0;
}

int 
cerebro_get_port(cerebro_t handle)
{
  if (_cerebro_handle_check(handle) < 0)
    return -1;

  return handle->port;
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
cerebro_get_timeout_len(cerebro_t handle)
{
  if (_cerebro_handle_check(handle) < 0)
    return -1;

  return handle->timeout_len;
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
cerebro_get_flags(cerebro_t handle)
{
  if (_cerebro_handle_check(handle) < 0)
    return -1;

  return handle->flags;
}

int 
cerebro_set_flags(cerebro_t handle, unsigned int flags)
{
  if (_cerebro_handle_check(handle) < 0)
    return -1;

  if (flags & ~CEREBRO_METRIC_FLAGS_MASK)
    {
      handle->errnum = CEREBRO_ERR_PARAMETERS;
      return -1;
    }

  handle->flags = flags;
  return 0;
}
