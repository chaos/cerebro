/*****************************************************************************\
 *  $Id: cerebro_error.c,v 1.2 2005-04-25 21:39:27 achu Exp $
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
#include <pthread.h>

#include "cerebro_error.h"
#include "error.h"
#include "wrappers.h"

#define CEREBRO_ERROR_STRING_BUFLEN 1024

/*  
 * error_output_mutex
 *
 * If set, requires locking before output of stdout or stderr messages
 */
pthread_mutex_t *error_output_mutex = NULL;

void 
cerebro_err_init(char *prog)
{
  if (!prog)
    return;
  err_init(prog);
}

void 
cerebro_err_register_mutex(pthread_mutex_t *mutex)
{
  error_output_mutex = mutex;
}

int 
cerebro_err_get_flags(void)
{
  int flags = 0;
  int cerebro_flags = 0;

  flags = err_get_flags();

  if (flags & ERROR_STDOUT)
    cerebro_flags |= CEREBRO_ERROR_STDOUT;
  if (flags & ERROR_STDERR)
    cerebro_flags |= CEREBRO_ERROR_STDERR;
  if (flags & ERROR_SYSLOG)
    cerebro_flags |= CEREBRO_ERROR_SYSLOG;
  
  return cerebro_flags;
}

void 
cerebro_err_set_flags(int flags)
{
  int err_flags = 0;

  if (flags & CEREBRO_ERROR_STDOUT)
    err_flags |= ERROR_STDOUT;
  if (flags & CEREBRO_ERROR_STDERR)
    err_flags |= ERROR_STDERR;
  if (flags & CEREBRO_ERROR_SYSLOG)
    err_flags |= ERROR_SYSLOG;

  err_set_flags(err_flags);
}

void 
cerebro_err_debug(const char *fmt, ...)
{
  char buffer[CEREBRO_ERROR_STRING_BUFLEN];
  int flags;
  va_list ap;

  if (!fmt)
    return;

  va_start(ap, fmt);
  vsnprintf(buffer, CEREBRO_ERROR_STRING_BUFLEN, fmt, ap);
  va_end(ap);

  flags = cerebro_err_get_flags();
  if (error_output_mutex
      && ((flags & CEREBRO_ERROR_STDOUT)
          || (flags & CEREBRO_ERROR_STDERR)))
    {
      Pthread_mutex_lock(error_output_mutex);
      err_debug(buffer);
      Pthread_mutex_unlock(error_output_mutex);
    }
  else
    err_debug(buffer);
}

void 
cerebro_err_output(const char *fmt, ...)
{
  char buffer[CEREBRO_ERROR_STRING_BUFLEN];
  int flags;
  va_list ap;

  if (!fmt)
    return;

  va_start(ap, fmt);
  vsnprintf(buffer, CEREBRO_ERROR_STRING_BUFLEN, fmt, ap);
  va_end(ap);

  flags = cerebro_err_get_flags();
  if (error_output_mutex
      && ((flags & CEREBRO_ERROR_STDOUT)
          || (flags & CEREBRO_ERROR_STDERR)))
    {
      Pthread_mutex_lock(error_output_mutex);
      err_output(buffer);
      Pthread_mutex_unlock(error_output_mutex);
    }
  else
    err_output(buffer);
}

void 
cerebro_err_exit(const char *fmt, ...)
{
  char buffer[CEREBRO_ERROR_STRING_BUFLEN];
  int flags;
  va_list ap;

  if (!fmt)
    err_exit("cerebro_err_exit called with null format");

  va_start(ap, fmt);
  vsnprintf(buffer, CEREBRO_ERROR_STRING_BUFLEN, fmt, ap);
  va_end(ap);

  flags = cerebro_err_get_flags();
  if (error_output_mutex
      && ((flags & CEREBRO_ERROR_STDOUT)
          || (flags & CEREBRO_ERROR_STDERR)))
    {
      Pthread_mutex_lock(error_output_mutex);
      err_exit(buffer);
      /* NOT REACHED */
      Pthread_mutex_unlock(error_output_mutex);
    }
  else
    err_exit(buffer);
}
