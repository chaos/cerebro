/*****************************************************************************\
 *  $Id: metric_util.c,v 1.1 2005-07-19 20:18:35 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"

#include "metric_util.h"

#include "debug.h"

int 
_check_metric_type_and_len(u_int32_t mtype, u_int32_t mlen, const char *caller)
{
  if (!caller)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  if (mtype == CEREBRO_METRIC_VALUE_TYPE_NONE && mlen)
    {
      CEREBRO_DBG(("%s: metric value len > 0 for type NONE", caller));
      return -1;
    }

  if ((mtype == CEREBRO_METRIC_VALUE_TYPE_INT32 && mlen != sizeof(int32_t))
      || (mtype == CEREBRO_METRIC_VALUE_TYPE_U_INT32 && mlen != sizeof(u_int32_t))
      || (mtype == CEREBRO_METRIC_VALUE_TYPE_FLOAT && mlen != sizeof(float))
      || (mtype == CEREBRO_METRIC_VALUE_TYPE_DOUBLE && mlen != sizeof(double)))
    {
      CEREBRO_DBG(("%s: invalid metric len", caller));
      return -1;
    }

  if (mtype == CEREBRO_METRIC_VALUE_TYPE_STRING && !mlen)
    {
      CEREBRO_DBG(("%s: empty string len", caller));
      return -1;
    }

  if (mtype == CEREBRO_METRIC_VALUE_TYPE_STRING 
      && mlen > CEREBRO_MAX_METRIC_STRING_LEN)
    {
      CEREBRO_DBG(("%s: string len too long", caller));
      return -1;
    }

  return 0;

}

int 
_check_metric_type_len_value(u_int32_t mtype, 
                             u_int32_t mlen, 
                             void *mvalue,
                             const char *caller)
{
  if (!caller)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  if (_check_metric_type_and_len(mtype, mlen, caller) < 0)
    return -1;

  if ((!mlen && mvalue) || (mlen && !mvalue))
    {
      CEREBRO_DBG(("invalid metric value"));
      return -1;
    }

  return 0;
}
