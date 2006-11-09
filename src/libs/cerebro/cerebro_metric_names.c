/*****************************************************************************\
 *  $id: cerebro_metric.c,v 1.17 2005/06/07 16:18:58 achu Exp $
 *****************************************************************************
 *  Copyright (C) 2005 The Regents of the University of California.
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
 *  with Genders; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
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
#include "cerebro_namelist_util.h"
#include "cerebro_util.h"

#include "cerebro/cerebro_metric_server_protocol.h"

#include "cerebro_metric_util.h"

#include "fd.h"
#include "debug.h"
#include "marshall.h"

/* 
 * _receive_metric_name_response
 *
 * Receive a metric server name response
 * 
 * Returns 0 on success, -1 on error
 */
static int
_receive_metric_name_response(cerebro_t handle,
                              void *list,
                              struct cerebro_metric_server_response *res,
                              unsigned int bytes_read,
                              int fd)
{
  char metric_name_buf[CEREBRO_MAX_METRIC_NAME_LEN+1];
  struct cerebro_namelist *namelist;

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

  namelist = (struct cerebro_namelist *)list;
  if (namelist->magic != CEREBRO_NAMELIST_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("invalid parameters"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  if (bytes_read != CEREBRO_METRIC_SERVER_RESPONSE_HEADER_LEN)
    {
      handle->errnum = CEREBRO_ERR_PROTOCOL;
      goto cleanup;
    }
      
  /* Guarantee ending '\0' character */
  memset(metric_name_buf, '\0', CEREBRO_MAX_METRIC_NAME_LEN+1);
  memcpy(metric_name_buf, res->name, CEREBRO_MAX_METRIC_NAME_LEN);
  
  if (_cerebro_namelist_append(namelist, metric_name_buf) < 0)
    goto cleanup;

  return 0;

 cleanup:
  return -1;
}

cerebro_namelist_t 
cerebro_get_metric_names(cerebro_t handle)
{
  struct cerebro_namelist *namelist = NULL;

  if (_cerebro_handle_check(handle) < 0)
    goto cleanup;

  if (!(namelist = _cerebro_namelist_create(handle)))
    goto cleanup;

  if (_cerebro_metric_get_data(handle,
                               namelist,
                               CEREBRO_METRIC_METRIC_NAMES,
                               _receive_metric_name_response) < 0)
    goto cleanup;
  
                                            
  
  handle->errnum = CEREBRO_ERR_SUCCESS;
  return namelist;
  
 cleanup:
  if (namelist)
    (void)cerebro_namelist_destroy(namelist);
  return NULL;
}
