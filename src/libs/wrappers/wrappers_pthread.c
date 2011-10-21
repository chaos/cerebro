/*****************************************************************************\
 *  $Id: wrappers_pthread.c,v 1.6 2010-02-02 01:01:21 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2011 Lawrence Livermore National Security, LLC.
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

int 
wrap_pthread_create(WRAPPERS_ARGS, pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *), void *arg)
{
  int rv;
  
  assert(file && function);

  if (!thread || !start_routine)
    WRAPPERS_ERR_INVALID_PARAMETERS("pthread_create");

  if ((rv = pthread_create(thread, attr, start_routine, arg)) != 0)
    WRAPPERS_ERR_MSG("pthread_create", strerror(rv));

  return rv;
}

int 
wrap_pthread_attr_init(WRAPPERS_ARGS, pthread_attr_t *attr)
{
  int rv;
  
  assert(file && function);

  if (!attr)
    WRAPPERS_ERR_INVALID_PARAMETERS("pthread_attr_init");

  if ((rv = pthread_attr_init(attr)) != 0)
    WRAPPERS_ERR_MSG("pthread_attr_init", strerror(rv));

  return rv;
}

int 
wrap_pthread_attr_destroy(WRAPPERS_ARGS, pthread_attr_t *attr)
{
  int rv;
  
  assert(file && function);

  if (!attr)
    WRAPPERS_ERR_INVALID_PARAMETERS("pthread_attr_destroy");

  if ((rv = pthread_attr_destroy(attr)) != 0)
    WRAPPERS_ERR_MSG("pthread_attr_destroy", strerror(rv));

  return rv;
}

int 
wrap_pthread_attr_setdetachstate(WRAPPERS_ARGS, pthread_attr_t *attr, int detachstate)
{
  int rv;
  
  assert(file && function);

  if (!attr)
    WRAPPERS_ERR_INVALID_PARAMETERS("pthread_attr_setdetachstate");

  if ((rv = pthread_attr_setdetachstate(attr, detachstate)) != 0)
    WRAPPERS_ERR_MSG("pthread_attr_setdetachstate", strerror(rv));

  return rv;
}

int 
wrap_pthread_attr_setstacksize(WRAPPERS_ARGS, pthread_attr_t *attr, size_t stacksize)
{
  int rv;
  
  assert(file && function);

  if (!attr || stacksize < PTHREAD_STACK_MIN)
    WRAPPERS_ERR_INVALID_PARAMETERS("pthread_attr_setstacksize");

  if ((rv = pthread_attr_setstacksize(attr, stacksize)) != 0)
    WRAPPERS_ERR_MSG("pthread_attr_setstacksize", strerror(rv));

  return rv;
}

int
wrap_pthread_mutex_lock(WRAPPERS_ARGS, pthread_mutex_t *mutex)
{
  int rv;
  
  assert(file && function);

  if (!mutex)
    WRAPPERS_ERR_INVALID_PARAMETERS("pthread_mutex_lock");

  if ((rv = pthread_mutex_lock(mutex)) != 0)
    WRAPPERS_ERR_MSG("pthread_mutex_lock", strerror(rv));

  return rv;
}

int 
wrap_pthread_mutex_trylock(WRAPPERS_ARGS, pthread_mutex_t *mutex)
{
  int rv;
  
  assert(file && function);

  if (!mutex)
    WRAPPERS_ERR_INVALID_PARAMETERS("pthread_mutex_trylock");

  rv = pthread_mutex_trylock(mutex);
  if (rv != 0 && rv != EBUSY)
    WRAPPERS_ERR_MSG("pthread_mutex_trylock", strerror(rv));

  return rv;
}

int 
wrap_pthread_mutex_unlock(WRAPPERS_ARGS, pthread_mutex_t *mutex)
{
  int rv;
  
  assert(file && function);

  if (!mutex)
    WRAPPERS_ERR_INVALID_PARAMETERS("pthread_mutex_unlock");

  if ((rv = pthread_mutex_unlock(mutex)) != 0)
    WRAPPERS_ERR_MSG("pthread_mutex_unlock", strerror(rv));

  return rv;
}

int
wrap_pthread_mutex_init(WRAPPERS_ARGS, pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr)
{
  int rv;
  
  assert(file && function);

  if (!mutex)
    WRAPPERS_ERR_INVALID_PARAMETERS("pthread_mutex_init");

  if ((rv = pthread_mutex_init(mutex, mutexattr)) != 0)
    WRAPPERS_ERR_MSG("pthread_mutex_init", strerror(rv));

  return rv;
}

int 
wrap_pthread_cond_signal(WRAPPERS_ARGS, pthread_cond_t *cond)
{
  int rv;

  assert(file && function);

  if (!cond)
    WRAPPERS_ERR_INVALID_PARAMETERS("pthread_cond_signal");

  if ((rv = pthread_cond_signal(cond)) != 0)
    WRAPPERS_ERR_MSG("pthread_cond_signal", strerror(rv));

  return rv;
}

int 
wrap_pthread_cond_wait(WRAPPERS_ARGS, pthread_cond_t *cond, pthread_mutex_t *mutex)
{
  int rv;

  assert(file && function);

  if (!cond || !mutex)
    WRAPPERS_ERR_INVALID_PARAMETERS("pthread_cond_wait");

  if ((rv = pthread_cond_wait(cond, mutex)) != 0)
    WRAPPERS_ERR_MSG("pthread_cond_signal", strerror(rv));

  return rv;
}
