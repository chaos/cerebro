/*****************************************************************************\
 *  $Id: cerebro_metric_common.c,v 1.3 2006-11-15 00:12:30 chu11 Exp $
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

#include "cerebro_metric_common.h"
#include "debug.h"

int
common_metric_setup_do_nothing(void)
{
  /* nothing to do */
  return 0;
}

int
common_metric_cleanup_do_nothing(void)
{
  /* nothing to do */
  return 0;
}

int 
common_metric_get_metric_period_0(int *period)
{
  if (!period)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  *period = 0;
  return 0;
}

int 
common_metric_get_metric_period_60(int *period)
{
  if (!period)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  *period = 60;
  return 0;
}

int 
common_metric_get_metric_period_300(int *period)
{
  if (!period)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  *period = 300;
  return 0;
}

int
common_metric_get_metric_flags_none(u_int32_t *flags)
{
  if (!flags)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  *flags = 0;
  return 0;
}

int
common_metric_destroy_metric_value_do_nothing(void *metric_value)
{
  return 0;
}

int
common_metric_destroy_metric_value_free_value(void *metric_value)
{
  if (!metric_value)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  free(metric_value);
  return 0;
}

Cerebro_metric_thread_pointer
common_metric_get_metric_thread_null(void)
{
  return NULL;
}

int
common_metric_send_message_function_pointer_unused(Cerebro_metric_send_message function_pointer)
{
  return 0;
}
