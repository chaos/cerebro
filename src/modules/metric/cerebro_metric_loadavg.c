/*****************************************************************************\
 *  $Id: cerebro_metric_loadavg.c,v 1.10 2010-02-02 01:01:21 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2015 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2005-2007 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <http://www.llnl.gov/linux/cerebro/>.
 *
 *  Cerebro is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  Cerebro is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with Cerebro.  If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#include <ctype.h>
#endif /* STDC_HEADERS */
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else  /* !TIME_WITH_SYS_TIME */
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else /* !HAVE_SYS_TIME_H */
#  include <time.h>
# endif /* !HAVE_SYS_TIME_H */
#endif /* !TIME_WITH_SYS_TIME */

#include "cerebro.h"
#include "cerebro/cerebro_metric_module.h"

#include "debug.h"

#define LOADAVG_FILE                "/proc/loadavg"
#define LOADAVG_BUFLEN              4096
#define LOADVAG_CACHETIMEOUT        5

static float cache_loadavg1;
static float cache_loadavg5;
static float cache_loadavg15;

static unsigned long int last_read = 0;

/*
 * cerebro_get_loadavgs
 *
 * Read load averages
 */
int
cerebro_metric_get_loadavgs(float *loadavg1,
			    float *loadavg5,
			    float *loadavg15)
{
  int len, fd = -1;
  char buf[LOADAVG_BUFLEN];
  struct timeval now;
  int rv = -1;

  if (gettimeofday(&now, NULL) < 0)
    {
      CEREBRO_ERR(("gettimeofday: %s", strerror(errno)));
      goto cleanup;
    }

  if ((now.tv_sec - last_read) > LOADVAG_CACHETIMEOUT) 
    {
      if ((fd = open(LOADAVG_FILE, O_RDONLY, 0)) < 0)
	{
	  CEREBRO_ERR(("open: %s", strerror(errno)));
	  goto cleanup;
	}
      
      memset(buf, '\0', LOADAVG_BUFLEN);
      if ((len = read(fd, buf, LOADAVG_BUFLEN)) < 0)
	{
	  CEREBRO_ERR(("read: %s", strerror(errno)));
	  goto cleanup;
	}
      
      if (sscanf(buf, 
		 "%f %f %f", 
		 &cache_loadavg1, 
		 &cache_loadavg5, 
		 &cache_loadavg15) != 3)
	{
	  CEREBRO_DBG(("loadavg file parse error"));
	  goto cleanup;
	}

      last_read = now.tv_sec;
    }

  if (loadavg1)
    *loadavg1 = cache_loadavg1;

  if (loadavg5)
    *loadavg5 = cache_loadavg5;

  if (loadavg15)
    *loadavg15 = cache_loadavg15;

  rv = 0;
 cleanup:
  /* ignore potential error, just return result */
  if (fd >= 0)
    close(fd);
  return rv;
}

