/*****************************************************************************\
 *  $Id: debug.c,v 1.2 2005-06-26 18:39:13 achu Exp $
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
