/*****************************************************************************\
 *  $Id: cerebrod_error.c,v 1.1 2005-03-29 21:30:29 achu Exp $
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
#include <assert.h>
#include <errno.h>

#include "cerebrod_error.h"
#include "cerebrod.h"
#include "error.h"
#include "wrappers.h"

#ifndef NDEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* NDEBUG */

void 
cerebrod_err_debug(const char *fmt, ...)
{
  char buffer[CEREBROD_STRING_BUFLEN];
  va_list ap;

  assert(fmt);

  va_start(ap, fmt);
  vsnprintf(buffer, CEREBROD_STRING_BUFLEN, fmt, ap);
  va_end(ap);

#ifndef NDEBUG
  if (err_get_flags() & ERROR_STDERR)
    {
      Pthread_mutex_lock(&debug_output_mutex);
      err_debug(buffer);
      Pthread_mutex_unlock(&debug_output_mutex);
    }
  else
#endif /* NDEBUG */
    err_debug(buffer);
}
