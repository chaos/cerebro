/*****************************************************************************\
 *  $Id: cerebrod_cache.h,v 1.1 2005-01-03 17:48:38 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_CACHE_H
#define _CEREBROD_CACHE_H

void cerebrod_cache(void);
u_int32_t cerebrod_get_starttime(void);
u_int32_t cerebrod_get_boottime(void);
void cerebrod_get_hostname(char *buf, unsigned int len);

#endif /* _CEREBROD_CACHE_H */
