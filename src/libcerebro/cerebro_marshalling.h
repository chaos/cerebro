/*****************************************************************************\
 *  $Id: cerebro_marshalling.h,v 1.3 2005-04-28 18:08:27 achu Exp $
\*****************************************************************************/
 
#ifndef _CEREBRO_MARSHALLING_H
#define _CEREBRO_MARSHALLING_H

/*
 * cerebro_marshall_int8
 *
 * marshall contents of a 8 bit integer
 *
 * Returns length of data copied into buffer, -1 on error
 */
int cerebro_marshall_int8(int8_t val, char *buffer, int bufferlen);

/*
 * cerebro_marshall_int32
 *
 * marshall contents of a 32 bit integer
 *
 * Returns length of data copied into buffer, -1 on error
 */
int cerebro_marshall_int32(int32_t val, char *buffer, int bufferlen);

/*
 * cerebro_marshall_uint8
 *
 * marshall contents of an unsigned 8 bit integer
 *
 * Returns length of data copied into buffer, -1 on error
 */
int cerebro_marshall_uint8(u_int8_t val, char *buffer, int bufferlen);

/*
 * cerebro_marshall_uint32
 *
 * marshall contents of an unsigned 32 bit integer
 *
 * Returns length of data copied into buffer, -1 on error
 */
int cerebro_marshall_uint32(u_int32_t val, char *buffer, int bufferlen);

/*
 * cerebro_marshall_buffer
 *
 * marshall contents of a buffer
 *
 * Returns length of data copied into buffer, -1 on error
 */
int cerebro_marshall_buffer(char *buf, int buflen, char *buffer, int bufferlen);

/*
 * cerebro_unmarshall_int8
 *
 * unmarshall contents of a 8 bit integer
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer does
 * not contain enough data, -1 on error
 */
int cerebro_unmarshall_int8(int8_t *val, char *buffer, int bufferlen);

/*
 * cerebro_unmarshall_int32
 *
 * unmarshall contents of a 32 bit integer
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer does
 * not contain enough data, -1 on error
 */
int cerebro_unmarshall_int32(int32_t *val, char *buffer, int bufferlen);

/*
 * cerebro_unmarshall_uint8
 *
 * unmarshall contents of an unsigned 8 bit integer
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer does
 * not contain enough data, -1 on error
 */
int cerebro_unmarshall_uint8(u_int8_t *val, char *buffer, int bufferlen);

/*
 * cerebro_unmarshall_uint32
 *
 * unmarshall contents of an unsigned 32 bit integer
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer does
 * not contain enough data, -1 on error
 */
int cerebro_unmarshall_uint32(u_int32_t *val, char *buffer, int bufferlen);

/*
 * cerebro_unmarshall_buffer
 *
 * unmarshall contents of a buffer
 *
 * Returns length of data unmarshalled from buffer, 0 if buffer does
 * not contain enough data, -1 on error
 */
int cerebro_unmarshall_buffer(char *buf, int buflen, char *buffer, int bufferlen);

#endif /* _CEREBRO_MARSHALLING_H */
