/*****************************************************************************\
 *  $Id: cerebro_error.c,v 1.8 2005-05-05 22:24:59 achu Exp $
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

/*  
 * If set, requires locking before output of stdout or stderr messages
 */
static Cerebro_err_lock cerebro_err_lock = NULL;
static Cerebro_err_unlock cerebro_err_unlock = NULL;

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

void 
cerebro_err_register_locking(Cerebro_err_lock lock,
			     Cerebro_err_unlock unlock)
{
  if (!lock || !unlock)
    return;

  cerebro_err_lock = lock;
  cerebro_err_unlock = unlock;
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
  int flags;
  va_list ap;

  if (!cerebro_err_initialized)
    return; 

  if (!fmt)
    return;

  va_start(ap, fmt);
  vsnprintf(buffer, CEREBRO_ERROR_STRING_BUFLEN, fmt, ap);
  va_end(ap);

  flags = cerebro_err_get_flags();
  if ((cerebro_err_lock 
       && cerebro_err_unlock)
      && 
      ((flags & CEREBRO_ERROR_STDOUT)
       || (flags & CEREBRO_ERROR_STDERR)))
    {
      (*cerebro_err_lock)();
      err_debug(buffer);
      (*cerebro_err_unlock)();
    }
  else
    err_debug(buffer);
}

void 
cerebro_err_debug_lib(const char *fmt, ...)
{
  char buffer[CEREBRO_ERROR_STRING_BUFLEN];
  int flags;
  va_list ap;

  if (!cerebro_err_initialized)
    return; 

  if (!fmt)
    return;

  if (!(cerebro_flags & CEREBRO_ERROR_LIB))
    return;

  va_start(ap, fmt);
  vsnprintf(buffer, CEREBRO_ERROR_STRING_BUFLEN, fmt, ap);
  va_end(ap);

  flags = cerebro_err_get_flags();
  if ((cerebro_err_lock 
       && cerebro_err_unlock)
      && 
      ((flags & CEREBRO_ERROR_STDOUT)
       || (flags & CEREBRO_ERROR_STDERR)))
    {
      (*cerebro_err_lock)();
      err_debug(buffer);
      (*cerebro_err_unlock)();
    }
  else
    err_debug(buffer);
}

void 
cerebro_err_debug_module(const char *fmt, ...)
{
  char buffer[CEREBRO_ERROR_STRING_BUFLEN];
  int flags;
  va_list ap;

  if (!cerebro_err_initialized)
    return; 

  if (!fmt)
    return;

  if (!(cerebro_flags & CEREBRO_ERROR_MODULE))
    return;

  va_start(ap, fmt);
  vsnprintf(buffer, CEREBRO_ERROR_STRING_BUFLEN, fmt, ap);
  va_end(ap);

  flags = cerebro_err_get_flags();
  if ((cerebro_err_lock 
       && cerebro_err_unlock)
      && 
      ((flags & CEREBRO_ERROR_STDOUT)
       || (flags & CEREBRO_ERROR_STDERR)))
    {
      (*cerebro_err_lock)();
      err_debug(buffer);
      (*cerebro_err_unlock)();
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

  if (!cerebro_err_initialized)
    return; 

  if (!fmt)
    return;

  va_start(ap, fmt);
  vsnprintf(buffer, CEREBRO_ERROR_STRING_BUFLEN, fmt, ap);
  va_end(ap);

  flags = cerebro_err_get_flags();
  if ((cerebro_err_lock 
       && cerebro_err_unlock)
      && 
      ((flags & CEREBRO_ERROR_STDOUT)
       || (flags & CEREBRO_ERROR_STDERR)))
    {
      (*cerebro_err_lock)();
      err_output(buffer);
      (*cerebro_err_unlock)();
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

  if (!cerebro_err_initialized)
    return; 

  if (!fmt)
    return;

  va_start(ap, fmt);
  vsnprintf(buffer, CEREBRO_ERROR_STRING_BUFLEN, fmt, ap);
  va_end(ap);

  flags = cerebro_err_get_flags();
  if ((cerebro_err_lock 
       && cerebro_err_unlock)
      && 
      ((flags & CEREBRO_ERROR_STDOUT)
       || (flags & CEREBRO_ERROR_STDERR)))
    {
      (*cerebro_err_lock)();
      err_exit(buffer);
      /* NOT REACHED */
      (*cerebro_err_unlock)();
    }
  else
    err_exit(buffer);
}
