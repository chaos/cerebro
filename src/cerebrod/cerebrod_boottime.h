/*****************************************************************************\
 *  $Id: cerebrod_boottime.h,v 1.2 2004-07-03 00:34:15 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_BOOTTIME_H
#define _CEREBROD_BOOTTIME_H

#if HAVE_CONFIG_H
#include "config.h"
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

time_t cerebrod_get_boottime(void);

#endif /* _CEREBROD_CONFIG_H */
