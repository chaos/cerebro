/*****************************************************************************\
 *  $Id: cerebro_nodelist.c,v 1.17 2010-02-02 01:01:20 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2015 Lawrence Livermore National Security, LLC.
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
#include <errno.h>

#include "cerebro.h"
#include "cerebro_api.h"
#include "cerebro_nodelist_util.h"
#include "cerebro_util.h"

#include "debug.h"
#include "list.h"

char *
cerebro_nodelist_metric_name(cerebro_nodelist_t nodelist)
{
  if (_cerebro_nodelist_check(nodelist) < 0)
    return NULL;
  
  nodelist->errnum = CEREBRO_ERR_SUCCESS;
  return nodelist->metric_name;
}

int 
cerebro_nodelist_length(cerebro_nodelist_t nodelist)
{
  if (_cerebro_nodelist_check(nodelist) < 0)
    return -1;
  
  nodelist->errnum = CEREBRO_ERR_SUCCESS;
  return list_count(nodelist->nodes);
}

int 
cerebro_nodelist_destroy(cerebro_nodelist_t nodelist)
{
  cerebro_nodelist_t tempnodelist;
  ListIterator itr = NULL;
  int found = 0, rv = -1;

  if (_cerebro_nodelist_check(nodelist) < 0)
    return -1;

  if (!(itr = list_iterator_create(nodelist->handle->nodelists)))
    {
      nodelist->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  while ((tempnodelist = list_next(itr)))
    {
      if (tempnodelist == nodelist)
        {
          list_remove(itr);
          
          nodelist->magic = ~CEREBRO_NODELIST_MAGIC_NUMBER;
          nodelist->errnum = CEREBRO_ERR_SUCCESS;
          /* 
           * destroy nodes first, since it will destroy iterators
           */
          list_destroy(nodelist->nodes);
          nodelist->nodes = NULL;
          list_destroy(nodelist->iterators);
          nodelist->iterators = NULL;
          nodelist->handle = NULL;
          free(nodelist);
          found++;
          break;
        }
    }

  if (!found)
    {
      nodelist->errnum = CEREBRO_ERR_PARAMETERS;
      goto cleanup;
    }

  rv = 0;
  list_iterator_destroy(itr);
  return 0;

 cleanup:
  if (itr)
    list_iterator_destroy(itr);
  return rv;
}
 
