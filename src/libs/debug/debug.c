/*****************************************************************************\
 *  $Id: debug.c,v 1.3 2005-07-22 17:21:07 achu Exp $
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

#if CEREBRO_DEBUG

#if STDC_HEADERS
#include <stdarg.h>
#endif /* STDC_HEADERS */

#include "debug.h"

char *
debug_msg_create(const char *fmt, ...)
{
  char *buffer;
  va_list ap;

  if (!fmt)
    return NULL;

  if (!(buffer = malloc(DEBUG_BUFFER_LEN)))
    return NULL;

  va_start(ap, fmt);
  vsnprintf(buffer, DEBUG_BUFFER_LEN, fmt, ap);
  va_end(ap);

  return buffer;
}

#endif /* CEREBRO_DEBUG */
