/*****************************************************************************\
 *  $Id: cerebro_metric_memory.c,v 1.1 2006-08-27 05:23:53 chu11 Exp $
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
#include <assert.h>

#include "cerebro.h"
#include "cerebro/cerebro_metric_module.h"

#include "debug.h"

#define MEMORY_FILE                "/proc/meminfo"
#define MEMTOTAL_KEYWORD           "MemTotal"
#define MEMFREE_KEYWORD            "MemFree"
#define SWAPTOTAL_KEYWORD          "SwapTotal"
#define SWAPFREE_KEYWORD           "SwapFree"
#define MEMORY_BUFLEN              4096

/* 
 * _read_memory
 *
 * Returns 0 on success, -1 on error
 */
static int
_read_memory(int fd,
	     char *buf,
	     char *keyword,
	     unsigned long int *memvalptr)
{
  char *parseptr;
  unsigned long int memval;
  int rv = -1;

  assert(buf);
  assert(keyword);
  assert(memvalptr);

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
  int fd, len;
  unsigned long int val;
  char buf[MEMORY_BUFLEN];
  int rv = -1;

  if ((fd = open(MEMORY_FILE, O_RDONLY, 0)) < 0)
    {
      CEREBRO_DBG(("open: %s", strerror(errno)));
      goto cleanup;
    }

  memset(buf, '\0', MEMORY_BUFLEN);
  if ((len = read(fd, buf, MEMORY_BUFLEN)) < 0)
    {
      CEREBRO_DBG(("read: %s", strerror(errno)));
      goto cleanup;
    }

  if (memtotal)
    {
      if (_read_memory(fd, buf, MEMTOTAL_KEYWORD, &val) < 0)
	goto cleanup;
      *memtotal = val;
    }

  if (memfree)
    {
      if (_read_memory(fd, buf, MEMFREE_KEYWORD, &val) < 0)
	goto cleanup;
      *memfree = val;
    }

  if (swaptotal)
    {
      if (_read_memory(fd, buf, SWAPTOTAL_KEYWORD, &val) < 0)
	goto cleanup;
      *swaptotal = val;
    }

  if (swapfree)
    {
      if (_read_memory(fd, buf, SWAPFREE_KEYWORD, &val) < 0)
	goto cleanup;
      *swapfree = val;
    }

  rv = 0;
 cleanup:
  close(fd);
  return rv;
}
