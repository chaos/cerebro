/*****************************************************************************\
 *  $Id: cerebro_util.c,v 1.2 2005-04-26 17:04:29 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>

#include "cerebro.h"
#include "cerebro_api.h"

int 
cerebro_handle_check(cerebro_t handle)
{
  if (!handle)
    {
      handle->errnum = CEREBRO_ERR_NULLHANDLE;
      return -1;
    }

  if (handle->magic != CEREBRO_MAGIC_NUMBER)
    {
      handle->errnum = CEREBRO_ERR_MAGIC_NUMBER;
      return -1;
    }

  handle->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}
