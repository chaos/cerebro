/*****************************************************************************\
 *  $Id: cerebro_error.c,v 1.11 2005-06-16 17:17:16 achu Exp $
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

#define CEREBRO_ERROR_STRING_BUFLEN 1024

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
