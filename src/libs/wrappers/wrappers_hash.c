/*****************************************************************************\
 *  $Id: wrappers_hash.c,v 1.8 2010-02-02 01:01:21 chu11 Exp $
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
 *  with Cerebro. If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <limits.h>
#include <assert.h>
#include <errno.h>

#include "wrappers.h"

hash_t 
wrap_hash_create(WRAPPERS_ARGS, int size, hash_key_f key_f, hash_cmp_f cmp_f, hash_del_f del_f)
{
  hash_t rv;

  assert(file && function);

  if (!(size > 0 || size <= INT_MAX) || !key_f || !cmp_f)
    WRAPPERS_ERR_INVALID_PARAMETERS("hash_create");

  if (!(rv = hash_create(size, key_f, cmp_f, del_f)))
    WRAPPERS_ERR_ERRNO("hash_create");

  return rv;
}

int 
wrap_hash_count(WRAPPERS_ARGS, hash_t h)
{
  int rv;

  assert(file && function);

  if (!h)
    WRAPPERS_ERR_INVALID_PARAMETERS("hash_count");

  if (!(rv = hash_count(h)))
    {
      if (errno != 0)
        WRAPPERS_ERR_ERRNO("hash_count");
    }

  return rv;
}

void *
wrap_hash_find(WRAPPERS_ARGS, hash_t h, const void *key)
{
  void *rv;

  assert(file && function);

  if (!h || !key)
    WRAPPERS_ERR_INVALID_PARAMETERS("hash_find");

  rv = hash_find(h, key);
  if (!rv && errno != 0)
    WRAPPERS_ERR_ERRNO("hash_find");

  return rv;
}

void *
wrap_hash_insert(WRAPPERS_ARGS, hash_t h, const void *key, void *data)
{
  void *rv;

  assert(file && function);

  if (!h || !key || !data)
    WRAPPERS_ERR_INVALID_PARAMETERS("hash_insert");

  if (!(rv = hash_insert(h, key, data)))
    WRAPPERS_ERR_ERRNO("hash_insert");

  if (rv != data)
    WRAPPERS_ERR_MSG("hash_insert", "invalid insert");

  return rv;
}

void *
wrap_hash_remove(WRAPPERS_ARGS, hash_t h, const void *key)
{
  void *rv;

  assert(file && function);

  if (!h || !key)
    WRAPPERS_ERR_INVALID_PARAMETERS("hash_remove");

  if (!(rv = hash_remove(h, key)))
    WRAPPERS_ERR_ERRNO("hash_remove");

  return rv;
}

int 
wrap_hash_remove_if(WRAPPERS_ARGS, hash_t h, hash_arg_f argf, void *arg)
{
  int rv;

  assert(file && function);
    
  if (!h || !argf)
    WRAPPERS_ERR_INVALID_PARAMETERS("hash_remove_if");

  if ((rv = hash_remove_if(h, argf, arg)) < 0)
    WRAPPERS_ERR_ERRNO("hash_remove_if");

  return rv;
}

int 
wrap_hash_delete_if(WRAPPERS_ARGS, hash_t h, hash_arg_f argf, void *arg)
{
  int rv;

  assert(file && function);
    
  if (!h || !argf)
    WRAPPERS_ERR_INVALID_PARAMETERS("hash_delete_if");

  if ((rv = hash_delete_if(h, argf, arg)) < 0)
    WRAPPERS_ERR_ERRNO("hash_delete_if");

  return rv;
}

int 
wrap_hash_for_each(WRAPPERS_ARGS, hash_t h, hash_arg_f argf, void *arg)
{
  int rv;

  assert(file && function);

  if (!h || !argf)
    WRAPPERS_ERR_INVALID_PARAMETERS("hash_for_each");

  if ((rv = hash_for_each(h, argf, arg)) < 0)
    WRAPPERS_ERR_ERRNO("hash_for_each");

  return rv;
}

void
wrap_hash_destroy(WRAPPERS_ARGS, hash_t h)
{
  assert(file && function);

  if (!h)
    WRAPPERS_ERR_INVALID_PARAMETERS("hash_destroy");

  hash_destroy(h);
  return;
}

