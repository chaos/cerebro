/*****************************************************************************\
 *  $Id: network_util.h,v 1.1 2005-07-01 17:24:46 achu Exp $
\*****************************************************************************/

#ifndef _NETWORK_UTIL_H
#define _NETWORK_UTIL_H

/* 
 * receive_data
 *
 * Receive a certain amount of data
 *
 * Returns bytes read on success, -1 and errnum on error.
 */
int receive_data(int fd,
                 unsigned int bytes_to_read,
                 char *buf,
                 unsigned int buflen,
                 unsigned int timeout_len,
                 unsigned int *errnum);


#endif /* _NETWORK_UTIL_H */
