/*****************************************************************************\
 *  $Id: cerebro_metric_memory.c,v 1.8 2007-10-08 22:33:16 chu11 Exp $
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
 *  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <errno.h>
#include <limits.h>
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

#define MEMORY_FILE                "/proc/meminfo"
#define MEMTOTAL_KEYWORD           "MemTotal"
#define MEMFREE_KEYWORD            "MemFree"
#define SWAPTOTAL_KEYWORD          "SwapTotal"
#define SWAPFREE_KEYWORD           "SwapFree"
#define MEMORY_BUFLEN              4096
#define MEMORY_CACHETIMEOUT        5

static u_int32_t cache_memtotal;
static u_int32_t cache_memfree;
static u_int32_t cache_swaptotal;
static u_int32_t cache_swapfree;

static unsigned long int last_read = 0;

/* 
 * _read_memory
 *
 * Returns 0 on success, -1 on error
 */
static int
_read_memory(int fd,
	     char *buf,
	     char *keyword,
	     u_int32_t *memvalptr)
{
  char *parseptr;
  unsigned long int memval;
  int rv = -1;

  if (!buf)
    {
      CEREBRO_DBG(("buf null"));
      goto cleanup;
    }

  if (!keyword)
    {
      CEREBRO_DBG(("keyword null"));
      goto cleanup;
    }

  if (!memvalptr)
    {
      CEREBRO_DBG(("memvalptr null"));
      goto cleanup;
    }

  if (!(parseptr = strstr(buf, keyword)))
    {
      CEREBRO_DBG(("memused file parse error"));
      goto cleanup;
    }
  parseptr += strlen(keyword);
  parseptr += 1;                /* for the ':' character */
  
  errno = 0;
  memval = (u_int32_t)strtoul(parseptr, NULL, 10);
  if ((memval == LONG_MIN || memval == LONG_MAX) && errno == ERANGE)
    {
      CEREBRO_DBG(("memtotal out of range"));
      goto cleanup;
    }
  
  *memvalptr = memval;
  rv = 0;
 cleanup:
  return rv;
}

/*
 * cerebro_metric_get_memory
 *
 * Read memory statistics
 */
int
cerebro_metric_get_memory(u_int32_t *memtotal,
			  u_int32_t *memfree,
			  u_int32_t *swaptotal,
			  u_int32_t *swapfree)
{
  int len, fd = -1;
  char buf[MEMORY_BUFLEN];
  struct timeval now;
  int rv = -1;

  if (gettimeofday(&now, NULL) < 0)
    {
      CEREBRO_ERR(("gettimeofday: %s", strerror(errno)));
      goto cleanup;
    }

  if ((now.tv_sec - last_read) > MEMORY_CACHETIMEOUT)
    {
      if ((fd = open(MEMORY_FILE, O_RDONLY, 0)) < 0)
	{
	  CEREBRO_ERR(("open: %s", strerror(errno)));
	  goto cleanup;
	}
      
      memset(buf, '\0', MEMORY_BUFLEN);
      if ((len = read(fd, buf, MEMORY_BUFLEN)) < 0)
	{
	  CEREBRO_ERR(("read: %s", strerror(errno)));
	  goto cleanup;
	}

      if (_read_memory(fd, buf, MEMTOTAL_KEYWORD, &cache_memtotal) < 0)
	goto cleanup;

      if (_read_memory(fd, buf, MEMFREE_KEYWORD, &cache_memfree) < 0)
	goto cleanup;

      if (_read_memory(fd, buf, SWAPTOTAL_KEYWORD, &cache_swaptotal) < 0)
	goto cleanup;

      if (_read_memory(fd, buf, SWAPFREE_KEYWORD, &cache_swapfree) < 0)
	goto cleanup;

      last_read = now.tv_sec;
    }

  if (memtotal)
    *memtotal = cache_memtotal;

  if (memfree)
    *memfree = cache_memfree;

  if (swaptotal)
    *swaptotal = cache_swaptotal;

  if (swapfree)
    *swapfree = cache_swapfree;

  rv = 0;
 cleanup:
  if (fd >= 0)
    close(fd);
  return rv;
}
