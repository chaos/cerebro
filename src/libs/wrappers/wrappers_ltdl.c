/*****************************************************************************\
 *  $Id: wrappers_ltdl.c,v 1.3 2007-10-16 22:43:17 chu11 Exp $
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
 *  with Cerebro; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <assert.h>
#include <errno.h>

#include "wrappers.h"

int
wrap_lt_dlinit(WRAPPERS_ARGS)
{
  int rv;

  assert(file && function);

  if ((rv = lt_dlinit()) != 0)
    WRAPPERS_ERR_MSG("lt_dlinit", lt_dlerror());

  return rv;
}

int
wrap_lt_dlexit(WRAPPERS_ARGS)
{
  int rv;

  assert(file && function);

  if ((rv = lt_dlexit()) != 0)
    WRAPPERS_ERR_MSG("lt_dlexit", lt_dlerror());

  return rv;
}

lt_dlhandle
wrap_lt_dlopen(WRAPPERS_ARGS, const char *filename)
{
  lt_dlhandle rv;

  assert(file && function);

  if (!filename)
    WRAPPERS_ERR_INVALID_PARAMETERS("lt_dlopen");

  if (!(rv = lt_dlopen(filename)))
    WRAPPERS_ERR_MSG("lt_dlopen", lt_dlerror());

  return rv;
}

lt_ptr
wrap_lt_dlsym(WRAPPERS_ARGS, void *handle, char *symbol)
{
  lt_ptr *rv;

  assert(file && function);

  if (!handle || !symbol)
    WRAPPERS_ERR_INVALID_PARAMETERS("lt_dlsym");

  /* "clear" lt_dlerror() */
  lt_dlerror();

  if (!(rv = lt_dlsym(handle, symbol)))
    {
      const char *err = lt_dlerror();
      if (err)
        WRAPPERS_ERR_MSG("lt_dlopen", err);
    }

  return rv;
}

int 
wrap_lt_dlclose(WRAPPERS_ARGS, void *handle)
{
  int rv;

  assert(file && function);

  if (!handle)
    WRAPPERS_ERR_INVALID_PARAMETERS("lt_dlclose");

  if ((rv = lt_dlclose(handle)) != 0)
    WRAPPERS_ERR_MSG("lt_dlclose", lt_dlerror());

  return rv;
}

