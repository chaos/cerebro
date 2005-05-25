/*****************************************************************************\
 *  $Id: cerebro_marshalling.h,v 1.9 2005-05-25 20:39:35 achu Exp $
\*****************************************************************************/
 
#ifndef _CEREBRO_MARSHALLING_H
#define _CEREBRO_MARSHALLING_H

/*
 * _cerebro_marshall_int8
 *
 * marshall contents of a 8 bit integer
 *
 * Returns length of data copied into buffer, -1 on error
 */
int _cerebro_marshall_int8(int8_t val, char *buf, unsigned int buflen);

/*
 * _cerebro_marshall_int32
 *
 * marshall contents of a 32 bit integer
 *
 * Returns length of data copied into buffer, -1 on error
 */
int _cerebro_marshall_int32(int32_t val, char *buf, unsigned int buflen);

/*
 * _cerebro_marshall_unsigned_int8
 *
 * marshall contents of an unsigned 8 bit integer
 *
 * Returns length of data copied into buffer, -1 on error
 */
int _cerebro_marshall_unsigned_int8(u_int8_t val, 
                                    char *buf, 
                                    unsigned int buflen);

/*
 * _cerebro_marshall_unsigned_int32
 *
 * marshall contents of an unsigned 32 bit integer
 *
 * Returns length of data copied into buffer, -1 on error
 */
int _cerebro_marshall_unsigned_int32(u_int32_t val, 
                                     char *buf, 
                                     unsigned int buflen);

/*
 * _cerebro_marshall_float
 *
 * marshall contents of a float
 *
 * Returns length of data copied into buffer, -1 on error
 */
int _cerebro_marshall_float(float val, char *buf, unsigned int buflen);

/*
 * _cerebro_marshall_double
 *
 * marshall contents of a double
 *
 * Returns length of data copied into buffer, -1 on error
 */
int _cerebro_marshall_double(double val, char *buf, unsigned int buflen);

/*
 * _cerebro_marshall_buffer
 *
 * marshall contents of a buffer
 *
 * Returns length of data copied into buffer, -1 on error
 */
int _cerebro_marshall_buffer(const char *val, 
                             unsigned int vallen, 
                             char *buf, 
                             unsigned int buflen);

/*
 * _cerebro_unmarshall_int8
 *
 * unmarshall contents of a 8 bit integer
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer does
 * not contain enough data, -1 on error
 */
int _cerebro_unmarshall_int8(int8_t *val, 
                             const char *buf, 
                             unsigned int buflen);

/*
 * _cerebro_unmarshall_int32
 *
 * unmarshall contents of a 32 bit integer
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer does
 * not contain enough data, -1 on error
 */
int _cerebro_unmarshall_int32(int32_t *val, 
                              const char *buf, 
                              unsigned int buflen);

/*
 * _cerebro_unmarshall_unsigned_int8
 *
 * unmarshall contents of an unsigned 8 bit integer
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer does
 * not contain enough data, -1 on error
 */
int _cerebro_unmarshall_unsigned_int8(u_int8_t *val, 
                                      const char *buf, 
                                      unsigned int buflen);

/*
 * _cerebro_unmarshall_unsigned_int32
 *
 * unmarshall contents of an unsigned 32 bit integer
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer does
 * not contain enough data, -1 on error
 */
int _cerebro_unmarshall_unsigned_int32(u_int32_t *val, 
                                       const char *buf, 
                                       unsigned int buflen);

/*
 * _cerebro_unmarshall_float
 *
 * unmarshall contents of a float
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer does
 * not contain enough data, -1 on error
 */
int _cerebro_unmarshall_float(float *val, 
                              const char *buf, 
                              unsigned int buflen);

/*
 * _cerebro_unmarshall_double
 *
 * unmarshall contents of a double
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer does
 * not contain enough data, -1 on error
 */
int _cerebro_unmarshall_double(double *val, 
                               const char *buf, 
                               unsigned int buflen);

/*
 * _cerebro_unmarshall_buffer
 *
 * unmarshall contents of a buffer
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer does
 * not contain enough data, -1 on error
 */
int _cerebro_unmarshall_buffer(char *val, 
                               unsigned int vallen, 
                               const char *buf, 
                               unsigned int buflen);

#endif /* __CEREBRO_MARSHALLING_H */
