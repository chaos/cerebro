/*****************************************************************************\
 *  $Id: wrappers_hostlist.c,v 1.2 2007-09-05 18:16:00 chu11 Exp $
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
 *  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

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

size_t 
wrap_hostlist_ranged_string(WRAPPERS_ARGS, hostlist_t hl, size_t n, char *buf)
{
  size_t rv;

  assert(file && function);

  if (!hl || !buf || !(n > 0 || n <= INT_MAX))
    WRAPPERS_ERR_INVALID_PARAMETERS("hostlist_ranged_string");

  if ((rv = hostlist_ranged_string(hl, n, buf)) < 0)
    WRAPPERS_ERR_ERRNO("hostlist_ranged_string");

  return rv;
}

size_t 
wrap_hostlist_deranged_string(WRAPPERS_ARGS, hostlist_t hl, size_t n, char *buf)
{
  size_t rv;

  assert(file && function);

  if (!hl || !buf || !(n > 0 || n <= INT_MAX))
    WRAPPERS_ERR_INVALID_PARAMETERS("hostlist_deranged_string");

  if ((rv = hostlist_deranged_string(hl, n, buf)) < 0)
    WRAPPERS_ERR_ERRNO("hostlist_deranged_string");

  return rv;
}
