/*****************************************************************************\
 *  $Id: cerebro_namelist_util.c,v 1.1 2006-11-09 23:20:08 chu11 Exp $
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
#include "cerebro_namelist_util.h"

#include "debug.h"
#include "list.h"

int
_cerebro_namelist_check(cerebro_namelist_t namelist)
{
  if (!namelist || namelist->magic != CEREBRO_NAMELIST_MAGIC_NUMBER)
    return -1;
                                                                                      
  if (!namelist->metric_names
      || !namelist->iterators
      || !namelist->handle)
    {
      CEREBRO_DBG(("invalid namelist data"));
      namelist->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
                                                                                      
  if (namelist->handle->magic != CEREBRO_MAGIC_NUMBER)
    {
      CEREBRO_DBG(("handle destroyed"));
      namelist->errnum = CEREBRO_ERR_MAGIC_NUMBER;
      return -1;
    }
                                                                                      
  return 0;
}

cerebro_namelist_t 
_cerebro_namelist_create(cerebro_t handle)
{
  cerebro_namelist_t namelist = NULL;

  if (!(namelist = (cerebro_namelist_t)malloc(sizeof(struct cerebro_namelist))))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  memset(namelist, '\0', sizeof(struct cerebro_namelist));
  namelist->magic = CEREBRO_NAMELIST_MAGIC_NUMBER;

  if (!(namelist->metric_names = list_create((ListDelF)free)))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  /* No delete function, list_destroy() on 'metric_names' will take care of
   * deleting iterators.
   */
  if (!(namelist->iterators = list_create(NULL)))
    {
      handle->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  if (!list_append(handle->namelists, namelist))
    {
      CEREBRO_DBG(("list_append: %s", strerror(errno)));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }
  
  namelist->handle = handle;

  return namelist;
      
 cleanup:
  if (namelist)
    {
      if (namelist->metric_names)
        list_destroy(namelist->metric_names);
      if (namelist->iterators)
        list_destroy(namelist->iterators);
      free(namelist);
    }
  return NULL;
}

int 
_cerebro_namelist_append(cerebro_namelist_t namelist, 
                           const char *metric_name)
{
  char *str = NULL;

  if (_cerebro_namelist_check(namelist) < 0)
    return -1;

  if (!metric_name || strlen(metric_name) > CEREBRO_MAX_METRIC_NAME_LEN)
    {
      CEREBRO_DBG(("metric_name invalid"));
      namelist->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  if (!(str = strdup(metric_name)))
    {
      namelist->errnum = CEREBRO_ERR_OUTMEM;
      goto cleanup;
    }

  if (!list_append(namelist->metric_names, str))
    {
      CEREBRO_DBG(("list_append: %s", strerror(errno)));
      namelist->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }
  
  return 0;

 cleanup:
  free(str);
  return -1;
}
