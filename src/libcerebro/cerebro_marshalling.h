/*****************************************************************************\
 *  $Id: cerebro_marshalling.h,v 1.1 2005-03-27 08:10:24 achu Exp $
\*****************************************************************************/
 
#ifndef _CEREBRO_MARSHALLING_H
#define _CEREBRO_MARSHALLING_H

/*
 * cerebro_marshall_int32
 *
 * marshall contents of a 32 bit integer
 *
 * Returns length of data copied into buffer
 */
int cerebro_marshall_int32(int32_t val, char *buffer, int bufferlen);

/*
 * cerebro_marshall_uint32
 *
 * marshall contents of an unsigned 32 bit integer
 *
 * Returns length of data copied into buffer
 */
int cerebro_marshall_uint32(u_int32_t val, char *buffer, int bufferlen);

/*
 * cerebro_marshall_buffer
 *
 * marshall contents of a buffer
 *
 * Returns length of data copied into buffer
 */
int cerebro_marshall_buffer(char *buf, int buflen, char *buffer, int bufferlen);

/*
 * cerebro_unmarshall_int32
 *
 * unmarshall contents of a 32 bit integer
 *
 * Returns length of data read from buffer
 */
int cerebro_unmarshall_int32(int32_t *val, char *buffer, int bufferlen);

/*
 * cerebro_unmarshall_uint32
 *
 * unmarshall contents of an unsigned 32 bit integer
 *
 * Returns length of data read from buffer
 */
int cerebro_unmarshall_uint32(u_int32_t *val, char *buffer, int bufferlen);

/*
 * cerebro_unmarshall_buffer
 *
 * unmarshall contents of a buffer
 *
 * Returns length of data read from buffer
 */
int cerebro_unmarshall_buffer(char *buf, int buflen, char *buffer, int bufferlen);

#endif /* _CEREBRO_MARSHALLING_H */
