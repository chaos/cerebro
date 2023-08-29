/*****************************************************************************\
 *  $Id: wrappers_marshall.c,v 1.6 2010-02-02 01:01:21 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2018 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2005-2007 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <https://github.com/chaos/cerebro>.
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

#include <assert.h>
#include <errno.h>

#include "wrappers.h"

int
wrap_marshall_int8(WRAPPERS_ARGS, int8_t val, char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("marshall_int8");

  if ((rv = marshall_int8(val, buf, buflen)) <= 0)
    WRAPPERS_ERR_ERRNO("marshall_int8");

  return rv;
}

int
wrap_marshall_int32(WRAPPERS_ARGS, int32_t val, char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("marshall_int32");

  if ((rv = marshall_int32(val, buf, buflen)) <= 0)
    WRAPPERS_ERR_ERRNO("marshall_int32");

  return rv;
}

int
wrap_marshall_u_int8(WRAPPERS_ARGS, u_int8_t val, char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("marshall_u_int8");

  if ((rv = marshall_u_int8(val, buf, buflen)) <= 0)
    WRAPPERS_ERR_ERRNO("mashall_u_int8");

  return rv;
}

int
wrap_marshall_u_int32(WRAPPERS_ARGS, u_int32_t val, char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("marshall_u_int32");

  if ((rv = marshall_u_int32(val, buf, buflen)) <= 0)
    WRAPPERS_ERR_ERRNO("marshall_u_int32");

  return rv;
}

int
wrap_marshall_float(WRAPPERS_ARGS, float val, char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("marshall_float");

  if ((rv = marshall_float(val, buf, buflen)) <= 0)
    WRAPPERS_ERR_ERRNO("marshall_float");

  return rv;
}

int
wrap_marshall_double(WRAPPERS_ARGS, double val, char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("marshall_double");

  if ((rv = marshall_double(val, buf, buflen)) <= 0)
    WRAPPERS_ERR_ERRNO("marshall_double");

  return rv;
}

int
wrap_marshall_buffer(WRAPPERS_ARGS, const char *val, unsigned int vallen, char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!val || !buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("marshall_buffer");

  if ((rv = marshall_buffer(val, vallen, buf, buflen)) <= 0)
    WRAPPERS_ERR_ERRNO("marshall_buffer");

  return rv;
}

int
wrap_unmarshall_int8(WRAPPERS_ARGS, int8_t *val, const char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!val || !buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("unmarshall_int8");

  if ((rv = unmarshall_int8(val, buf, buflen)) < 0)
    WRAPPERS_ERR_ERRNO("unmarshall_int8");

  return rv;
}

int
wrap_unmarshall_int32(WRAPPERS_ARGS, int32_t *val, const char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!val || !buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("unmarshall_int32");

  if ((rv = unmarshall_int32(val, buf, buflen)) < 0)
    WRAPPERS_ERR_ERRNO("unmarshall_int32");

  return rv;
}

int
wrap_unmarshall_u_int8(WRAPPERS_ARGS, u_int8_t *val, const char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!val || !buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("unmarshall_u_int8");

  if ((rv = unmarshall_u_int8(val, buf, buflen)) < 0)
    WRAPPERS_ERR_ERRNO("unmarshall_u_int8");

  return rv;
}

int
wrap_unmarshall_u_int32(WRAPPERS_ARGS, u_int32_t *val, const char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!val || !buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("unmarshall_u_int32");

  if ((rv = unmarshall_u_int32(val, buf, buflen)) < 0)
    WRAPPERS_ERR_ERRNO("unmarshall_u_int32");

  return rv;
}

int
wrap_unmarshall_float(WRAPPERS_ARGS, float *val, const char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!val || !buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("unmarshall_float");

  if ((rv = unmarshall_float(val, buf, buflen)) < 0)
    WRAPPERS_ERR_ERRNO("unmarshall_float");

  return rv;
}

int
wrap_unmarshall_double(WRAPPERS_ARGS, double *val, const char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!val || !buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("unmarshall_double");

  if ((rv = unmarshall_double(val, buf, buflen)) < 0)
    WRAPPERS_ERR_ERRNO("unmarshall_double");

  return rv;
}

int
wrap_unmarshall_buffer(WRAPPERS_ARGS, char *val, unsigned int vallen, const char *buf, unsigned int buflen)
{
  int rv;

  assert(file && function);

  if (!val || !buf)
    WRAPPERS_ERR_INVALID_PARAMETERS("unmarshall_buffer");

  if ((rv = unmarshall_buffer(val, vallen, buf, buflen)) < 0)
    WRAPPERS_ERR_ERRNO("unmarshall_float");

  return rv;
}
