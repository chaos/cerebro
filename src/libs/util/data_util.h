/*****************************************************************************\
 *  $Id: data_util.h,v 1.6 2008-03-28 17:06:49 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2008 Lawrence Livermore National Security, LLC.
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

#ifndef _DATA_UTIL_H
#define _DATA_UTIL_H

#include <sys/types.h>

#define check_data_type_len(t,l) \
        _check_data_type_len(t,l,__FUNCTION__)

#define check_data_type_len_value(t,l,v) \
        _check_data_type_len_value(t,l,v,__FUNCTION__)

#define marshall_data(t,l,v,b,bl,e) \
        _marshall_data(t,l,v,b,bl,e,__FUNCTION__)

#define unmarshall_data_type_len(t,l,b,bl,e) \
        _unmarshall_data_type_len(t,l,b,bl,e,__FUNCTION__)

#define unmarshall_data_value(t,l,v,vl,b,bl,e) \
        _unmarshall_data_value(t,l,v,vl,b,bl,e,__FUNCTION__)

/* 
 * _check_data_type_len
 *
 * Check if data type and len are reasonable
 *
 * Returns 0 if data is sane, -1 if not
 */
int _check_data_type_len(u_int32_t dtype, 
                           u_int32_t dlen, 
                           const char *caller);

/* 
 * _check_data_type_len_value
 *
 * Check if data type, len, and value are reasonable
 *
 * Returns 0 if data is sane, -1 if not
 */
int _check_data_type_len_value(u_int32_t dtype, 
                               u_int32_t dlen, 
                               void *dvalue,
                               const char *caller);

/* 
 * _marshall_data
 *
 * Marshall data type, len, and if appropriate the value
 *
 * Returns bytes written to buffer on success, -1 on error
 */
int _marshall_data(u_int32_t dtype,
                   u_int32_t dlen,
                   void *dvalue,
                   char *buf,
                   unsigned int buflen,
                   int *errnum,
                   const char *caller);

/* 
 * _unmarshall_data_type_len
 *
 * Unmarshall a data type and len
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer 
 * does not contain enough data, -1 on error
 */
int _unmarshall_data_type_len(u_int32_t *dtype,
                              u_int32_t *dlen,
                              const char *buf,
                              unsigned int buflen,
                              int *errnum,
                              const char *caller);

/* 
 * _unmarshall_data_value
 *
 * Unmarshall a data value
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer 
 * does not contain enough data, -1 on error
 */
int _unmarshall_data_value(u_int32_t dtype,
                           u_int32_t dlen,
                           void *dvalue,
                           unsigned int dvalue_len,
                           const char *buf,
                           unsigned int buflen,
                           int *errnum,
                           const char *caller);

#endif /* _DATA_UTIL_H */
