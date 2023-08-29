/*****************************************************************************\
 *  $Id: cerebro_metric_txerrs.c,v 1.13 2010-02-02 01:01:21 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2018 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2005-2007 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <https://github.com/chaos/cerebro>.
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

#include "cerebro.h"
#include "cerebro/cerebro_metric_module.h"

#include "cerebro_metric_common.h"
#include "cerebro_metric_network.h"
#include "debug.h"

#define TXERRS_METRIC_MODULE_NAME  "txerrs"
#define TXERRS_METRIC_NAME         "txerrs"

/*
 * txerrs_metric_get_metric_name
 *
 * txerrs metric module get_metric_name function
 */
static char *
txerrs_metric_get_metric_name(void)
{
  return TXERRS_METRIC_NAME;
}

/*
 * txerrs_metric_get_metric_value
 *
 * txerrs metric module get_metric_value function
 */
static int
txerrs_metric_get_metric_value(unsigned int *metric_value_type,
			       unsigned int *metric_value_len,
			       void **metric_value)
{
  u_int32_t txerrsval;
  u_int32_t *txerrsptr = NULL;
  int rv = -1;

  if (!metric_value_type || !metric_value_len || !metric_value)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  if (cerebro_metric_get_network(NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &txerrsval) < 0)
    goto cleanup;

  if (!(txerrsptr = (u_int32_t *)malloc(sizeof(u_int32_t))))
    {
      CEREBRO_ERR(("malloc: %s", strerror(errno)));
      goto cleanup;
    }

  *txerrsptr = txerrsval;

  *metric_value_type = CEREBRO_DATA_VALUE_TYPE_U_INT32;
  *metric_value_len = sizeof(u_int32_t);
  *metric_value = (void *)txerrsptr;

  rv = 0;
 cleanup:
  if (rv < 0 && txerrsptr)
    free(txerrsptr);
  return rv;
}

#if WITH_STATIC_MODULES
struct cerebro_metric_module_info txerrs_metric_module_info =
#else  /* !WITH_STATIC_MODULES */
struct cerebro_metric_module_info metric_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    TXERRS_METRIC_MODULE_NAME,
    &common_metric_interface_version,
    &common_metric_setup_do_nothing,
    &common_metric_cleanup_do_nothing,
    &txerrs_metric_get_metric_name,
    &common_metric_get_metric_period_300,
    &common_metric_get_metric_flags_none,
    &txerrs_metric_get_metric_value,
    &common_metric_destroy_metric_value_free_value,
    &common_metric_get_metric_thread_null,
    &common_metric_send_message_function_pointer_unused,
  };
