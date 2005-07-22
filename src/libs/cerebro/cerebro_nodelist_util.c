/*****************************************************************************\
 *  $Id: cerebro_nodelist_util.c,v 1.8 2005-07-22 17:21:07 achu Exp $
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
#include "cerebro_nodelist_util.h"

#include "debug.h"
#include "list.h"

int
_cerebro_nodelist_check(cerebro_nodelist_t nodelist)
{
  if (!nodelist || nodelist->magic != CEREBRO_NODELIST_MAGIC_NUMBER)
    return -1;

  if (!nodelist->nodes
      || !nodelist->iterators
      || !nodelist->handle)
    {
      CEREBRO_DBG(("invalid nodelist data"));
      nodelist->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (nodelist->handle->magic != CEREBRO_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("handle destroyed"));
      nodelist->errnum = CEREBRO_ERR_MAGIC_NUMBER;
      return -1;
    }

  return 0;
}

/* 
 * _cerebro_nodelist_data_destroy
 *
 * Free contents of a nodelist data item
 */
static void
_cerebro_nodelist_data_destroy(void *x)
{
  struct cerebro_nodelist_data *nd;
  
  nd = (struct cerebro_nodelist_data *)x;

  if (nd->metric_value)
    free(nd->metric_value);
  free(nd);
}

cerebro_nodelist_t 
_cerebro_nodelist_create(cerebro_t handle, const char *metric_name)
{
  cerebro_nodelist_t nodelist = NULL;

  if (!(nodelist = (cerebro_nodelist_t)malloc(sizeof(struct cerebro_nodelist))))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  memset(nodelist, '\0', sizeof(struct cerebro_nodelist));
  nodelist->magic = CEREBRO_NODELIST_MAGIC_NUMBER;
  strcpy(nodelist->metric_name, metric_name);

  if (!(nodelist->nodes = list_create((ListDelF)_cerebro_nodelist_data_destroy)))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  /* No delete function, list_destroy() on 'nodes' will take care of
   * deleting iterators.
   */
  if (!(nodelist->iterators = list_create(NULL)))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  if (!list_append(handle->nodelists, nodelist))
    {
      CEREBRO_DBG(("list_append: %s", strerror(errno)));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }
  
  nodelist->handle = handle;

  return nodelist;
      
 cleanup:
  if (nodelist)
    {
      if (nodelist->nodes)
        list_destroy(nodelist->nodes);
      if (nodelist->iterators)
        list_destroy(nodelist->iterators);
      free(nodelist);
    }
  return NULL;
}

int 
_cerebro_nodelist_append(cerebro_nodelist_t nodelist,
			 const char *nodename,
                         u_int32_t metric_value_type,
                         u_int32_t metric_value_len,
                         void *metric_value)
{
  struct cerebro_nodelist_data *nd = NULL;

  if (_cerebro_nodelist_check(nodelist) < 0)
    return -1;

  if (!nodename || strlen(nodename) > CEREBRO_MAX_NODENAME_LEN)
    {
      CEREBRO_DBG(("nodename invalid"));
      nodelist->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  if (!(nd = (struct cerebro_nodelist_data *)malloc(sizeof(struct cerebro_nodelist_data))))
    {
      nodelist->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }
  memset(nd, '\0', sizeof(struct cerebro_nodelist_data));

  strncpy(nd->nodename, nodename, CEREBRO_MAX_NODENAME_LEN);
  nd->metric_value_type = metric_value_type;
  nd->metric_value_len = metric_value_len;
  if (metric_value)
    {
      int len;

      if (metric_value_type == CEREBRO_METRIC_VALUE_TYPE_STRING)
        len = metric_value_len + 1;
      else
        len = metric_value_len;

      if (!(nd->metric_value = malloc(len)))
        {
          nodelist->errnum = CEREBRO_ERR_OUTMEM;
          goto cleanup;
        }
      memset(nd->metric_value, '\0', len);
      memcpy(nd->metric_value, metric_value, metric_value_len);
    }
  else
    nd->metric_value = NULL;

  if (!list_append(nodelist->nodes, nd))
    {
      CEREBRO_DBG(("list_append: %s", strerror(errno)));
      nodelist->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }
  
  return 0;

 cleanup:
  if (nd)
    {
      if (nd->metric_value)
        free(nd->metric_value);
      free(nd);
    }
  return -1;
}

/*
 * _cerebro_nodelist_data_strcmp
 *
 * String comparison for nodelist data nodename
 *
 * Returns value identical to strcmp() for string comparisons
 */
int
_cerebro_nodelist_data_strcmp(void *x, void *y)
{
  struct cerebro_nodelist_data *datax, *datay;
  datax = (struct cerebro_nodelist_data *)x;
  datay = (struct cerebro_nodelist_data *)y;

  return strcmp(datax->nodename, datay->nodename);
}

int
_cerebro_nodelist_sort(cerebro_nodelist_t nodelist)
{
  if (_cerebro_nodelist_check(nodelist) < 0)
    return -1;

  list_sort(nodelist->nodes, _cerebro_nodelist_data_strcmp);
  return 0;
}
