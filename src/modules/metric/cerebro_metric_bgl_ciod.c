/*****************************************************************************\
 *  $Id: cerebro_metric_bgl_ciod.c,v 1.1 2005-08-05 00:19:32 achu Exp $
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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
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
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#if HAVE_PTHREAD_H
#include <pthread.h>
#endif /* HAVE_PTHREAD_H */

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_metric_module.h"

#include "debug.h"

#define BGL_CIOD_METRIC_MODULE_NAME "bgl_ciod"
#define BGL_CIOD_METRIC_NAME        "bgl_ciod"

/*
 * metric_bgl_ciod
 *
 * cached system bgl_ciod
 */
static u_int32_t metric_bgl_ciod = 0;

/*
 * bgl_ciod_metric_setup
 *
 * bgl_ciod metric module setup function.
 */
static int
bgl_ciod_metric_setup(void)
{
  return 0;
}

/*
 * bgl_ciod_metric_cleanup
 *
 * bgl_ciod metric module cleanup function
 */
static int
bgl_ciod_metric_cleanup(void)
{
  /* nothing to do */
  return 0;
}

/*
 * bgl_ciod_metric_get_metric_name
 *
 * bgl_ciod metric module get_metric_name function
 */
static char *
bgl_ciod_metric_get_metric_name(void)
{
  return BGL_CIOD_METRIC_NAME;
}

/*
 * bgl_ciod_metric_get_metric_period
 *
 * bgl_ciod metric module get_metric_period function
 */
static int
bgl_ciod_metric_get_metric_period(int *period)
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
 * bgl_ciod_metric_get_metric_value
 *
 * bgl_ciod metric module get_metric_value function
 */
static int
bgl_ciod_metric_get_metric_value(unsigned int *metric_value_type,
                                    unsigned int *metric_value_len,
                                    void **metric_value)
{
  if (!metric_value_type || !metric_value_len || !metric_value)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  *metric_value_type = CEREBRO_METRIC_VALUE_TYPE_U_INT32;
  *metric_value_len = sizeof(u_int32_t);
  *metric_value = (void *)&metric_bgl_ciod;

  return 0;
}

/*
 * bgl_ciod_metric_destroy_metric_value
 *
 * bgl_ciod metric module destroy_metric_value function
 */
static int
bgl_ciod_metric_destroy_metric_value(void *metric_value)
{
  return 0;
}

/*
 * bgl_ciod_metric_get_metric_thread
 *
 * bgl_ciod metric module get_metric_thread function
 */
static Cerebro_metric_thread_pointer
bgl_ciod_metric_get_metric_thread(void)
{
  return NULL;
}

struct cerebro_metric_module_info metric_module_info =
  {
    BGL_CIOD_METRIC_MODULE_NAME,
    &bgl_ciod_metric_setup,
    &bgl_ciod_metric_cleanup,
    &bgl_ciod_metric_get_metric_name,
    &bgl_ciod_metric_get_metric_period,
    &bgl_ciod_metric_get_metric_value,
    &bgl_ciod_metric_destroy_metric_value,
    &bgl_ciod_metric_get_metric_thread,
  };
