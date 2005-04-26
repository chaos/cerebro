/*****************************************************************************\
 *  $Id: cerebro_util.c,v 1.1 2005-04-26 00:09:13 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>

#include "cerebro.h"
#include "cerebro_defs.h"

int 
cerebro_handle_err_check(cerebro_t handle)
{
  if (!handle)
    {
      handle->errnum = CEREBRO_ERR_NULLHANDLE;
      return -1;
    }

  if (handle->magic != CEREBRO_MAGIC)
    {
      handle->errnum = CEREBRO_ERR_MAGIC;
      return -1;
    }

  handle->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}
