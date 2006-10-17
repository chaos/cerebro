/*****************************************************************************\
 *  $Id: cerebro_event_updown.c,v 1.1.2.1 2006-10-17 06:42:13 chu11 Exp $
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
#include <stdarg.h>
#endif /* STDC_HEADERS */
#include <time.h>
#include <errno.h>

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_event_module.h"

#include "debug.h"
#include "hash.h"
#include "list.h"

#define UPDOWN_EVENT_MODULE_NAME    "updown"
#define UPDOWN_EVENT_METRIC_NAMES   "boottime"
#define UPDOWN_EVENT_TIMEOUT_LENGTH 60
#define UPDOWN_HASH_SIZE            1024

/*
 * updown_event_setup
 *
 * updown event module setup function.
 */
static int
updown_event_setup(void)
{
  return 0;
}

/*
 * updown_event_cleanup
 *
 * updown event module cleanup function
 */
static int
updown_event_cleanup(void)
{
  return 0;
}

/*
 * updown_event_metric_names
 *
 * updown event module metric_names function
 */
static char *
updown_event_metric_names(void)
{
  return UPDOWN_EVENT_METRIC_NAMES;
}

/*
 * updown_event_timeout_length
 *
 * updown event module timeout_length function
 */
static int
updown_event_timeout_length(void)
{
  return UPDOWN_EVENT_TIMEOUT_LENGTH;
}

/*
 * updown_event_node_timeout
 *
 * updown event module node_timeout function
 */
static int
updown_event_node_timeout(const char *nodename,
                          struct cerebro_event *event)
{
  return 0;
}

/*
 * updown_event_metric_data
 *
 * updown event module metric_data function.  Store results the
 * updown database appropriately.
 */
static int 
updown_event_metric_data(const char *nodename,
                         const char *metric_name,
                         unsigned int metric_value_type,
                         unsigned int metric_value_len,
                         void *metric_value,
                         struct cerebro_event *event)
{
  return 0;
}

#if WITH_STATIC_MODULES
struct cerebro_event_module_info boottime_event_module_info =
#else  /* !WITH_STATIC_MODULES */
struct cerebro_event_module_info event_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    UPDOWN_EVENT_MODULE_NAME,
    &updown_event_setup,
    &updown_event_cleanup,
    &updown_event_metric_names,
    &updown_event_timeout_length,
    &updown_event_node_timeout,
    &updown_event_metric_data,
  };
