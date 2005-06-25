/*****************************************************************************\
 *  $Id: error_util.c,v 1.3 2005-06-25 00:01:48 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if CEREBRO_DEBUG

#if STDC_HEADERS
#include <stdarg.h>
#endif /* STDC_HEADERS */

#include "error_util.h"

char *
error_msg_create(const char *fmt, ...)
{
  char *buffer;
  va_list ap;

  if (!fmt)
    return NULL;

  if (!(buffer = malloc(ERROR_BUFFER_LEN)))
    return NULL;

  va_start(ap, fmt);
  vsnprintf(buffer, ERROR_BUFFER_LEN, fmt, ap);
  va_end(ap);

  return buffer;
}

#endif /* CEREBRO_DEBUG */
