/*****************************************************************************\
 *  $Id: cerebro_util.h,v 1.5 2005-05-04 22:21:33 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_UTIL_H
#define _CEREBRO_UTIL_H

/* 
 * cerebro_handle_check
 *
 * Checks for a proper cerebro handle, setting the errnum
 * appropriately if an error is found.
 *
 * Returns 0 on succss, -1 on error
 */
int cerebro_handle_check(cerebro_t handle);

/*
 * cerebro_low_timeout_connect
 *
 * Setup a tcp connection to 'hostname' and 'port' using a connection
 * timeout of 'connect_timeout'.
 *
 * Return file descriptor on success, -1 on error.
 */
int cerebro_low_timeout_connect(cerebro_t handle,
                                const char *hostname,
                                unsigned int port,
                                unsigned int connect_timeout);

#endif /* _CEREBRO_UTIL_H */
