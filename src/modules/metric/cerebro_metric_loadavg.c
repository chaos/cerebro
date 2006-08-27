/*****************************************************************************\
 *  $Id: cerebro_metric_loadavg.c,v 1.1 2006-08-27 04:43:40 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2005 The Regents of the University of California.
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
 *  with Genders; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
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

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_metric_module.h"

#include "debug.h"

#define LOADAVG_FILE                "/proc/loadavg"
#define LOADAVG_BUFLEN              4096

/*
 * cerebro_get_loadavgs
 *
 * Read the load averages
 */
int
cerebro_metric_get_loadavgs(float *loadavg1,
			    float *loadavg5,
			    float *loadavg15)
{
  int fd, len;
  float temp_loadavg1, temp_loadavg5, temp_loadavg15;
  char buf[LOADAVG_BUFLEN];
  int rv = -1;

  if ((fd = open(LOADAVG_FILE, O_RDONLY, 0)) < 0)
    {
      CEREBRO_DBG(("open: %s", strerror(errno)));
      goto cleanup;
    }

  memset(buf, '\0', LOADAVG_BUFLEN);
  if ((len = read(fd, buf, LOADAVG_BUFLEN)) < 0)
    {
      CEREBRO_DBG(("read: %s", strerror(errno)));
      goto cleanup;
    }

  if (sscanf(buf, 
	     "%f %f %f", 
	     &temp_loadavg1, 
	     &temp_loadavg5, 
	     &temp_loadavg15) != 3)
    {
      CEREBRO_DBG(("loadavg file parse error"));
      goto cleanup;
    }
  
  if (loadavg1)
    *loadavg1 = temp_loadavg1;

  if (loadavg5)
    *loadavg5 = temp_loadavg5;

  if (loadavg15)
    *loadavg15 = temp_loadavg15;

  rv = 0;
 cleanup:
  close(fd);
  return rv;
}

