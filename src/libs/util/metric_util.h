/*****************************************************************************\
 *  $Id: metric_util.h,v 1.5 2005-07-20 18:08:18 achu Exp $
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
 * _unmarshall_metric_value
 *
 * Unmarshall a metric type and len
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
