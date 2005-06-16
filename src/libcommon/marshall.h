/*****************************************************************************\
 *  $Id: marshall.h,v 1.1 2005-06-16 15:45:32 achu Exp $
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
 * marshall_u_int8
 *
 * marshall contents of an unsigned 8 bit integer
 *
 * Returns length of data copied into buffer, 0 if not enough buffer
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
