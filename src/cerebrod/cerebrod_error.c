/*****************************************************************************\
 *  $Id: cerebrod_error.c,v 1.2 2005-03-30 05:41:45 achu Exp $
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
#include "wrappers.h"

#ifndef NDEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* NDEBUG */

void 
cerebrod_err_init(char *prog)
{
  assert(prog);
  err_init(prog);
}

int 
cerebrod_err_get_flags(void)
{
  return err_get_flags();
}

void 
cerebrod_err_set_flags(int flags)
{
  err_set_flags(flags);
}

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

void 
cerebrod_err_output(const char *fmt, ...)
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
      err_output(buffer);
      Pthread_mutex_unlock(&debug_output_mutex);
    }
  else
#endif /* NDEBUG */
    err_output(buffer);
}

void 
cerebrod_err_exit(const char *fmt, ...)
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
      err_exit(buffer);
      /* NOT REACHED */
      Pthread_mutex_unlock(&debug_output_mutex);
    }
  else
#endif /* NDEBUG */
    err_exit(buffer);
}
