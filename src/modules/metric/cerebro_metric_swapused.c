/*****************************************************************************\
 *  $Id: cerebro_metric_swapused.c,v 1.1 2006-08-26 16:06:56 chu11 Exp $
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

#define SWAPUSED_FILE                "/proc/meminfo"
#define SWAPTOTAL_KEYWORD            "SwapTotal"
#define SWAPFREE_KEYWORD             "SwapFree"
#define SWAPUSED_BUFLEN              4096
#define SWAPUSED_METRIC_MODULE_NAME  "swapused"
#define SWAPUSED_METRIC_NAME         "swapused"

/*
 * swapused_metric_setup
 *
 * swapused metric module setup function.  Read and store the swapused
 * out of /proc.
 */
static int
swapused_metric_setup(void)
{
  /* nothing to do */
  return 0;
}

/*
 * swapused_metric_cleanup
 *
 * swapused metric module cleanup function
 */
static int
swapused_metric_cleanup(void)
{
  /* nothing to do */
  return 0;
}

/*
 * swapused_metric_get_metric_name
 *
 * swapused metric module get_metric_name function
 */
static char *
swapused_metric_get_metric_name(void)
{
  return SWAPUSED_METRIC_NAME;
}

/*
 * swapused_metric_get_metric_period
 *
 * swapused metric module get_metric_period function
 */
static int
swapused_metric_get_metric_period(int *period)
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
 * swapused_metric_get_metric_value
 *
 * swapused metric module get_metric_value function
 */
static int
swapused_metric_get_metric_value(unsigned int *metric_value_type,
                                 unsigned int *metric_value_len,
                                 void **metric_value)
{
  int fd, len;
  unsigned long int swaptotalval, swapfreeval;
  char *swapvalptr;
  u_int32_t *swapusedptr = NULL;
  char buf[SWAPUSED_BUFLEN];
  int rv = -1;

  if (!metric_value_type || !metric_value_len || !metric_value)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }
 
  if ((fd = open(SWAPUSED_FILE, O_RDONLY, 0)) < 0)
    {
      CEREBRO_DBG(("open: %s", strerror(errno)));
      goto cleanup;
    }

  memset(buf, '\0', SWAPUSED_BUFLEN);
  if ((len = read(fd, buf, SWAPUSED_BUFLEN)) < 0)
    {
      CEREBRO_DBG(("read: %s", strerror(errno)));
      goto cleanup;
    }

  if (!(swapvalptr = strstr(buf, SWAPTOTAL_KEYWORD)))
    {
      CEREBRO_DBG(("swapused file parse error"));
      goto cleanup;
    }
  swapvalptr += strlen(SWAPTOTAL_KEYWORD);
  swapvalptr += 1;                /* for the ':' character */

  errno = 0;
  swaptotalval = (u_int32_t)strtoul(swapvalptr, NULL, 10);
  if ((swaptotalval == LONG_MIN || swaptotalval == LONG_MAX) && errno == ERANGE)
    {
      CEREBRO_DBG(("swaptotal out of range"));
      goto cleanup;
    }

  if (!(swapvalptr = strstr(buf, SWAPFREE_KEYWORD)))
    {
      CEREBRO_DBG(("swapused file parse error"));
      goto cleanup;
    }
  swapvalptr += strlen(SWAPFREE_KEYWORD);
  swapvalptr += 1;                /* for the ':' character */

  errno = 0;
  swapfreeval = (u_int32_t)strtoul(swapvalptr, NULL, 10);
  if ((swapfreeval == LONG_MIN || swapfreeval == LONG_MAX) && errno == ERANGE)
    {
      CEREBRO_DBG(("swapfree out of range"));
      goto cleanup;
    }

  if (!(swapusedptr = (u_int32_t *)malloc(sizeof(u_int32_t))))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      goto cleanup;
    }

  *swapusedptr = swaptotalval - swapfreeval;

  *metric_value_type = CEREBRO_METRIC_VALUE_TYPE_U_INT32;
  *metric_value_len = sizeof(u_int32_t);
  *metric_value = (void *)swapusedptr;

  rv = 0;
 cleanup:
  close(fd);
  if (rv < 0 && swapusedptr)
    free(swapusedptr);
  return rv;
}

/*
 * swapused_metric_destroy_metric_value
 *
 * swapused metric module destroy_metric_value function
 */
static int
swapused_metric_destroy_metric_value(void *metric_value)
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
 * swapused_metric_get_metric_thread
 *
 * swapused metric module get_metric_thread function
 */
static Cerebro_metric_thread_pointer
swapused_metric_get_metric_thread(void)
{
  return NULL;
}

/*
 * swapused_metric_send_heartbeat_function_pointer
 *
 * swapused metric module send_heartbeat_function_pointer function
 */
static int
swapused_metric_send_heartbeat_function_pointer(Cerebro_metric_send_heartbeat function_pointer)
{
  return 0;
}

#if WITH_STATIC_MODULES
struct cerebro_metric_module_info swapused_metric_module_info =
#else  /* !WITH_STATIC_MODULES */
struct cerebro_metric_module_info metric_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    SWAPUSED_METRIC_MODULE_NAME,
    &swapused_metric_setup,
    &swapused_metric_cleanup,
    &swapused_metric_get_metric_name,
    &swapused_metric_get_metric_period,
    &swapused_metric_get_metric_value,
    &swapused_metric_destroy_metric_value,
    &swapused_metric_get_metric_thread,
    &swapused_metric_send_heartbeat_function_pointer,
  };
