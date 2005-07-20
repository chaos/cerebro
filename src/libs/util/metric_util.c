/*****************************************************************************\
 *  $Id: metric_util.c,v 1.6 2005-07-20 21:17:48 achu Exp $
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

int 
_unmarshall_metric_type_len(u_int32_t *mtype,
                            u_int32_t *mlen,
                            const char *buf,
                            unsigned int buflen,
                            int *errnum,
                            const char *caller)
{
  int n, c = 0;

  if (!mtype || !mlen || !buf || !caller)
    {
      CEREBRO_DBG(("%s: invalid parameters", caller));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (buflen < 2*sizeof(u_int32_t))
    {
      CEREBRO_DBG(("%s: invalid packet size", caller));
      if (errnum)
        *errnum = CEREBRO_ERR_PROTOCOL;
      return -1;
    }

  if ((n = unmarshall_u_int32(mtype, buf + c,  buflen - c)) < 0)
    {
      CEREBRO_DBG(("unmarshall_u_int32"));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += n;
                                                                                     
  if ((n = unmarshall_u_int32(mlen, buf + c, buflen - c)) < 0)
    {
      CEREBRO_DBG(("unmarshall_u_int32"));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += n;

  if (c != 2*sizeof(u_int32_t))
    {
      /* Internal error b/c this should have been caught earlier */
      CEREBRO_DBG(("%s: unfinished unmarshalling", caller));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  return c;
}

int 
_unmarshall_metric_value(u_int32_t mtype,
                         u_int32_t mlen,
                         void *mvalue,
                         unsigned int mvalue_len,
                         const char *buf,
                         unsigned int buflen,
                         int *errnum,
                         const char *caller)
{
  int n = -1;

  if (mlen < mvalue_len || !buf || !caller)
    {
      CEREBRO_DBG(("%s: invalid parameters", caller));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (_check_metric_type_len(mtype, mlen, caller) < 0)
    {
      if (errnum)
        *errnum = CEREBRO_ERR_PROTOCOL;
      return -1;
    }
  
  if (!mlen)
    return 0;

  if (buflen < mlen)
    {
      CEREBRO_DBG(("%s: invalid packet size", caller));
      if (errnum)
        *errnum = CEREBRO_ERR_PROTOCOL;
      return -1;
    }

  if (mtype == CEREBRO_METRIC_VALUE_TYPE_INT32)
    {
      if ((n = unmarshall_int32((int32_t *)mvalue, buf, buflen)) < 0)
        CEREBRO_DBG(("%s: unmarshall_int32", caller));
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_U_INT32)
    {
      if ((n = unmarshall_u_int32((u_int32_t *)mvalue, buf, buflen)) < 0)
        CEREBRO_DBG(("%s: unmarshall_u_int32", caller));
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_FLOAT)
    {
      if ((n = unmarshall_float((float *)mvalue, buf, buflen)) < 0)
        CEREBRO_DBG(("%s: unmarshall_float", caller));
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_DOUBLE)
    {
      if ((n = unmarshall_double((double *)mvalue, buf, buflen)) < 0)
        CEREBRO_DBG(("%s: unmarshall_double", caller));
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_STRING)
    {
      if ((n = unmarshall_buffer((char *)mvalue, mlen, buf, buflen)) < 0)
        CEREBRO_DBG(("%s: unmarshall_buffer", caller));
    }
  else
    /* If an invalid param, should have been caught before here */
    CEREBRO_DBG(("%s: invalid type %d", caller, mtype));
  
  if (n < 0)
    {
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (n != mlen)
    {
      /* Internal error b/c this should have been caught earlier */
      CEREBRO_DBG(("%s: unfinished unmarshalling", caller));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  
  return n;
}
