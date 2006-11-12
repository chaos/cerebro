/*****************************************************************************\
 *  $Id: cerebro_metric_common.h,v 1.1 2006-11-12 07:43:08 chu11 Exp $
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

#ifndef _CEREBRO_METRIC_COMMON
#define _CEREBRO_METRIC_COMMON

#include "cerebro/cerebro_metric_module.h"

/*
 * common_metric_setup_do_nothing
 *
 * common metric module setup function that does nothing.
 */
int common_metric_setup_do_nothing(void);

/*
 * common_metric_cleanup_do_nothing
 *
 * common metric module cleanup function that does nothing
 */
int common_metric_cleanup_do_nothing(void);

/* 
 * common_metric_get_metric_period_0
 * 
 * common metric module get_metric_period function returning a period
 * of 0
 */
int common_metric_get_metric_period_0(int *period);

/* 
 * common_metric_get_metric_period_60
 * 
 * common metric module get_metric_period function returning a period
 * of 60
 */
int common_metric_get_metric_period_60(int *period);

/* 
 * common_metric_get_metric_period_300
 * 
 * common metric module get_metric_period function returning a period
 * of 300
 */
int common_metric_get_metric_period_300(int *period);

/*
 * common_metric_destroy_metric_value_do_nothing
 *
 * common metric module destroy_metric_value function that just frees
 * the value.
 */
int common_metric_destroy_metric_value_do_nothing(void *metric_value);

/*
 * common_metric_destroy_metric_value_free_value
 *
 * common metric module destroy_metric_value function that just frees
 * the value.
 */
int common_metric_destroy_metric_value_free_value(void *metric_value);

/*
 * common_metric_get_metric_thread_null
 *
 * common metric module get_metric_thread function that doesn't
 * use a thread for monitoring.
 */
Cerebro_metric_thread_pointer common_metric_get_metric_thread_null(void);

/*
 * common_metric_send_heartbeat_function_pointer_unused
 *
 * common metric module send_heartbeat_function_pointer function that
 * doesn't use the pointer.
 */
int common_metric_send_heartbeat_function_pointer_unused(Cerebro_metric_send_heartbeat function_pointer);

#endif /* _CEREBRO_METRIC_COMMON */
