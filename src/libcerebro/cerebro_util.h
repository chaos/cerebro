/*****************************************************************************\
 *  $Id: cerebro_util.h,v 1.1 2005-04-26 00:09:13 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_UTIL_H
#define _CEREBRO_UTIL_H

/* 
 * cerebro_handle_err_check
 *
 * Checks for a proper cerebro handle, setting the errnum
 * appropriately if an error is found.
 *
 * Returns 0 on succss, -1 one error
 */
int cerebro_handle_err_check(cerebro_t handle);

#endif /* _CEREBRO_UTIL_H */
