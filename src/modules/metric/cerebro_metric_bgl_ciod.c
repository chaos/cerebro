/*****************************************************************************\
 *  $Id: cerebro_metric_bgl_ciod.c,v 1.8 2005-09-01 20:14:27 achu Exp $
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
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <errno.h>

#include "cerebro.h"
#include "cerebro/cerebro_metric_module.h"

#include "debug.h"

#include "conffile.h"

#include "network_util.h"

#define BGL_CIOD_METRIC_MODULE_NAME      "bgl_ciod"
#define BGL_CIOD_METRIC_NAME             "bgl_ciod"
#define BGL_CIOD_HOSTNAME                "localhost"
#define BGL_CIOD_PORT                    7000
#define BGL_CIOD_PERIOD_DEFAULT          60
#define BGL_CIOD_FAILURE_MAX_DEFAULT     3
#define BGL_CIOD_CONNECT_TIMEOUT_DEFAULT 5

/*
 * bgl_ciod_state
 *
 * cached system bgl_ciod state
 */
static u_int32_t bgl_ciod_state = 0;

/* 
 * bgl_ciod_failures
 *
 * counts consecutive connection failures
 */
static unsigned int bgl_ciod_failures = 0;

/* 
 * bgl_ciod_period
 *
 * the monitoring period.
 */
static unsigned int bgl_ciod_period = BGL_CIOD_PERIOD_DEFAULT;

/* 
 * bgl_ciod_failure_max
 *
 * count of consecutive failures at which we determine the ciod daemon
 * is in fact down.
 */
static unsigned int bgl_ciod_failure_max = BGL_CIOD_FAILURE_MAX_DEFAULT;

/* 
 * bgl_ciod_connect_timeout
 *
 * count of consecutive failures at which we determine the ciod daemon
 * is in fact down.
 */
static unsigned int bgl_ciod_connect_timeout = BGL_CIOD_CONNECT_TIMEOUT_DEFAULT;

/*
 * bgl_ciod_metric_setup
 *
 * bgl_ciod metric module setup function.
 */
static int
bgl_ciod_metric_setup(void)
{
  int period = 0, period_flag = 0, failure_max = 0, failure_max_flag = 0, 
    connect_timeout = 0, connect_timeout_flag = 0;

  struct conffile_option options[] =
    {
      {
        "cerebro_bgl_ciod_period",
        CONFFILE_OPTION_INT,
        -1,
        conffile_int,
        1,
        0,
        &period_flag,
        &period,
        0
      },
      {
        "cerebro_bgl_ciod_failure_max",
        CONFFILE_OPTION_INT,
        -1,
        conffile_int,
        1,
        0,
        &failure_max_flag,
        &failure_max,
        0
      },
      {
        "cerebro_bgl_ciod_connect_timeout",
        CONFFILE_OPTION_INT,
        -1,
        conffile_int,
        1,
        0,
        &connect_timeout_flag,
        &connect_timeout,
        0
      },
    };
  conffile_t cf = NULL;
  int num;

  /* 
   * If any of this fails, who cares, just move on.
   */

  if (!(cf = conffile_handle_create()))
    {
      CEREBRO_DBG(("conffile_handle_create"));
      goto cleanup;
    }

  num = sizeof(options)/sizeof(struct conffile_option);
  if (conffile_parse(cf, BGL_CIOD_CONFIG_FILE, options, num, NULL, 0, 0) < 0)
    {
      char buf[CONFFILE_MAX_ERRMSGLEN];

      /* Its not an error if the configuration file doesn't exist */
      if (conffile_errnum(cf) == CONFFILE_ERR_EXIST)
        goto cleanup;

      if (conffile_errmsg(cf, buf, CONFFILE_MAX_ERRMSGLEN) < 0)
        CEREBRO_DBG(("conffile_parse: %d", conffile_errnum(cf)));
      else
        CEREBRO_DBG(("conffile_parse: %s", buf));

      goto cleanup;
    }

  if (period_flag)
    {
      if (period > 0)
        bgl_ciod_period = period;
      else
        CEREBRO_DBG(("invalid period input: %d", period));
    }

  if (failure_max_flag)
    {
      if (failure_max > 0)
        bgl_ciod_failure_max = failure_max;
      else
        CEREBRO_DBG(("invalid failure_max input: %d", failure_max));
    }

  if (connect_timeout_flag)
    {
      if (connect_timeout > 0)
        bgl_ciod_connect_timeout = connect_timeout;
      else
        CEREBRO_DBG(("invalid connect_timeout input: %d", connect_timeout));
    }

 cleanup:
  conffile_handle_destroy(cf);
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

  *period = bgl_ciod_period;
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
  int fd, errnum;

  if (!metric_value_type || !metric_value_len || !metric_value)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  if ((fd = low_timeout_connect(BGL_CIOD_HOSTNAME,
                                BGL_CIOD_PORT,
                                bgl_ciod_connect_timeout,
                                &errnum)) < 0)
    {
      if (!(errnum == CEREBRO_ERR_CONNECT || errnum == CEREBRO_ERR_CONNECT_TIMEOUT))
        CEREBRO_DBG(("low_timeout_connect: %d", errnum));
      
      bgl_ciod_failures++;
      if (bgl_ciod_failures >= bgl_ciod_failure_max)
        bgl_ciod_state = 0;
    }
  else
    {
      bgl_ciod_failures = 0;
      bgl_ciod_state = 1;
      close(fd);
    }
  
  *metric_value_type = CEREBRO_METRIC_VALUE_TYPE_U_INT32;
  *metric_value_len = sizeof(u_int32_t);
  *metric_value = (void *)&bgl_ciod_state;

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

#if WITH_STATIC_MODULES
struct cerebro_metric_module_info bgl_ciod_metric_module_info =
#else  /* !WITH_STATIC_MODULES */
struct cerebro_metric_module_info metric_module_info =
#endif /* !WITH_STATIC_MODULES */
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
