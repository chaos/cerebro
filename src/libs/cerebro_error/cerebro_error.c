/*****************************************************************************\
 *  $Id: cerebro_error.c,v 1.3 2005-07-22 17:21:07 achu Exp $
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
#include <stdarg.h>
#endif /* STDC_HEADERS */
#include <errno.h>

#include "cerebro/cerebro_error.h"
#include "error.h"

#define CEREBRO_ERROR_STRING_BUFLEN 4096

static int cerebro_err_initialized = 0;

static int cerebro_flags = 0;

void 
cerebro_err_init(char *prog)
{
  if (!prog)
    return;

  err_init(prog);

  cerebro_err_initialized++;
}

int 
cerebro_err_get_flags(void)
{
  return cerebro_flags;
}

void 
cerebro_err_set_flags(int flags)
{
  int err_flags = 0;

  cerebro_flags = flags;

  if (cerebro_flags & CEREBRO_ERROR_STDOUT)
    err_flags |= ERROR_STDOUT;
  if (cerebro_flags & CEREBRO_ERROR_STDERR)
    err_flags |= ERROR_STDERR;
  if (cerebro_flags & CEREBRO_ERROR_SYSLOG)
    err_flags |= ERROR_SYSLOG;

  err_set_flags(err_flags);
}

void 
cerebro_err_debug(const char *fmt, ...)
{
  char buffer[CEREBRO_ERROR_STRING_BUFLEN];
  va_list ap;

  if (!cerebro_err_initialized)
    return; 

  if (!fmt)
    return;

  va_start(ap, fmt);
  vsnprintf(buffer, CEREBRO_ERROR_STRING_BUFLEN, fmt, ap);
  va_end(ap);

  err_debug(buffer);
}

void 
cerebro_err_output(const char *fmt, ...)
{
  char buffer[CEREBRO_ERROR_STRING_BUFLEN];
  va_list ap;

  if (!cerebro_err_initialized)
    return; 

  if (!fmt)
    return;

  va_start(ap, fmt);
  vsnprintf(buffer, CEREBRO_ERROR_STRING_BUFLEN, fmt, ap);
  va_end(ap);

  err_output(buffer);
}

void 
cerebro_err_exit(const char *fmt, ...)
{
  char buffer[CEREBRO_ERROR_STRING_BUFLEN];
  va_list ap;

  if (!cerebro_err_initialized)
    return; 

  if (!fmt)
    return;

  va_start(ap, fmt);
  vsnprintf(buffer, CEREBRO_ERROR_STRING_BUFLEN, fmt, ap);
  va_end(ap);

  err_exit(buffer);
}
