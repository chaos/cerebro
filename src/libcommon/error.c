/*****************************************************************************\
 *  $Id: error.c,v 1.8 2005-05-04 17:24:05 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <syslog.h>
#include <assert.h>
#include <errno.h>

#include "error.h"

static char *err_prog = NULL;  
static int err_flags = 0;

#define ERROR_BUFLEN   1024

void 
err_init(char *prog)
{
  char *p = strrchr(prog, '/');
  err_prog = p ? p + 1 : prog;
}

int 
err_get_flags(void)
{
  assert(err_prog);

  return err_flags;
}

void 
err_set_flags(int flags)
{
  assert(err_prog);

  err_flags = flags;
}

static void 
_err(int syslog_level, const char *fmt, va_list ap)
{
  char buf[ERROR_BUFLEN];

  assert(err_prog);

  vsnprintf(buf, ERROR_BUFLEN, fmt, ap);
  if (err_flags & ERROR_STDOUT)
    fprintf(stdout, "%s: %s\n", err_prog, buf);
  if (err_flags & ERROR_STDERR)
    fprintf(stderr, "%s: %s\n", err_prog, buf);
  if (err_flags & ERROR_SYSLOG)
    syslog(syslog_level, "%s", buf);
}

void
err_debug(const char *fmt, ...)
{
  va_list ap;

  assert(err_prog);

  va_start(ap, fmt);
  _err(LOG_DEBUG, fmt, ap);
  va_end(ap);
}

void 
err_output(const char *fmt, ...)
{
  va_list ap;

  assert(err_prog);

  va_start(ap, fmt);
  _err(LOG_ERR, fmt, ap);
  va_end(ap);
}

void
err_exit(const char *fmt, ...)
{
  va_list ap;

  assert(err_prog);

  va_start(ap, fmt);
  _err(LOG_ERR, fmt, ap);
  va_end(ap);
  exit(1);
}

/* 
 * lsd_fatal_error
 *
 * for lsd libs like list.[ch] and hash.[ch]
 */
void 
lsd_fatal_error(char *file, int line, char *mesg)
{
  err_exit("LSD FATAL ERROR(%s:%d) %s: %s", file, line, mesg, strerror(errno));
}
