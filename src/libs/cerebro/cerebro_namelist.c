/*****************************************************************************\
 *  $Id: cerebro_namelist.c,v 1.6 2010-02-02 01:01:20 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2018 Lawrence Livermore National Security, LLC.
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
#include "cerebro_namelist_util.h"
#include "cerebro_util.h"

#include "debug.h"
#include "list.h"

int
cerebro_namelist_length(cerebro_namelist_t namelist)
{
  if (_cerebro_namelist_check(namelist) < 0)
    return -1;

  namelist->errnum = CEREBRO_ERR_SUCCESS;
  return list_count(namelist->metric_names);
}

int
cerebro_namelist_destroy(cerebro_namelist_t namelist)
{
  cerebro_namelist_t tempnamelist;
  ListIterator itr = NULL;
  int found = 0, rv = -1;

  if (_cerebro_namelist_check(namelist) < 0)
    return -1;

  if (!(itr = list_iterator_create(namelist->handle->namelists)))
    {
      namelist->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  while ((tempnamelist = list_next(itr)))
    {
      if (tempnamelist == namelist)
        {
          list_remove(itr);

          namelist->magic = ~CEREBRO_NAMELIST_MAGIC_NUMBER;
          namelist->errnum = CEREBRO_ERR_SUCCESS;
          /*
           * destroy metric_names first, since it will destroy iterators
           */
          list_destroy(namelist->metric_names);
          namelist->metric_names = NULL;
          list_destroy(namelist->iterators);
          namelist->iterators = NULL;
          namelist->handle = NULL;
          free(namelist);
          found++;
          break;
        }
    }

  if (!found)
    {
      namelist->errnum = CEREBRO_ERR_PARAMETERS;
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

cerebro_namelist_iterator_t
cerebro_namelist_iterator_create(cerebro_namelist_t namelist)
{
  cerebro_namelist_iterator_t namelistItr = NULL;

  if (_cerebro_namelist_check(namelist) < 0)
    return NULL;

  if (!(namelistItr = malloc(sizeof(struct cerebro_namelist_iterator))))
    {
      namelist->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }
  memset(namelistItr, '\0', sizeof(struct cerebro_namelist_iterator));
  namelistItr->magic = CEREBRO_NAMELIST_ITERATOR_MAGIC_NUMBER;

  if (!(namelistItr->itr = list_iterator_create(namelist->metric_names)))
    {
      namelist->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  if (!list_append(namelist->iterators, namelistItr))
    {
      CEREBRO_DBG(("list_append: %s", strerror(errno)));
      namelist->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  namelistItr->namelist = namelist;
  namelistItr->errnum = CEREBRO_ERR_SUCCESS;
  namelistItr->current = list_next(namelistItr->itr);
  return namelistItr;

 cleanup:
  if (namelistItr)
    {
      if (namelistItr->itr)
        list_iterator_destroy(namelistItr->itr);
      free(namelistItr);
    }
  return NULL;
}

/*
 * _cerebro_namelist_iterator_check
 *
 * Checks for a proper cerebro namelist
 *
 * Returns 0 on success, -1 on error
 */
int
_cerebro_namelist_iterator_check(cerebro_namelist_iterator_t namelistItr)
{
  if (!namelistItr
      || namelistItr->magic != CEREBRO_NAMELIST_ITERATOR_MAGIC_NUMBER)
    return -1;

  if (!namelistItr->itr || !namelistItr->namelist)
    {
      CEREBRO_DBG(("invalid namelist iterator data"));
      namelistItr->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (namelistItr->namelist->magic != CEREBRO_NAMELIST_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("namelist destroyed"));
      namelistItr->errnum = CEREBRO_ERR_MAGIC_NUMBER;
      return -1;
    }

  return 0;
}


int
cerebro_namelist_iterator_name(cerebro_namelist_iterator_t namelistItr,
                               char **name)
{
  if (_cerebro_namelist_iterator_check(namelistItr) < 0)
    return -1;

  if (!namelistItr->current)
    {
      namelistItr->errnum = CEREBRO_ERR_END_OF_LIST;
      return -1;
    }

  if (name)
    *name = namelistItr->current;
  return 0;
}

int
cerebro_namelist_iterator_next(cerebro_namelist_iterator_t namelistItr)
{
  if (_cerebro_namelist_iterator_check(namelistItr) < 0)
    return -1;

  if (namelistItr->current)
    namelistItr->current = list_next(namelistItr->itr);
  namelistItr->errnum = CEREBRO_ERR_SUCCESS;
  return (namelistItr->current) ? 1 : 0;
}

int
cerebro_namelist_iterator_reset(cerebro_namelist_iterator_t namelistItr)
{
  if (_cerebro_namelist_iterator_check(namelistItr) < 0)
    return -1;

  list_iterator_reset(namelistItr->itr);
  namelistItr->current = list_next(namelistItr->itr);
  namelistItr->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}

int
cerebro_namelist_iterator_at_end(cerebro_namelist_iterator_t namelistItr)
{
  if (_cerebro_namelist_iterator_check(namelistItr) < 0)
    return -1;

  return (namelistItr->current) ? 0 : 1;
}

int
cerebro_namelist_iterator_destroy(cerebro_namelist_iterator_t namelistItr)
{
  cerebro_namelist_t namelist;
  cerebro_namelist_iterator_t tempItr;
  ListIterator itr = NULL;
  int found = 0, rv = -1;

  if (_cerebro_namelist_iterator_check(namelistItr) < 0)
    return -1;

  namelist = namelistItr->namelist;

  if (!(itr = list_iterator_create(namelist->iterators)))
    {
      namelistItr->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  while ((tempItr = list_next(itr)))
    {
      if (tempItr == namelistItr)
        {
          list_remove(itr);
          list_iterator_destroy(namelistItr->itr);
          namelistItr->magic = ~CEREBRO_NAMELIST_ITERATOR_MAGIC_NUMBER;
          namelistItr->errnum = CEREBRO_ERR_SUCCESS;
          free(namelistItr);
          found++;
          break;
        }
    }

  if (!found)
    {
      namelistItr->errnum = CEREBRO_ERR_PARAMETERS;
      goto cleanup;
    }

  rv = 0;
 cleanup:
  if (itr)
    list_iterator_destroy(itr);
  return rv;
}
