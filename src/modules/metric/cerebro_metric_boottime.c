/*****************************************************************************\
 *  $Id: cerebro_metric_boottime.c,v 1.31 2010-02-02 01:01:21 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2018 Lawrence Livermore National Security, LLC.
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
#include "cerebro/cerebro_metric_module.h"

#include "cerebro_metric_common.h"
#include "debug.h"

#define BOOTTIME_FILE                "/proc/stat"
#define BOOTTIME_BUFLEN              4096
#define BOOTTIME_KEYWORD             "btime"
#define BOOTTIME_METRIC_MODULE_NAME  "boottime"
#define BOOTTIME_METRIC_NAME         "boottime"

/*
 * metric_boottime
 *
 * cached system boottime
 *
 * On some systems, due to kernel bugs, the boottime value may change
 * as the system executes.  We will read the boottime value from
 * /proc, cache it, and assume that it is always correct.
 */
static u_int32_t metric_boottime = 0;

/*
 * boottime_metric_setup
 *
 * boottime metric module setup function.  Read and store the boottime
 * out of /proc.
 */
static int
boottime_metric_setup(void)
{
  FILE *fp = NULL;
  char *bootvalptr;
  char buf[BOOTTIME_BUFLEN];
  unsigned long int bootval;

  if (!(fp = fopen(BOOTTIME_FILE, "r")))
    {
      CEREBRO_ERR(("open: %s", strerror(errno)));
      goto cleanup;
    }

  /* Read lines until EOF or error */
  while (fgets(buf, BOOTTIME_BUFLEN, fp))
    {
      bootvalptr = strstr(buf, BOOTTIME_KEYWORD);
      if (bootvalptr)
        {
          bootvalptr += strlen(BOOTTIME_KEYWORD);
          break;
        }
    }
  if (!bootvalptr)
    {
      CEREBRO_ERR(("strstr: boottime can't be found"));
      goto cleanup;
    }



  errno = 0;
  bootval = (u_int32_t)strtoul(bootvalptr, NULL, 10);
  if ((bootval == LONG_MIN || bootval == LONG_MAX) && errno == ERANGE)
    {
      CEREBRO_DBG(("boottime out of range"));
      goto cleanup;
    }

  /* ignore potential error, just return result */
  fclose(fp);
  metric_boottime = (u_int32_t)bootval;
  return 0;

 cleanup:
  /* ignore potential error, just return result */
  fclose(fp);
  return -1;
}

/*
 * boottime_metric_get_metric_name
 *
 * boottime metric module get_metric_name function
 */
static char *
boottime_metric_get_metric_name(void)
{
  return BOOTTIME_METRIC_NAME;
}

/*
 * boottime_metric_get_metric_value
 *
 * boottime metric module get_metric_value function
 */
static int
boottime_metric_get_metric_value(unsigned int *metric_value_type,
                                 unsigned int *metric_value_len,
                                 void **metric_value)
{
  if (!metric_value_type || !metric_value_len || !metric_value)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  *metric_value_type = CEREBRO_DATA_VALUE_TYPE_U_INT32;
  *metric_value_len = sizeof(u_int32_t);
  *metric_value = (void *)&metric_boottime;
  return 0;
}

#if WITH_STATIC_MODULES
struct cerebro_metric_module_info boottime_metric_module_info =
#else  /* !WITH_STATIC_MODULES */
struct cerebro_metric_module_info metric_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    BOOTTIME_METRIC_MODULE_NAME,
    &common_metric_interface_version,
    &boottime_metric_setup,
    &common_metric_cleanup_do_nothing,
    &boottime_metric_get_metric_name,
    &common_metric_get_metric_period_0,
    &common_metric_get_metric_flags_none,
    &boottime_metric_get_metric_value,
    &common_metric_destroy_metric_value_do_nothing,
    &common_metric_get_metric_thread_null,
    &common_metric_send_message_function_pointer_unused,
  };
