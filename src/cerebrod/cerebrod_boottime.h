/*****************************************************************************\
 *  $Id: cerebrod_boottime.h,v 1.3 2004-07-06 17:06:26 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_BOOTTIME_H
#define _CEREBROD_BOOTTIME_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else  /* !TIME_WITH_SYS_TIME */
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else /* !HAVE_SYS_TIME_H */
#  include <time.h>
# endif	/* !HAVE_SYS_TIME_H */
#endif /* !TIME_WITH_SYS_TIME */

time_t cerebrod_get_boottime(void);

#endif /* _CEREBROD_BOOTTIME_H */
