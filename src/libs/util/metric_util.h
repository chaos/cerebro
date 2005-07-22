/*****************************************************************************\
 *  $Id: metric_util.h,v 1.7 2005-07-22 17:21:07 achu Exp $
 *****************************************************************************
 *  Copyright (C) 2005 The Regents of the University of California.
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
 *  with Genders; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
\*****************************************************************************/

#ifndef _METRIC_UTIL_H
#define _METRIC_UTIL_H

#include <sys/types.h>

#define check_metric_type_len(t,l) \
        _check_metric_type_len(t,l,__FUNCTION__)

#define check_metric_type_len_value(t,l,v) \
        _check_metric_type_len_value(t,l,v,__FUNCTION__)

#define marshall_metric(t,l,v,b,bl,e) \
        _marshall_metric(t,l,v,b,bl,e,__FUNCTION__)

#define unmarshall_metric_type_len(t,l,b,bl,e) \
        _unmarshall_metric_type_len(t,l,b,bl,e,__FUNCTION__)

#define unmarshall_metric_value(t,l,v,vl,b,bl,e) \
        _unmarshall_metric_value(t,l,v,vl,b,bl,e,__FUNCTION__)

/* 
 * _check_metric_type_len
 *
 * Check if metric type and len are reasonable
 *
 * Returns 0 if data is sane, -1 if not
 */
int _check_metric_type_len(u_int32_t mtype, 
                           u_int32_t mlen, 
                           const char *caller);

/* 
 * _check_metric_type_len_value
 *
 * Check if metric type, len, and value are reasonable
 *
 * Returns 0 if data is sane, -1 if not
 */
int _check_metric_type_len_value(u_int32_t mtype, 
                                 u_int32_t mlen, 
                                 void *mvalue,
                                 const char *caller);

/* 
 * _marshall_metric
 *
 * Marshall metric type, len, and if appropriate the value
 *
 * Returns bytes written to buffer on success, -1 on error
 */
int _marshall_metric(u_int32_t mtype,
                     u_int32_t mlen,
                     void *mvalue,
                     char *buf,
                     unsigned int buflen,
                     int *errnum,
                     const char *caller);

/* 
 * _unmarshall_metric_type_len
 *
 * Unmarshall a metric type and len
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer 
 * does not contain enough data, -1 on error
 */
int _unmarshall_metric_type_len(u_int32_t *mtype,
                                u_int32_t *mlen,
                                const char *buf,
                                unsigned int buflen,
                                int *errnum,
                                const char *caller);

/* 
 * _unmarshall_metric_value
 *
 * Unmarshall a metric value
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer 
 * does not contain enough data, -1 on error
 */
int _unmarshall_metric_value(u_int32_t mtype,
                             u_int32_t mlen,
                             void *mvalue,
                             unsigned int mvalue_len,
                             const char *buf,
                             unsigned int buflen,
                             int *errnum,
                             const char *caller);

#endif /* _METRIC_UTIL_H */