cerebro_nodelist_iterator_t 
cerebro_nodelist_iterator_create(cerebro_nodelist_t nodelist)
{
  cerebro_nodelist_iterator_t nodelistItr = NULL;

  if (_cerebro_nodelist_check(nodelist) < 0)
    return NULL;

  if (!(nodelistItr = malloc(sizeof(struct cerebro_nodelist_iterator))))
    {
      nodelist->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }
  memset(nodelistItr, '\0', sizeof(struct cerebro_nodelist_iterator));
  nodelistItr->magic = CEREBRO_NODELIST_ITERATOR_MAGIC_NUMBER;
  
  if (!(nodelistItr->itr = list_iterator_create(nodelist->nodes)))
    {
      nodelist->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }
  
  if (!list_append(nodelist->iterators, nodelistItr))
    {
      CEREBRO_DBG(("list_append: %s", strerror(errno)));
      nodelist->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  nodelistItr->nodelist = nodelist;
  nodelistItr->errnum = CEREBRO_ERR_SUCCESS;
  nodelistItr->current = list_next(nodelistItr->itr);
  return nodelistItr;

 cleanup:
  if (nodelistItr)
    {
      if (nodelistItr->itr)
        list_iterator_destroy(nodelistItr->itr);
      free(nodelistItr);
    }
  return NULL;
}
 
/* 
 * _cerebro_nodelist_iterator_check
 *
 * Checks for a proper cerebro nodelist
 *
 * Returns 0 on success, -1 on error
 */
int
_cerebro_nodelist_iterator_check(cerebro_nodelist_iterator_t nodelistItr)
{
  if (!nodelistItr 
      || nodelistItr->magic != CEREBRO_NODELIST_ITERATOR_MAGIC_NUMBER)
    return -1;

  if (!nodelistItr->itr || !nodelistItr->nodelist)
    {
      CEREBRO_DBG(("invalid namelist iterator data"));
      nodelistItr->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (nodelistItr->nodelist->magic != CEREBRO_NODELIST_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("nodelist destroyed"));
      nodelistItr->errnum = CEREBRO_ERR_MAGIC_NUMBER;
      return -1;
    }

  return 0;
}


int 
cerebro_nodelist_iterator_nodename(cerebro_nodelist_iterator_t nodelistItr,
                                   char **nodename)
{
  if (_cerebro_nodelist_iterator_check(nodelistItr) < 0)
    return -1;
  
  if (!nodelistItr->current)
    {
      nodelistItr->errnum = CEREBRO_ERR_END_OF_LIST;
      return -1;
    }
  
  if (nodename)
    *nodename = nodelistItr->current->nodename;
  return 0;
}

int 
cerebro_nodelist_iterator_metric_value(cerebro_nodelist_iterator_t nodelistItr,
                                       unsigned int *metric_value_received_time,
                                       unsigned int *metric_value_type,
                                       unsigned int *metric_value_len,
                                       void **metric_value)
{
  if (_cerebro_nodelist_iterator_check(nodelistItr) < 0)
    return -1;

  if (!nodelistItr->current)
    {
      nodelistItr->errnum = CEREBRO_ERR_END_OF_LIST;
      return -1;
    }

  if (metric_value_received_time)
    *metric_value_received_time = nodelistItr->current->metric_value_received_time;

  if (metric_value_type)
    *metric_value_type = nodelistItr->current->metric_value_type;
  
  if (metric_value_len)
    *metric_value_len = nodelistItr->current->metric_value_len;
  
  if (metric_value)
    *metric_value = nodelistItr->current->metric_value;

  return 0;
}

int
cerebro_nodelist_iterator_next(cerebro_nodelist_iterator_t nodelistItr)
{
  if (_cerebro_nodelist_iterator_check(nodelistItr) < 0)
    return -1;

  if (nodelistItr->current)
    nodelistItr->current = (struct cerebro_nodelist_data *)list_next(nodelistItr->itr);
  nodelistItr->errnum = CEREBRO_ERR_SUCCESS;
  return (nodelistItr->current) ? 1 : 0;
}
 
int 
cerebro_nodelist_iterator_reset(cerebro_nodelist_iterator_t nodelistItr)
{
  if (_cerebro_nodelist_iterator_check(nodelistItr) < 0)
    return -1;

  list_iterator_reset(nodelistItr->itr);
  nodelistItr->current = (struct cerebro_nodelist_data *)list_next(nodelistItr->itr);
  nodelistItr->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}

int
cerebro_nodelist_iterator_at_end(cerebro_nodelist_iterator_t nodelistItr)
{
  if (_cerebro_nodelist_iterator_check(nodelistItr) < 0)
    return -1;

  return (nodelistItr->current) ? 0 : 1;
}

int 
cerebro_nodelist_iterator_destroy(cerebro_nodelist_iterator_t nodelistItr)
{
  cerebro_nodelist_t nodelist;
  cerebro_nodelist_iterator_t tempItr;
  ListIterator itr = NULL;
  int found = 0, rv = -1;

  if (_cerebro_nodelist_iterator_check(nodelistItr) < 0)
    return -1;
  
  nodelist = nodelistItr->nodelist;

  if (!(itr = list_iterator_create(nodelist->iterators)))
    {
      nodelistItr->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }
  
  while ((tempItr = list_next(itr)))
    {
      if (tempItr == nodelistItr)
        {
          list_remove(itr);
          list_iterator_destroy(nodelistItr->itr);
          nodelistItr->magic = ~CEREBRO_NODELIST_ITERATOR_MAGIC_NUMBER;
          nodelistItr->errnum = CEREBRO_ERR_SUCCESS;
          free(nodelistItr);
          found++;
          break;
        }
    }
  
  if (!found)
    {
      nodelistItr->errnum = CEREBRO_ERR_PARAMETERS;
      goto cleanup;
    }

  rv = 0;
 cleanup:
  if (itr)
    list_iterator_destroy(itr);
  return rv;
}
