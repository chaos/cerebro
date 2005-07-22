/*****************************************************************************\
 *  $Id: cerebro_metriclist_util.c,v 1.7 2005-07-22 17:21:07 achu Exp $
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
#include <errno.h>

#include "cerebro.h"
#include "cerebro_api.h"
#include "cerebro_metriclist_util.h"

#include "debug.h"
#include "list.h"

int
_cerebro_metriclist_check(cerebro_metriclist_t metriclist)
{
  if (!metriclist || metriclist->magic != CEREBRO_METRICLIST_MAGIC_NUMBER)
    return -1;
                                                                                      
  if (!metriclist->metric_names
      || !metriclist->iterators
      || !metriclist->handle)
    {
      CEREBRO_DBG(("invalid metriclist data"));
      metriclist->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
                                                                                      
  if (metriclist->handle->magic != CEREBRO_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("handle destroyed"));
      metriclist->errnum = CEREBRO_ERR_MAGIC_NUMBER;
      return -1;
    }
                                                                                      
  return 0;
}

cerebro_metriclist_t 
_cerebro_metriclist_create(cerebro_t handle)
{
  cerebro_metriclist_t metriclist = NULL;

  if (!(metriclist = (cerebro_metriclist_t)malloc(sizeof(struct cerebro_metriclist))))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  memset(metriclist, '\0', sizeof(struct cerebro_metriclist));
  metriclist->magic = CEREBRO_METRICLIST_MAGIC_NUMBER;

  if (!(metriclist->metric_names = list_create((ListDelF)free)))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  /* No delete function, list_destroy() on 'metric_names' will take care of
   * deleting iterators.
   */
  if (!(metriclist->iterators = list_create(NULL)))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  if (!list_append(handle->metriclists, metriclist))
    {
      CEREBRO_DBG(("list_append: %s", strerror(errno)));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }
  
  metriclist->handle = handle;

  return metriclist;
      
 cleanup:
  if (metriclist)
    {
      if (metriclist->metric_names)
        list_destroy(metriclist->metric_names);
      if (metriclist->iterators)
        list_destroy(metriclist->iterators);
      free(metriclist);
    }
  return NULL;
}

int 
_cerebro_metriclist_append(cerebro_metriclist_t metriclist, 
                           const char *metric_name)
{
  char *str = NULL;

  if (_cerebro_metriclist_check(metriclist) < 0)
    return -1;

  if (!metric_name || strlen(metric_name) > CEREBRO_MAX_METRIC_NAME_LEN)
    {
      CEREBRO_DBG(("metric_name invalid"));
      metriclist->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  if (!(str = strdup(metric_name)))
    {
      metriclist->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  if (!list_append(metriclist->metric_names, str))
    {
      CEREBRO_DBG(("list_append: %s", strerror(errno)));
      metriclist->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }
  
  return 0;

 cleanup:
  free(str);
  return -1;
}
