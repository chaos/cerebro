/*****************************************************************************\
 *  $Id: cerebro_util.c,v 1.8 2007-09-05 18:15:56 chu11 Exp $
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

#include <stdio.h>
#include <stdlib.h>

#include "cerebro.h"
#include "cerebro_api.h"
#include "cerebro_util.h"

#include "debug.h"

int 
_cerebro_handle_check(cerebro_t handle)
{
  if (!handle || handle->magic != CEREBRO_MAGIC_NUMBER)
    return -1;

  if (!handle->namelists)
    {
      CEREBRO_DBG(("namelists null"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (!handle->nodelists)
    {
      CEREBRO_DBG(("nodelists null"));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  
  return 0;
}

