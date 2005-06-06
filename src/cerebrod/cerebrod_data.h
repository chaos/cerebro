/*****************************************************************************\
 *  $Id: cerebrod_data.h,v 1.4 2005-06-06 20:39:55 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_CACHE_H
#define _CEREBROD_CACHE_H

/* 
 * cerebrod_load_data
 *
 * Load and cache cerebrod machine boottime, and machine nodename
 */
void cerebrod_load_data(void);

/* 
 * cerebrod_get_boottime
 *
 * Return the cached system boottime
 */
u_int32_t cerebrod_get_boottime(void);

/* 
 * cerebrod_get_nodename
 *
 * Return the cached system nodename
 */
void cerebrod_get_nodename(char *buf, unsigned int len);

#endif /* _CEREBROD_CACHE_H */
