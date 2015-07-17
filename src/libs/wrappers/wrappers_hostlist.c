/*****************************************************************************\
 *  $Id: wrappers_hostlist.c,v 1.11 2010-02-02 01:01:21 chu11 Exp $
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
 *  with Cerebro. If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <limits.h>
#include <assert.h>
#include <errno.h>

#include "wrappers.h"

hostlist_t 
wrap_hostlist_create(WRAPPERS_ARGS, const char *hostlist)
{
  hostlist_t rv;

  assert(file && function);

  if (!(rv = hostlist_create(hostlist)))
    WRAPPERS_ERR_ERRNO("hostlist_create");

  return rv;
}

void
wrap_hostlist_destroy(WRAPPERS_ARGS, hostlist_t hl)
{
  assert(file && function);

  if (!hl)
    WRAPPERS_ERR_INVALID_PARAMETERS("hostlist_destroy");

  hostlist_destroy(hl);
  return;
}

void 
wrap_hostlist_sort(WRAPPERS_ARGS, hostlist_t hl)
{
  assert(file && function);

  if (!hl)
    WRAPPERS_ERR_INVALID_PARAMETERS("hostlist_sort");

  hostlist_sort(hl);
  return;
}

void 
wrap_hostlist_uniq(WRAPPERS_ARGS, hostlist_t hl)
{
 assert(file && function);

  if (!hl)
    WRAPPERS_ERR_INVALID_PARAMETERS("hostlist_uniq");

  hostlist_uniq(hl);
  return;
}

int 
wrap_hostlist_push(WRAPPERS_ARGS, hostlist_t hl, const char *host)
{
  int rv;

  assert(file && function);

  if (!hl || !host)
    WRAPPERS_ERR_INVALID_PARAMETERS("hostlist_push");

  if (!(rv = hostlist_push(hl, host)))
    WRAPPERS_ERR_ERRNO("hostlist_push");

  return rv;
}

int 
wrap_hostlist_find(WRAPPERS_ARGS, hostlist_t hl, const char *hostname)
{
  assert(file && function);

  if (!hl || !hostname)
    WRAPPERS_ERR_INVALID_PARAMETERS("hostlist_find");

  /* -1 isn't an error, it indicates the host isn't found */
  return hostlist_find(hl, hostname);
}

ssize_t 
wrap_hostlist_ranged_string(WRAPPERS_ARGS, hostlist_t hl, size_t n, char *buf)
{
  ssize_t rv;

  assert(file && function);

  if (!hl || !buf || !(n > 0 || n <= INT_MAX))
    WRAPPERS_ERR_INVALID_PARAMETERS("hostlist_ranged_string");

  if ((rv = hostlist_ranged_string(hl, n, buf)) < 0)
    WRAPPERS_ERR_ERRNO("hostlist_ranged_string");

  return rv;
}

ssize_t 
wrap_hostlist_deranged_string(WRAPPERS_ARGS, hostlist_t hl, size_t n, char *buf)
{
  ssize_t rv;

  assert(file && function);

  if (!hl || !buf || !(n > 0 || n <= INT_MAX))
    WRAPPERS_ERR_INVALID_PARAMETERS("hostlist_deranged_string");

  if ((rv = hostlist_deranged_string(hl, n, buf)) < 0)
    WRAPPERS_ERR_ERRNO("hostlist_deranged_string");

  return rv;
}

hostlist_iterator_t 
wrap_hostlist_iterator_create(WRAPPERS_ARGS, hostlist_t hl)
{
  hostlist_iterator_t rv;

  assert(file && function && hl);

  if (!(rv = hostlist_iterator_create(hl)))
    WRAPPERS_ERR_ERRNO("hostlist_iterator_create");

  return rv;
}

void
wrap_hostlist_iterator_destroy(WRAPPERS_ARGS, hostlist_iterator_t i)
{
  assert(file && function);

  if (!i)
    WRAPPERS_ERR_INVALID_PARAMETERS("hostlist_itreator_destroy");

  hostlist_iterator_destroy(i);
  return;
}

char *
wrap_hostlist_next(WRAPPERS_ARGS, hostlist_iterator_t i)
{
  char *rv;

  assert(file && function && i);

  /* Can return NULL value to indicate end of list */
  rv = hostlist_next(i);
  
  return rv;
}
