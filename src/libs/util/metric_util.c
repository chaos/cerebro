/*****************************************************************************\
 *  $Id: metric_util.c,v 1.3 2005-07-19 23:41:13 achu Exp $
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
#include "marshall.h"

int 
_check_metric_type_len(u_int32_t mtype, u_int32_t mlen, const char *caller)
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

  if (_check_metric_type_len(mtype, mlen, caller) < 0)
    return -1;

  if ((!mlen && mvalue) || (mlen && !mvalue))
    {
      CEREBRO_DBG(("%s: invalid metric value", caller));
      return -1;
    }

  return 0;
}

int
_marshall_metric(u_int32_t mtype,
                 u_int32_t mlen,
                 void *mvalue,
                 char *buf,
                 unsigned int buflen,
                 int *errnum,
                 const char *caller)
{
  int n, c = 0;
  
  if (!buf || !caller)
    {
      CEREBRO_DBG(("%s: invalid parameters", caller));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (_check_metric_type_len_value(mtype, mlen, mvalue, caller) < 0)
    {
      if (errnum)
        *errnum = CEREBRO_ERR_PARAMETERS;
      return -1;
    }

  if ((n = marshall_u_int32(mtype, buf + c, buflen - c)) <= 0)
    {
      CEREBRO_DBG(("%s: marshall_u_int32", caller));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += n;
                                                                                     
  if ((n = marshall_u_int32(mlen, buf + c, buflen - c)) <= 0)
    {
      CEREBRO_DBG(("%s: marshall_u_int32", caller));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += n;

  if (!mlen)
    return c;

  if (mtype == CEREBRO_METRIC_VALUE_TYPE_INT32)
    {
      if ((n = marshall_int32(*((int32_t *)mvalue), buf + c, buflen - c)) <= 0)
        {
          CEREBRO_DBG(("%s: marshall_int32", caller));
          if (errnum)
            *errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_U_INT32)
    {
      if ((n = marshall_u_int32(*((u_int32_t *)mvalue), buf + c, buflen - c)) <= 0)
        {
          CEREBRO_DBG(("%s: marshall_u_int32", caller));
          if (errnum)
            *errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_FLOAT)
    {
      if ((n = marshall_float(*((float *)mvalue), buf + c, buflen - c)) <= 0)
        {
          CEREBRO_DBG(("%s: marshall_float", caller));
          if (errnum)
            *errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_DOUBLE)
    {
      if ((n = marshall_double(*((double *)mvalue), buf + c, buflen - c)) <= 0)
        {
          CEREBRO_DBG(("%s: marshall_double", caller));
          if (errnum)
            *errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_STRING)
    {
      if ((n = marshall_buffer(mvalue, mlen, buf + c, buflen - c)) <= 0)
        {
          CEREBRO_DBG(("%s: marshall_buffer", caller));
          if (errnum)
            *errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }
  else
    {
      CEREBRO_DBG(("%s: invalid type %d", caller, mtype));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  c += n;

  return c;
}

#if 0
static int
_unmarshall_metric_value(u_int32_t mtype,
                         u_int32_t mlen,
                         void *metric_value,
                         unsigned int metric_value_len,
                         const char *buf,
                         unsigned int buflen,
                         int *errnum,
                         const char *caller)
{
  int n, malloc_len = 0;
  void *mvalue = NULL;
  u_int32_t mtype, mlen;
                                                                                     
  if (mtype == CEREBRO_METRIC_VALUE_TYPE_NONE
      || !mlen
      || !metric_value
      || !(metric_value_len < mlen)
      || !buf
      || !(buflen >= mlen)
      || !caller)
    {
      CEREBRO_DBG(("invalid parameters"));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (mtype == CEREBRO_METRIC_VALUE_TYPE_INT32)
    {
      if ((n = unmarshall_int32((int32_t *)mvalue, buf, buflen)) < 0)
        {
          CEREBRO_DBG(("unmarshall_int32"));
          goto cleanup;
        }
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_U_INT32)
    {
      if ((n = unmarshall_u_int32((u_int32_t *)mvalue, buf, buflen)) < 0)
        {
          CEREBRO_DBG(("unmarshall_u_int32"));
          goto cleanup;
        }
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_FLOAT)
    {
      if ((n = unmarshall_float((float *)mvalue, buf, buflen)) < 0)
        {
          CEREBRO_DBG(("unmarshall_float"));
          goto cleanup;
        }
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_DOUBLE)
    {
      if ((n = unmarshall_double((double *)mvalue, buf, buflen)) < 0)
        {
          CEREBRO_DBG(("unmarshall_double"));
          goto cleanup;
        }
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_STRING)
    {
      if ((n = unmarshall_buffer((char *)mvalue, mlen, buf, buflen)) < 0)
        {
          CEREBRO_DBG(("unmarshall_buffer"));
          goto cleanup;
        }
    }
  else
    {
      /* If an invalid param, should have been caught before here */
      CEREBRO_DBG(("invalid type %d", mtype));
      handle->errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }

  if (n != buflen)
    {
      CEREBRO_DBG(("received invalid metric value buflen"));
      handle->errnum = CEREBRO_ERR_PROTOCOL;
      goto cleanup;
    }
                                                                                     
  *metric_value = mvalue;
  return buflen;
                                                                                     
 cleanup:
  free(mvalue);
  return -1;
}
#endif
