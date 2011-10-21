/*****************************************************************************\
 *  $Id: data_util.c,v 1.8 2010-02-02 01:01:21 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2011 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2005-2007 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <http://www.llnl.gov/linux/cerebro/>.
 *
 *  Cerebro is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  Cerebro is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with Cerebro. If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"

#include "data_util.h"

#include "debug.h"
#include "marshall.h"

int 
_check_data_type_len(u_int32_t dtype, u_int32_t dlen, const char *caller)
{
  if (!caller)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  if (dtype == CEREBRO_DATA_VALUE_TYPE_NONE && dlen)
    {
      CEREBRO_DBG(("%s: data value len > 0 for type NONE", caller));
      return -1;
    }

  if ((dtype == CEREBRO_DATA_VALUE_TYPE_INT32 && dlen != sizeof(int32_t))
      || (dtype == CEREBRO_DATA_VALUE_TYPE_U_INT32 && dlen != sizeof(u_int32_t))
      || (dtype == CEREBRO_DATA_VALUE_TYPE_FLOAT && dlen != sizeof(float))
      || (dtype == CEREBRO_DATA_VALUE_TYPE_DOUBLE && dlen != sizeof(double))
      || (dtype == CEREBRO_DATA_VALUE_TYPE_INT64 && dlen != sizeof(int64_t))
      || (dtype == CEREBRO_DATA_VALUE_TYPE_U_INT64 && dlen != sizeof(u_int64_t)))
    {
      CEREBRO_DBG(("%s: invalid data len", caller));
      return -1;
    }

  if (dtype == CEREBRO_DATA_VALUE_TYPE_STRING && !dlen)
    {
      CEREBRO_DBG(("%s: empty string len", caller));
      return -1;
    }

  if (dtype == CEREBRO_DATA_VALUE_TYPE_STRING 
      && dlen > CEREBRO_MAX_DATA_STRING_LEN)
    {
      CEREBRO_DBG(("%s: string len too long", caller));
      return -1;
    }

  return 0;

}

int 
_check_data_type_len_value(u_int32_t dtype, 
                           u_int32_t dlen, 
                           void *dvalue,
                           const char *caller)
{
  if (!caller)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  if (_check_data_type_len(dtype, dlen, caller) < 0)
    return -1;

  if ((!dlen && dvalue) || (dlen && !dvalue))
    {
      CEREBRO_DBG(("%s: invalid data value", caller));
      return -1;
    }

  return 0;
}

int
_marshall_data(u_int32_t dtype,
               u_int32_t dlen,
               void *dvalue,
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

  if (_check_data_type_len_value(dtype, dlen, dvalue, caller) < 0)
    {
      if (errnum)
        *errnum = CEREBRO_ERR_PARAMETERS;
      return -1;
    }

  if ((n = marshall_u_int32(dtype, buf + c, buflen - c)) <= 0)
    {
      CEREBRO_DBG(("%s: marshall_u_int32", caller));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += n;
                                                                                     
  if ((n = marshall_u_int32(dlen, buf + c, buflen - c)) <= 0)
    {
      CEREBRO_DBG(("%s: marshall_u_int32", caller));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += n;

  if (!dlen)
    return c;

  if (dtype == CEREBRO_DATA_VALUE_TYPE_INT32)
    {
      if ((n = marshall_int32(*((int32_t *)dvalue), buf + c, buflen - c)) <= 0)
        {
          CEREBRO_DBG(("%s: marshall_int32", caller));
          if (errnum)
            *errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }
  else if (dtype == CEREBRO_DATA_VALUE_TYPE_U_INT32)
    {
      if ((n = marshall_u_int32(*((u_int32_t *)dvalue), buf + c, buflen - c)) <= 0)
        {
          CEREBRO_DBG(("%s: marshall_u_int32", caller));
          if (errnum)
            *errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }
  else if (dtype == CEREBRO_DATA_VALUE_TYPE_FLOAT)
    {
      if ((n = marshall_float(*((float *)dvalue), buf + c, buflen - c)) <= 0)
        {
          CEREBRO_DBG(("%s: marshall_float", caller));
          if (errnum)
            *errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }
  else if (dtype == CEREBRO_DATA_VALUE_TYPE_DOUBLE)
    {
      if ((n = marshall_double(*((double *)dvalue), buf + c, buflen - c)) <= 0)
        {
          CEREBRO_DBG(("%s: marshall_double", caller));
          if (errnum)
            *errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }
  else if (dtype == CEREBRO_DATA_VALUE_TYPE_STRING)
    {
      if ((n = marshall_buffer(dvalue, dlen, buf + c, buflen - c)) <= 0)
        {
          CEREBRO_DBG(("%s: marshall_buffer", caller));
          if (errnum)
            *errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }
  else if (dtype == CEREBRO_DATA_VALUE_TYPE_INT64)
    {
      if ((n = marshall_int64(*((int64_t *)dvalue), buf + c, buflen - c)) <= 0)
        {
          CEREBRO_DBG(("%s: marshall_int64", caller));
          if (errnum)
            *errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }
  else if (dtype == CEREBRO_DATA_VALUE_TYPE_U_INT64)
    {
      if ((n = marshall_u_int64(*((u_int64_t *)dvalue), buf + c, buflen - c)) <= 0)
        {
          CEREBRO_DBG(("%s: marshall_u_int64", caller));
          if (errnum)
            *errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }
  else
    {
      CEREBRO_DBG(("%s: invalid type %d", caller, dtype));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  c += n;

  return c;
}

int 
_unmarshall_data_type_len(u_int32_t *dtype,
                          u_int32_t *dlen,
                          const char *buf,
                          unsigned int buflen,
                          int *errnum,
                          const char *caller)
{
  int n, c = 0;

  if (!dtype || !dlen || !buf || !caller)
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

  if ((n = unmarshall_u_int32(dtype, buf + c,  buflen - c)) < 0)
    {
      CEREBRO_DBG(("unmarshall_u_int32"));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  c += n;
                                                                                     
  if ((n = unmarshall_u_int32(dlen, buf + c, buflen - c)) < 0)
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
_unmarshall_data_value(u_int32_t dtype,
                       u_int32_t dlen,
                       void *dvalue,
                       unsigned int dvalue_len,
                       const char *buf,
                       unsigned int buflen,
                       int *errnum,
                       const char *caller)
{
  int n = -1;

  if (dlen > dvalue_len || !buf || !caller)
    {
      CEREBRO_DBG(("%s: invalid parameters", caller));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (_check_data_type_len(dtype, dlen, caller) < 0)
    {
      if (errnum)
        *errnum = CEREBRO_ERR_PROTOCOL;
      return -1;
    }
  
  if (!dlen)
    return 0;

  if (buflen < dlen)
    {
      CEREBRO_DBG(("%s: invalid packet size", caller));
      if (errnum)
        *errnum = CEREBRO_ERR_PROTOCOL;
      return -1;
    }

  if (dtype == CEREBRO_DATA_VALUE_TYPE_INT32)
    {
      if ((n = unmarshall_int32((int32_t *)dvalue, buf, buflen)) < 0)
        CEREBRO_DBG(("%s: unmarshall_int32", caller));
    }
  else if (dtype == CEREBRO_DATA_VALUE_TYPE_U_INT32)
    {
      if ((n = unmarshall_u_int32((u_int32_t *)dvalue, buf, buflen)) < 0)
        CEREBRO_DBG(("%s: unmarshall_u_int32", caller));
    }
  else if (dtype == CEREBRO_DATA_VALUE_TYPE_FLOAT)
    {
      if ((n = unmarshall_float((float *)dvalue, buf, buflen)) < 0)
        CEREBRO_DBG(("%s: unmarshall_float", caller));
    }
  else if (dtype == CEREBRO_DATA_VALUE_TYPE_DOUBLE)
    {
      if ((n = unmarshall_double((double *)dvalue, buf, buflen)) < 0)
        CEREBRO_DBG(("%s: unmarshall_double", caller));
    }
  else if (dtype == CEREBRO_DATA_VALUE_TYPE_STRING)
    {
      if ((n = unmarshall_buffer((char *)dvalue, dlen, buf, buflen)) < 0)
        CEREBRO_DBG(("%s: unmarshall_buffer", caller));
    }
  else if (dtype == CEREBRO_DATA_VALUE_TYPE_INT64)
    {
      if ((n = unmarshall_int64((int64_t *)dvalue, buf, buflen)) < 0)
        CEREBRO_DBG(("%s: unmarshall_int64", caller));
    }
  else if (dtype == CEREBRO_DATA_VALUE_TYPE_U_INT64)
    {
      if ((n = unmarshall_u_int64((u_int64_t *)dvalue, buf, buflen)) < 0)
        CEREBRO_DBG(("%s: unmarshall_u_int64", caller));
    }
  else
    /* If an invalid param, should have been caught before here */
    CEREBRO_DBG(("%s: invalid type %d", caller, dtype));
  
  if (n < 0)
    {
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }

  if (n != dlen)
    {
      /* Internal error b/c this should have been caught earlier */
      CEREBRO_DBG(("%s: unfinished unmarshalling", caller));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      return -1;
    }
  
  return n;
}
