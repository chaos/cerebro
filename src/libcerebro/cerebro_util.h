/*****************************************************************************\
 *  $Id: cerebro_util.h,v 1.2 2005-04-26 17:04:29 achu Exp $
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

#endif /* _CEREBRO_UTIL_H */
