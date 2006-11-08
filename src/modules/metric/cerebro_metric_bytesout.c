/*****************************************************************************\
 *  $Id: cerebro_metric_bytesout.c,v 1.3 2006-11-08 00:34:05 chu11 Exp $
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

#include "cerebro.h"
#include "cerebro/cerebro_metric_module.h"

#include "cerebro_metric_network.h"
#include "debug.h"

#define BYTESOUT_METRIC_MODULE_NAME  "bytesout"
#define BYTESOUT_METRIC_NAME         "bytesout"

/*
 * bytesout_metric_setup
 *
 * bytesout metric module setup function.  Read and store the bytesout
 * out of /proc.
 */
static int
bytesout_metric_setup(void)
{
  /* nothing to do */
  return 0;
}

/*
 * bytesout_metric_cleanup
 *
 * bytesout metric module cleanup function
 */
static int
bytesout_metric_cleanup(void)
{
  /* nothing to do */
  return 0;
}

/*
 * bytesout_metric_get_metric_name
 *
 * bytesout metric module get_metric_name function
 */
static char *
bytesout_metric_get_metric_name(void)
{
  return BYTESOUT_METRIC_NAME;
}

/*
 * bytesout_metric_get_metric_period
 *
 * bytesout metric module get_metric_period function
 */
static int
bytesout_metric_get_metric_period(int *period)
{
  if (!period)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }
  
  *period = 300;
  return 0;
}

/*
 * bytesout_metric_get_metric_value
 *
 * bytesout metric module get_metric_value function
 */
static int
bytesout_metric_get_metric_value(unsigned int *metric_value_type,
                                 unsigned int *metric_value_len,
                                 void **metric_value)
{
  u_int64_t bytesoutval;
  u_int64_t *bytesoutptr = NULL;
  int rv = -1;

  if (!metric_value_type || !metric_value_len || !metric_value)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }
  
  if (cerebro_metric_get_network(NULL,
                                 &bytesoutval,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL) < 0)
    goto cleanup;
  
  if (!(bytesoutptr = (u_int64_t *)malloc(sizeof(u_int64_t))))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      goto cleanup;
    }
  
  *bytesoutptr = bytesoutval;

  *metric_value_type = CEREBRO_DATA_VALUE_TYPE_U_INT64;
  *metric_value_len = sizeof(u_int64_t);
  *metric_value = (void *)bytesoutptr;

  rv = 0;
 cleanup:
  if (rv < 0 && bytesoutptr)
    free(bytesoutptr);
  return rv;
}

/*
 * bytesout_metric_destroy_metric_value
 *
 * bytesout metric module destroy_metric_value function
 */
static int
bytesout_metric_destroy_metric_value(void *metric_value)
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
 * bytesout_metric_get_metric_thread
 *
 * bytesout metric module get_metric_thread function
 */
static Cerebro_metric_thread_pointer
bytesout_metric_get_metric_thread(void)
{
  return NULL;
}

/*
 * bytesout_metric_send_heartbeat_function_pointer
 *
 * bytesout metric module send_heartbeat_function_pointer function
 */
static int
bytesout_metric_send_heartbeat_function_pointer(Cerebro_metric_send_heartbeat function_pointer)
{
  return 0;
}

#if WITH_STATIC_MODULES
struct cerebro_metric_module_info bytesout_metric_module_info =
#else  /* !WITH_STATIC_MODULES */
struct cerebro_metric_module_info metric_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    BYTESOUT_METRIC_MODULE_NAME,
    &bytesout_metric_setup,
    &bytesout_metric_cleanup,
    &bytesout_metric_get_metric_name,
    &bytesout_metric_get_metric_period,
    &bytesout_metric_get_metric_value,
    &bytesout_metric_destroy_metric_value,
    &bytesout_metric_get_metric_thread,
    &bytesout_metric_send_heartbeat_function_pointer,
  };
