/*****************************************************************************\
 *  $Id: cerebro_metriclist.c,v 1.4 2005-07-22 17:21:07 achu Exp $
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
#include "cerebro_util.h"

#include "debug.h"
#include "list.h"

int 
cerebro_metriclist_length(cerebro_metriclist_t metriclist)
{
  if (_cerebro_metriclist_check(metriclist) < 0)
    return -1;
  
  metriclist->errnum = CEREBRO_ERR_SUCCESS;
  return list_count(metriclist->metric_names);
}

int 
cerebro_metriclist_destroy(cerebro_metriclist_t metriclist)
{
  cerebro_metriclist_t tempmetriclist;
  ListIterator itr = NULL;
  int found = 0, rv = -1;

  if (_cerebro_metriclist_check(metriclist) < 0)
    return -1;

  if (!(itr = list_iterator_create(metriclist->handle->metriclists)))
    {
      metriclist->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  while ((tempmetriclist = list_next(itr)))
    {
      if (tempmetriclist == metriclist)
        {
          list_remove(itr);
          
          metriclist->magic = ~CEREBRO_METRICLIST_MAGIC_NUMBER;
          metriclist->errnum = CEREBRO_ERR_SUCCESS;
          /* 
           * destroy metric_names first, since it will destroy iterators
           */
          list_destroy(metriclist->metric_names);
          metriclist->metric_names = NULL;
          list_destroy(metriclist->iterators);
          metriclist->iterators = NULL;
          metriclist->handle = NULL;
          free(metriclist);
          found++;
          break;
        }
    }

  if (!found)
    {
      metriclist->errnum = CEREBRO_ERR_PARAMETERS;
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
 
cerebro_metriclist_iterator_t 
cerebro_metriclist_iterator_create(cerebro_metriclist_t metriclist)
{
  cerebro_metriclist_iterator_t metriclistItr = NULL;

  if (_cerebro_metriclist_check(metriclist) < 0)
    return NULL;

  if (!(metriclistItr = malloc(sizeof(struct cerebro_metriclist_iterator))))
    {
      metriclist->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }
  memset(metriclistItr, '\0', sizeof(struct cerebro_metriclist_iterator));
  metriclistItr->magic = CEREBRO_METRICLIST_ITERATOR_MAGIC_NUMBER;
  
  if (!(metriclistItr->itr = list_iterator_create(metriclist->metric_names)))
    {
      metriclist->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }
  
  if (!list_append(metriclist->iterators, metriclistItr))
    {
      CEREBRO_DBG(("list_append: %s", strerror(errno)));
      metriclist->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  metriclistItr->metriclist = metriclist;
  metriclistItr->errnum = CEREBRO_ERR_SUCCESS;
  metriclistItr->current = list_next(metriclistItr->itr);
  return metriclistItr;

 cleanup:
  if (metriclistItr)
    {
      if (metriclistItr->itr)
        list_iterator_destroy(metriclistItr->itr);
      free(metriclistItr);
    }
  return NULL;
}
 
/* 
 * _cerebro_metriclist_iterator_check
 *
 * Checks for a proper cerebro metriclist
 *
 * Returns 0 on success, -1 on error
 */
int
_cerebro_metriclist_iterator_check(cerebro_metriclist_iterator_t metriclistItr)
{
  if (!metriclistItr 
      || metriclistItr->magic != CEREBRO_METRICLIST_ITERATOR_MAGIC_NUMBER)
    return -1;

  if (!metriclistItr->itr || !metriclistItr->metriclist)
    {
      CEREBRO_DBG(("invalid metriclist iterator data"));
      metriclistItr->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  
  if (metriclistItr->metriclist->magic != CEREBRO_METRICLIST_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("metriclist destroyed"));
      metriclistItr->errnum = CEREBRO_ERR_MAGIC_NUMBER;
      return -1;
    }

  return 0;
}


int 
cerebro_metriclist_iterator_metric_name(cerebro_metriclist_iterator_t metriclistItr,
                                        char **metric_name)
{
  if (_cerebro_metriclist_iterator_check(metriclistItr) < 0)
    return -1;
  
  if (!metriclistItr->current)
    {
      metriclistItr->errnum = CEREBRO_ERR_END_OF_LIST;
      return -1;
    }
  
  if (metric_name)
    *metric_name = metriclistItr->current;
  return 0;
}

int
cerebro_metriclist_iterator_next(cerebro_metriclist_iterator_t metriclistItr)
{
  if (_cerebro_metriclist_iterator_check(metriclistItr) < 0)
    return -1;

  if (metriclistItr->current)
    metriclistItr->current = list_next(metriclistItr->itr);
  metriclistItr->errnum = CEREBRO_ERR_SUCCESS;
  return (metriclistItr->current) ? 1 : 0;
}
 
int 
cerebro_metriclist_iterator_reset(cerebro_metriclist_iterator_t metriclistItr)
{
  if (_cerebro_metriclist_iterator_check(metriclistItr) < 0)
    return -1;

  list_iterator_reset(metriclistItr->itr);
  metriclistItr->current = list_next(metriclistItr->itr);
  metriclistItr->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}

int
cerebro_metriclist_iterator_at_end(cerebro_metriclist_iterator_t metriclistItr)
{
  if (_cerebro_metriclist_iterator_check(metriclistItr) < 0)
    return -1;

  return (metriclistItr->current) ? 0 : 1;
}

int 
cerebro_metriclist_iterator_destroy(cerebro_metriclist_iterator_t metriclistItr)
{
  cerebro_metriclist_t metriclist;
  cerebro_metriclist_iterator_t tempItr;
  ListIterator itr = NULL;
  int found = 0, rv = -1;

  if (_cerebro_metriclist_iterator_check(metriclistItr) < 0)
    return -1;
  
  metriclist = metriclistItr->metriclist;

  if (!(itr = list_iterator_create(metriclist->iterators)))
    {
      metriclistItr->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }
  
  while ((tempItr = list_next(itr)))
    {
      if (tempItr == metriclistItr)
        {
          list_remove(itr);
          list_iterator_destroy(metriclistItr->itr);
          metriclistItr->magic = ~CEREBRO_METRICLIST_ITERATOR_MAGIC_NUMBER;
          metriclistItr->errnum = CEREBRO_ERR_SUCCESS;
          free(metriclistItr);
          found++;
          break;
        }
    }
  
  if (!found)
    {
      metriclistItr->errnum = CEREBRO_ERR_PARAMETERS;
      goto cleanup;
    }

  rv = 0;
 cleanup:
  if (itr)
    list_iterator_destroy(itr);
  return rv;
}
