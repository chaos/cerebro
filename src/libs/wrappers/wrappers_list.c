/*****************************************************************************\
 *  $Id: wrappers_list.c,v 1.8 2010-04-06 22:10:11 chu11 Exp $
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

#include <assert.h>
#include <errno.h>

#include "wrappers.h"

List 
wrap_list_create(WRAPPERS_ARGS, ListDelF f)
{
  List rv;

  assert(file && function);

  if (!(rv = list_create(f)))
    WRAPPERS_ERR_ERRNO("list_create");

  return rv;
}

void
wrap_list_destroy(WRAPPERS_ARGS, List l)
{
  assert(file && function);

  if (!l)
    WRAPPERS_ERR_INVALID_PARAMETERS("list_destroy");

  list_destroy(l);

  return;
}

int
wrap_list_count(WRAPPERS_ARGS, List l)
{
  assert(file && function);

  if (!l)
    WRAPPERS_ERR_INVALID_PARAMETERS("list_count");

  return list_count(l);
}

void *
wrap_list_append (WRAPPERS_ARGS, List l, void *x)
{
  void *rv;

  assert(file && function);

  if (!l || !x)
    WRAPPERS_ERR_INVALID_PARAMETERS("list_append");

  if (!(rv = list_append(l, x)))
    WRAPPERS_ERR_ERRNO("list_append");
  
  return rv;
}

void * 
wrap_list_find_first (WRAPPERS_ARGS, List l, ListFindF f, void *key)
{
  assert(file && function);

  if (!l || !f || !key)
    WRAPPERS_ERR_INVALID_PARAMETERS("list_find_first");

  return list_find_first(l, f, key);
}

int 
wrap_list_delete_all(WRAPPERS_ARGS, List l, ListFindF f, void *key)
{
  int rv;

  assert(file && function);

  if (!l || !f || !key)
    WRAPPERS_ERR_INVALID_PARAMETERS("list_delete_all");

  if ((rv = list_delete_all(l, f, key)) < 0)
    WRAPPERS_ERR_ERRNO("list_delete_all");
  
  return rv;
}

int
wrap_list_for_each(WRAPPERS_ARGS, List l, ListForF f, void *arg)
{
  int rv;

  assert(file && function);

  if (!l || !f)
    WRAPPERS_ERR_INVALID_PARAMETERS("list_for_each");

  if ((rv = list_for_each(l, f, arg)) < 0)
    WRAPPERS_ERR_ERRNO("list_for_each");
  
  return rv;
}

void
wrap_list_sort(WRAPPERS_ARGS, List l, ListCmpF f)
{
  assert(file && function);

  if (!l || !f)
    WRAPPERS_ERR_INVALID_PARAMETERS("list_sort");

  list_sort(l, f);
  return;
}

void *
wrap_list_peek(WRAPPERS_ARGS, List l)
{
  void *rv;

  assert(file && function);

  if (!l)
    WRAPPERS_ERR_INVALID_PARAMETERS("list_peek");
  
  rv = list_peek(l);

  return rv;
}

ListIterator
wrap_list_iterator_create(WRAPPERS_ARGS, List l)
{
  ListIterator rv;

  assert(file && function);

  if (!l)
    WRAPPERS_ERR_INVALID_PARAMETERS("list_iterator_create");

  if (!(rv = list_iterator_create(l)))
    WRAPPERS_ERR_ERRNO("list_iterator_create");

  return rv;
}

void
wrap_list_iterator_destroy(WRAPPERS_ARGS, ListIterator i)
{
  assert(file && function);

  if (!i)
    WRAPPERS_ERR_INVALID_PARAMETERS("list_iterator_destroy");

  list_iterator_destroy(i);
  return;
}

void *
wrap_list_remove(WRAPPERS_ARGS, ListIterator i)
{
  int rv;

  assert(file && function);

  if (!i)
    WRAPPERS_ERR_INVALID_PARAMETERS("list_remove");

  if (!list_remove(i))
    WRAPPERS_ERR_ERRNO("list_remove");

  return rv;
}

int 
wrap_list_delete(WRAPPERS_ARGS, ListIterator i)
{
  int rv;

  assert(file && function);

  if (!i)
    WRAPPERS_ERR_INVALID_PARAMETERS("list_delete");

  if ((rv = list_delete(i)) != 1)
    WRAPPERS_ERR_ERRNO("list_delete");

  return rv;
}

