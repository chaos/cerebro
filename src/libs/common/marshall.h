/*****************************************************************************\
 *  $Id: marshall.h,v 1.6 2007-10-17 22:04:49 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007 Lawrence Livermore National Security, LLC.
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
 *  with Cerebro.  If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/
 
#ifndef _MARSHALL_H
#define _MARSHALL_H

#include <sys/types.h>

/*
 * marshall_int8
 *
 * marshall contents of a 8 bit integer
 *
 * Returns length of data copied into buffer, 0 if not enough buffer
 * space available, -1 on error
 */
int marshall_int8(int8_t val, char *buf, unsigned int buflen);

/*
 * marshall_int32
 *
 * marshall contents of a 32 bit integer
 *
 * Returns length of data copied into buffer, 0 if not enough buffer
 * space available, -1 on error
 */
int marshall_int32(int32_t val, char *buf, unsigned int buflen);

/*
 * marshall_int64
 *
 * marshall contents of a 64 bit integer
 *
 * Returns length of data copied into buffer, 0 if not enough buffer
 * space available, -1 on error
 */
int marshall_int64(int64_t val, char *buf, unsigned int buflen);

/*
 * marshall_u_int8
 *
 * marshall contents of an unsigned 8 bit integer
 *
g * Returns length of data copied into buffer, 0 if not enough buffer
 * space available, -1 on error
 */
int marshall_u_int8(u_int8_t val, char *buf, unsigned int buflen);

/*
 * marshall_u_int32
 *
 * marshall contents of an unsigned 32 bit integer
 *
 * Returns length of data copied into buffer, 0 if not enough buffer
 * space available, -1 on error
 */
int marshall_u_int32(u_int32_t val, char *buf, unsigned int buflen);

/*
 * marshall_u_int64
 *
 * marshall contents of an unsigned 64 bit integer
 *
 * Returns length of data copied into buffer, 0 if not enough buffer
 * space available, -1 on error
 */
int marshall_u_int64(u_int64_t val, char *buf, unsigned int buflen);

/*
 * marshall_float
 *
 * marshall contents of a float
 *
 * Returns length of data copied into buffer, 0 if not enough buffer
 * space available, -1 on error
 */
int marshall_float(float val, char *buf, unsigned int buflen);

/*
 * marshall_double
 *
 * marshall contents of a double
 *
 * Returns length of data copied into buffer, 0 if not enough buffer
 * space available, -1 on error
 */
int marshall_double(double val, char *buf, unsigned int buflen);

/*
 * marshall_buffer
 *
 * marshall contents of a buffer
 *
 * Returns length of data copied into buffer, 0 if not enough buffer
 * space available, -1 on error
 */
int marshall_buffer(const char *val, 
                    unsigned int vallen, 
                    char *buf, 
                    unsigned int buflen);

/*
 * unmarshall_int8
 *
 * unmarshall contents of a 8 bit integer
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer does
 * not contain enough data, -1 on error
 */
int unmarshall_int8(int8_t *val, const char *buf, unsigned int buflen);

/*
 * unmarshall_int32
 *
 * unmarshall contents of a 32 bit integer
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer does
 * not contain enough data, -1 on error
 */
int unmarshall_int32(int32_t *val, const char *buf, unsigned int buflen);

/*
 * unmarshall_int64
 *
 * unmarshall contents of a 64 bit integer
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer does
 * not contain enough data, -1 on error
 */
int unmarshall_int64(int64_t *val, const char *buf, unsigned int buflen);

/*
 * unmarshall_u_int8
 *
 * unmarshall contents of an unsigned 8 bit integer
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer does
 * not contain enough data, -1 on error
 */
int unmarshall_u_int8(u_int8_t *val, const char *buf, unsigned int buflen);

/*
 * unmarshall_u_int32
 *
 * unmarshall contents of an unsigned 32 bit integer
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer does
 * not contain enough data, -1 on error
 */
int unmarshall_u_int32(u_int32_t *val, const char *buf, unsigned int buflen);

/*
 * unmarshall_u_int64
 *
 * unmarshall contents of an unsigned 64 bit integer
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer does
 * not contain enough data, -1 on error
 */
int unmarshall_u_int64(u_int64_t *val, const char *buf, unsigned int buflen);

/*
 * unmarshall_float
 *
 * unmarshall contents of a float
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer does
 * not contain enough data, -1 on error
 */
int unmarshall_float(float *val, const char *buf, unsigned int buflen);

/*
 * unmarshall_double
 *
 * unmarshall contents of a double
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer does
 * not contain enough data, -1 on error
 */
int unmarshall_double(double *val, const char *buf, unsigned int buflen);

/*
 * unmarshall_buffer
 *
 * unmarshall contents of a buffer
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer does
 * not contain enough data, -1 on error
 */
int unmarshall_buffer(char *val, 
                      unsigned int vallen, 
                      const char *buf, 
                      unsigned int buflen);

#endif /* _MARSHALL_H */
