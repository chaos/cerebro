/*****************************************************************************\
 *  $Id: cerebro_metric_memtotal.c,v 1.1 2006-08-26 16:06:56 chu11 Exp $
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
#include <limits.h>
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

#define MEMTOTAL_FILE                "/proc/meminfo"
#define MEMTOTAL_KEYWORD             "MemTotal"
#define MEMTOTAL_BUFLEN              4096
#define MEMTOTAL_METRIC_MODULE_NAME  "memtotal"
#define MEMTOTAL_METRIC_NAME         "memtotal"

/*
 * memtotal_metric_setup
 *
 * memtotal metric module setup function.  Read and store the memtotal
 * out of /proc.
 */
static int
memtotal_metric_setup(void)
{
  /* nothing to do */
  return 0;
}

/*
 * memtotal_metric_cleanup
 *
 * memtotal metric module cleanup function
 */
static int
memtotal_metric_cleanup(void)
{
  /* nothing to do */
  return 0;
}

/*
 * memtotal_metric_get_metric_name
 *
 * memtotal metric module get_metric_name function
 */
static char *
memtotal_metric_get_metric_name(void)
{
  return MEMTOTAL_METRIC_NAME;
}

/*
 * memtotal_metric_get_metric_period
 *
 * memtotal metric module get_metric_period function
 */
static int
memtotal_metric_get_metric_period(int *period)
{
  if (!period)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }
  
  *period = 60;
  return 0;
}

/*
 * memtotal_metric_get_metric_value
 *
 * memtotal metric module get_metric_value function
 */
static int
memtotal_metric_get_metric_value(unsigned int *metric_value_type,
                                 unsigned int *metric_value_len,
                                 void **metric_value)
{
  int fd, len;
  unsigned long int memtotalval;
  char *memtotalvalptr;
  u_int32_t *memtotalptr = NULL;
  char buf[MEMTOTAL_BUFLEN];
  int rv = -1;

  if (!metric_value_type || !metric_value_len || !metric_value)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }
 
  if ((fd = open(MEMTOTAL_FILE, O_RDONLY, 0)) < 0)
    {
      CEREBRO_DBG(("open: %s", strerror(errno)));
      goto cleanup;
    }

  memset(buf, '\0', MEMTOTAL_BUFLEN);
  if ((len = read(fd, buf, MEMTOTAL_BUFLEN)) < 0)
    {
      CEREBRO_DBG(("read: %s", strerror(errno)));
      goto cleanup;
    }

  if (!(memtotalvalptr = strstr(buf, MEMTOTAL_KEYWORD)))
    {
      CEREBRO_DBG(("memtotal file parse error"));
      goto cleanup;
    }
  memtotalvalptr += strlen(MEMTOTAL_KEYWORD);
  memtotalvalptr += 1;                /* for the ':' character */

  errno = 0;
  memtotalval = (u_int32_t)strtoul(memtotalvalptr, NULL, 10);
  if ((memtotalval == LONG_MIN || memtotalval == LONG_MAX) && errno == ERANGE)
    {
      CEREBRO_DBG(("memtotal out of range"));
      goto cleanup;
    }

  if (!(memtotalptr = (u_int32_t *)malloc(sizeof(u_int32_t))))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      goto cleanup;
    }

  *memtotalptr = memtotalval;

  *metric_value_type = CEREBRO_METRIC_VALUE_TYPE_U_INT32;
  *metric_value_len = sizeof(u_int32_t);
  *metric_value = (void *)memtotalptr;

  rv = 0;
 cleanup:
  close(fd);
  if (rv < 0 && memtotalptr)
    free(memtotalptr);
  return rv;
}

/*
 * memtotal_metric_destroy_metric_value
 *
 * memtotal metric module destroy_metric_value function
 */
static int
memtotal_metric_destroy_metric_value(void *metric_value)
{
  if (!metric_value)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  free(metric_value);
  return 0;
}

/*
 * memtotal_metric_get_metric_thread
 *
 * memtotal metric module get_metric_thread function
 */
static Cerebro_metric_thread_pointer
memtotal_metric_get_metric_thread(void)
{
  return NULL;
}

/*
 * memtotal_metric_send_heartbeat_function_pointer
 *
 * memtotal metric module send_heartbeat_function_pointer function
 */
static int
memtotal_metric_send_heartbeat_function_pointer(Cerebro_metric_send_heartbeat function_pointer)
{
  return 0;
}

#if WITH_STATIC_MODULES
struct cerebro_metric_module_info memtotal_metric_module_info =
#else  /* !WITH_STATIC_MODULES */
struct cerebro_metric_module_info metric_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    MEMTOTAL_METRIC_MODULE_NAME,
    &memtotal_metric_setup,
    &memtotal_metric_cleanup,
    &memtotal_metric_get_metric_name,
    &memtotal_metric_get_metric_period,
    &memtotal_metric_get_metric_value,
    &memtotal_metric_destroy_metric_value,
    &memtotal_metric_get_metric_thread,
    &memtotal_metric_send_heartbeat_function_pointer,
  };
