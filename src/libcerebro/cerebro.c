/*****************************************************************************\
 *  $Id: cerebro.c,v 1.3 2005-04-26 17:04:29 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */

#include "cerebro.h"
#include "cerebro_api.h"
#include "cerebro_util.h"
#include "cerebro_updown.h"

char *cerebro_error_messages[] =
  {
    "success",
    "null cerebro_t handle",
    "invalid magic number",
    "invalid parameters",
    "server data not loaded",
    "out of memory",
    "internal error",
    "errnum out of range",
  };

cerebro_t
cerebro_handle_create(void)
{
  cerebro_t handle = NULL;

  if (!(handle = (cerebro_t)malloc(sizeof(struct cerebro))))
    goto cleanup;
                 
  memset(handle, '\0', sizeof(struct cerebro));
  handle->magic = CEREBRO_MAGIC_NUMBER;
  handle->errnum = CEREBRO_ERR_SUCCESS;
  handle->loaded_state = 0;
  handle->updown_data = NULL;

 cleanup:
  return handle;
}

int
cerebro_handle_destroy(cerebro_t handle)
{
  if (cerebro_handle_check(handle) < 0)
    return -1;

  if (handle->loaded_state & CEREBRO_UPDOWN_DATA_LOADED)
    {
      if (cerebro_updown_unload_data(handle) < 0)
        return -1;

      if (handle->loaded_state & CEREBRO_UPDOWN_DATA_LOADED)
        {
          handle->errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }

  handle->errnum = CEREBRO_ERR_SUCCESS;
  handle->magic = ~CEREBRO_MAGIC_NUMBER;
  free(handle);
  return 0;
}

int
cerebro_errnum(cerebro_t handle)
{
  if (!handle)
    return CEREBRO_ERR_NULLHANDLE;
  else if (handle->magic != CEREBRO_MAGIC_NUMBER)
    return CEREBRO_ERR_MAGIC_NUMBER;
  else
    return handle->errnum;
}

char *
cerebro_strerror(int errnum)
{
  if (errnum >= CEREBRO_ERR_SUCCESS && errnum <= CEREBRO_ERR_ERRNUMRANGE)
    return cerebro_error_messages[errnum];
  else
    return cerebro_error_messages[CEREBRO_ERR_ERRNUMRANGE];
}

char *
cerebro_errormsg(cerebro_t handle)
{
  return cerebro_strerror(cerebro_errnum(handle));
}

void
cerebro_perror(cerebro_t handle, const char *msg)
{
  char *errormsg = cerebro_strerror(cerebro_errnum(handle));
 
  if (!msg)
    fprintf(stderr, "%s\n", errormsg);
  else
    fprintf(stderr, "%s: %s\n", msg, errormsg);
}
