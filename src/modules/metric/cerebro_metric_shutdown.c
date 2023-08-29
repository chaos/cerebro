/*****************************************************************************\
 *  $Id: cerebro_metric_shutdown.c,v 1.13 2010-02-02 01:01:21 chu11 Exp $
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
#include <limits.h>
#include <sys/types.h>
#if HAVE_SIGNAL_H
#include <signal.h>
#endif /* HAVE_SIGNAL_H */
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <errno.h>

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_metric_module.h"
#include "cerebro/cerebrod_message_protocol.h"

#include "cerebro_metric_common.h"
#include "debug.h"

#define SHUTDOWN_METRIC_MODULE_NAME  "shutdown"
#define SHUTDOWN_METRIC_NAME         "shutdown"

/*
 * metric_shutdown
 *
 * Set to 1 when the machine is about to shutdown.  Really
 * a dummy value, not really needed.
 */
static u_int32_t metric_shutdown = 0;

/*
 * send_message_function
 *
 * Stores pointer to function to send a message
 */
Cerebro_metric_send_message send_message_function = NULL;

/*
 * _metric_shutdown_handler
 *
 * Will handle the SIGTERM (and SIGINT if we're debugging) signals.
 * After receiving the signal, will create and send the last
 * message, then kill the program.
 *
 * Although the author took reasonably good steps to make the primary
 * code survive SIGINT signals in system calls, it hasn't been
 * auditted fully.  Sending metrics/messages via signals probably
 * isn't a good idea.  This is an exception b/c we'll die right away.
 */
static void
_metric_shutdown_handler(int signum)
{
  struct cerebrod_message *hb = NULL;
  struct cerebrod_message_metric *hd = NULL;
  char nodename[CEREBRO_MAX_NODENAME_LEN+1];

  if (!(hb = (struct cerebrod_message *)malloc(sizeof(struct cerebrod_message))))
    {
      CEREBRO_ERR(("malloc: %s", strerror(errno)));
      goto cleanup;
    }

  memset(nodename, '\0', CEREBRO_MAX_NODENAME_LEN+1);
  if (gethostname(nodename, CEREBRO_MAX_NODENAME_LEN) < 0)
    {
      CEREBRO_ERR(("gethostname: %s", strerror(errno)));
      goto cleanup;
    }

  hb->version = CEREBROD_MESSAGE_PROTOCOL_VERSION;
  memcpy(hb->nodename, nodename, CEREBRO_MAX_NODENAME_LEN);

  hb->metrics_len = 1;
  if (!(hb->metrics = (struct cerebrod_message_metric **)malloc(sizeof(struct cerebrod_message_metric *)*(hb->metrics_len + 1))))
    goto cleanup;
  memset(hb->metrics, '\0', sizeof(struct cerebrod_message_metric *)*(hb->metrics_len + 1));

  if (!(hd = (struct cerebrod_message_metric *)malloc(sizeof(struct cerebrod_message_metric))))
    goto cleanup;
  memset(hd, '\0', sizeof(struct cerebrod_message_metric));

  /* need not overflow */
  strncpy(hd->metric_name, SHUTDOWN_METRIC_NAME, CEREBRO_MAX_METRIC_NAME_LEN);

  hd->metric_value_type = CEREBRO_DATA_VALUE_TYPE_U_INT32;
  hd->metric_value_len = sizeof(u_int32_t);
  metric_shutdown = 1;
  hd->metric_value = (void *)&metric_shutdown;

  hb->metrics[0] = hd;

  if ((*send_message_function)(hb) < 0)
    {
      CEREBRO_DBG(("cerebrod_send_message"));
      goto cleanup;
    }

 cleanup:
  exit(1);
}

/*
 * shutdown_metric_setup
 *
 * shutdown metric module setup function.  Read and store the shutdown
 * out of /proc.
 */
static int
shutdown_metric_setup(void)
{
  if (signal(SIGTERM, _metric_shutdown_handler) == SIG_ERR)
    {
      CEREBRO_ERR(("signal: %s", strerror(errno)));
      return (-1);
    }

#if CEREBRO_DEBUG
  /* So we can Ctrl+C for testing */
  if (signal(SIGINT, _metric_shutdown_handler) == SIG_ERR)
    {
      CEREBRO_ERR(("signal: %s", strerror(errno)));
      return (-1);
    }
#endif /* CEREBRO_DEBUG */

  return 0;
}

/*
 * shutdown_metric_get_metric_name
 *
 * shutdown metric module get_metric_name function
 */
static char *
shutdown_metric_get_metric_name(void)
{
  return SHUTDOWN_METRIC_NAME;
}

/*
 * shutdown_metric_get_metric_period
 *
 * shutdown metric module get_metric_period function
 */
static int
shutdown_metric_get_metric_period(int *period)
{
  if (!period)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  /* Rather than making the period -1 (so the metric is never sent
   * except on during shutdown when the signal is caught), we set the
   * period to INT_MAX, so that the value of '0' is sent once at the
   * beginning.
   */
  *period = INT_MAX;
  return 0;
}

/*
 * shutdown_metric_get_metric_value
 *
 * shutdown metric module get_metric_value function
 */
static int
shutdown_metric_get_metric_value(unsigned int *metric_value_type,
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
  *metric_value = (void *)&metric_shutdown;
  return 0;
}

/*
 * shutdown_metric_send_message_function_pointer
 *
 * shutdown metric module send_message_function_pointer function
 */
static int
shutdown_metric_send_message_function_pointer(Cerebro_metric_send_message function_pointer)
{
  if (!function_pointer)
    {
      CEREBRO_DBG(("invalid function_pointer"));
      return -1;
    }

  send_message_function = function_pointer;
  return 0;
}

#if WITH_STATIC_MODULES
struct cerebro_metric_module_info shutdown_metric_module_info =
#else  /* !WITH_STATIC_MODULES */
struct cerebro_metric_module_info metric_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    SHUTDOWN_METRIC_MODULE_NAME,
    &common_metric_interface_version,
    &shutdown_metric_setup,
    &common_metric_cleanup_do_nothing,
    &shutdown_metric_get_metric_name,
    &shutdown_metric_get_metric_period,
    &common_metric_get_metric_flags_none,
    &shutdown_metric_get_metric_value,
    &common_metric_destroy_metric_value_do_nothing,
    &common_metric_get_metric_thread_null,
    &shutdown_metric_send_message_function_pointer,
  };
