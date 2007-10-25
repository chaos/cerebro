/*****************************************************************************\
 *  $id: cerebro_metric.c,v 1.17 2005/06/07 16:18:58 achu Exp $
 *****************************************************************************
 *  Copyright (C) 2007 Lawrence Livermore National Security, LLC.
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
#include "cerebro_nodelist_util.h"
#include "cerebro_util.h"

#include "cerebro/cerebro_metric_server_protocol.h"

#include "cerebro_metric_util.h"

#include "debug.h"
#include "fd.h"
#include "marshall.h"
#include "data_util.h"
#include "network_util.h"

/* 
 * _metric_value_unmarshall
 *
 * Unmarshall contents of a metric server response
 *
 * Returns 0 on success, -1 on error
 */
static int
_metric_value_unmarshall(cerebro_t handle,
                         struct cerebro_metric_server_response *res,
                         void **metric_value,
                         const char *buf,
                         unsigned int buflen)
{
  int errnum = 0, mvalue_len = 0;
  void *mvalue = NULL;
  u_int32_t mtype, mlen;

#if CEREBRO_DEBUG
  if (!res 
      || res->metric_value_type == CEREBRO_DATA_VALUE_TYPE_NONE 
      || !metric_value 
      || !buf)
    {
      CEREBRO_DBG(("invalid parameters"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
#endif /* CEREBRO_DEBUG */

  mtype = res->metric_value_type;
  mlen = res->metric_value_len;

  /* Special case for ending null character */
  if (mtype == CEREBRO_DATA_VALUE_TYPE_STRING)
    mvalue_len = buflen + 1;
  else
    mvalue_len = buflen;

  if (!(mvalue = malloc(mvalue_len)))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      return -1;
    }
  memset(mvalue, '\0', mvalue_len);

  if (unmarshall_data_value(mtype, 
                            mlen,
                            mvalue,
                            mvalue_len,
                            buf,
                            buflen,
                            &errnum) < 0)
    {
      handle->errnum = errnum;
      goto cleanup;
    }

  *metric_value = mvalue;
  return 0;

 cleanup:
  free(mvalue);
  return -1;
}

/* 
 * _receive_metric_value
 *
 * Receive the metric value
 *
 * Returns 0 on success, -1 on error
 */
static int
_receive_metric_value(cerebro_t handle, 
                      struct cerebro_metric_server_response *res,
                      int fd)
{
  char *vbuf = NULL;
  int vbytes_read, rv = -1;
  void *metric_value = NULL;
  unsigned int errnum;

  if (!res->metric_value_len)
    {
      CEREBRO_DBG(("invalid parameters"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  if (!(vbuf = malloc(res->metric_value_len)))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }
      
  if ((vbytes_read = receive_data(fd,
                                  res->metric_value_len,
                                  vbuf,
                                  res->metric_value_len,
                                  CEREBRO_METRIC_SERVER_PROTOCOL_CLIENT_TIMEOUT_LEN,
                                  &errnum)) < 0)
    {
      handle->errnum = errnum;
      goto cleanup;
    }
  
  if (vbytes_read != res->metric_value_len)
    {
      handle->errnum = CEREBRO_ERR_PROTOCOL;
      goto cleanup;
    }
  
  if (_metric_value_unmarshall(handle, 
                               res, 
                               &metric_value, 
                               vbuf, 
                               vbytes_read) < 0)
      goto cleanup;

  res->metric_value = metric_value;
  
  rv = 0;
 cleanup:
  free(vbuf);
  return rv;
}

/* 
 * _receive_metric_data_response
 *
 * Receive a metric server data response.
 * 
 * Returns 0 on success, -1 on error
 */
static int
_receive_metric_data_response(cerebro_t handle, 
                              void *list, 
                              struct cerebro_metric_server_response *res,
                              unsigned int bytes_read,
                              int fd)
{
  struct cerebro_nodelist *nodelist;
  char nodename_buf[CEREBRO_MAX_NODENAME_LEN+1];
  u_int32_t mtype, mlen;
  int rv = -1;

  res->metric_value = NULL;

  if (_cerebro_handle_check(handle) < 0)
    {
      CEREBRO_DBG(("handle invalid"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  if (!list || !res || fd <= 0)
    {
      CEREBRO_DBG(("invalid parameters"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  nodelist = (struct cerebro_nodelist *)list;
  if (nodelist->magic != CEREBRO_NODELIST_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("invalid parameters"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  mtype = res->metric_value_type;
  mlen = res->metric_value_len;
  if (check_data_type_len(mtype, mlen) < 0)
    {
      handle->errnum = CEREBRO_ERR_PROTOCOL;
      goto cleanup;
    }

  if (mlen)
    {
      if (_receive_metric_value(handle, res, fd) < 0)
        goto cleanup;
    }

  /* Guarantee ending '\0' character */
  memset(nodename_buf, '\0', CEREBRO_MAX_NODENAME_LEN+1);
  memcpy(nodename_buf, res->name, CEREBRO_MAX_NODENAME_LEN);

  if (_cerebro_nodelist_append(nodelist, 
                               nodename_buf,
                               res->metric_value_received_time,
                               res->metric_value_type,
                               res->metric_value_len,
                               res->metric_value) < 0)
    goto cleanup_metric_value;
  
  rv = 0;
 cleanup_metric_value:
  free(res->metric_value);
 cleanup:
  return rv;
}

cerebro_nodelist_t 
cerebro_get_metric_data(cerebro_t handle, const char *metric_name)
{
  struct cerebro_nodelist *nodelist = NULL;

  if (_cerebro_handle_check(handle) < 0)
    goto cleanup;

  if (!metric_name || strlen(metric_name) > CEREBRO_MAX_METRIC_NAME_LEN)
    {
      handle->errnum = CEREBRO_ERR_PARAMETERS;
      goto cleanup;
    }
  
  if (!(nodelist = _cerebro_nodelist_create(handle, metric_name)))
    goto cleanup;
  
  if (_cerebro_metric_get_data(handle,
                               nodelist,
                               metric_name,
                               _receive_metric_data_response) < 0)
    goto cleanup;
  
  if (_cerebro_nodelist_sort(nodelist) < 0)
    goto cleanup;

  handle->errnum = CEREBRO_ERR_SUCCESS;
  return nodelist;
  
 cleanup:
  if (nodelist)
    (void)cerebro_nodelist_destroy(nodelist);
  return NULL;
}
