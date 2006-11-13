/*****************************************************************************\
 *  $Id: cerebro_metric_module.h,v 1.9.4.2 2006-11-13 02:27:49 chu11 Exp $
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

#ifndef _CEREBRO_METRIC_MODULE_H
#define _CEREBRO_METRIC_MODULE_H

#include <sys/types.h>
#include <cerebro/cerebrod_heartbeat_protocol.h>

#define CEREBRO_METRIC_MODULE_FLAGS_SEND_ON_PERIOD 0x1

/*
 * Cerebro_metric_setup
 *
 * function prototype for metric module function to setup the
 * module.  Required to be defined by each metric module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_metric_setup)(void);

/*
 * Cerebro_metric_cleanup
 *
 * function prototype for metric module function to cleanup.  Required
 * to be defined by each metric module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_metric_cleanup)(void);

/* 
 * Cerebro_metric_get_metric_name
 *
 * Returns the name of the metric metriced by this module Required to
 * be defined by each metric module.
 *
 * Returns string on success, -1 on error
 */
typedef char *(*Cerebro_metric_get_metric_name)(void);

/* 
 * Cerebro_metric_get_metric_period
 *
 * Retrieve the period in seconds that the metric value should be read
 * and propogated.  If the period is 0, the metric will be read and
 * propogated with every cerebro heartbeat. If the period is < 0, the
 * metric will be propogated only when instructed to. Note that the
 * period is not precise, and is only an approximation.  Data is only
 * propogated in cerebro heartbeats, therefore the period time
 * granularity will be related to the cerebro heartbeat period.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_metric_get_metric_period)(int *period);

/* 
 * Cerebro_metric_get_metric_flags
 *
 * function prototype for metric module function to indicate the flags
 * it supports.  Required to be defined by each metric module.
 * 
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_metric_get_metric_flags)(u_int32_t *flags);

/*
 * Cerebro_metric_get_metric_value
 *
 * function prototype for metric module function to get a metric
 * value.  Required to be defined by each metric module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_metric_get_metric_value)(unsigned int *metric_value_type,
                                               unsigned int *metric_value_len,
                                               void **metric_value);

/*
 * Cerebro_metric_destroy_metric_value
 *
 * function prototype for metric module function to destroy a metric
 * value if necessary.  Required to be defined by each metric module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_metric_destroy_metric_value)(void *metric_value);

/* 
 * Cerebro_metric_updated
 *
 * function prototype to inform the cerebrod daemon a metric
 * has been updated.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_metric_updated)(char *metric_name);

/* 
 * Cerebro_metric_thread_pointer
 *
 * function prototype for a thread which will be passed to
 * pthread_create
 *
 * The function will be passed a pointer to a 'Cerebro_metric_updated'
 * function can be called when the metric value is updated.  This
 * thread can perform any metric monitoring duties it pleases and
 * optionally call the 'Cerebro_metric_updated' function when a metric
 * value is updated.
 *
 * Typically the thread is used to watch or monitor for some event and
 * locally update data so that cerebrod will propogate the newly
 * received data from a 'get_metric_value' call.
 *
 * If the user wishes to use mutexes within the metric thread to
 * protect against concurrent access, the user is responsible for not
 * putting the locks in situations that can lead to a deadlock.  The
 * 'Cerebro_metric_updated' function call may require locks to
 * function appropriately within the cerebrod daemon.
 */
typedef void *(*Cerebro_metric_thread_pointer)(void *arg);

/*
 * Cerebro_metric_get_metric_thread
 *
 * function prototype for metric module function that will return a
 * pointer to function that will be executed as a detached thread.
 *
 * If a metric_thread is not needed, this function returns NULL.
 * Required to be defined by each metric module.
 *
 * Returns 0 on success, -1 on error
 */
typedef Cerebro_metric_thread_pointer (*Cerebro_metric_get_metric_thread)(void);

/* Cerebro_metric_send_heartbeat 
 *
 * function prototype to inform the cerebrod daemon to send
 * a heartbeat.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_metric_send_heartbeat)(struct cerebrod_heartbeat *hb);

/* 
 * Cerebro_metric_send_heartbeat_function_pointer
 *
 * function prototype to give a Cerebro_metric_send_heartbeat function
 * pointer to the metric module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_metric_send_heartbeat_function_pointer)(Cerebro_metric_send_heartbeat function_pointer);

/*
 * struct cerebro_metric_module_info 
 * 
 * contains metric module information and operations.  Required to be
 * defined in each metric module.
 */
struct cerebro_metric_module_info
{
  char *metric_module_name;
  Cerebro_metric_setup setup;
  Cerebro_metric_cleanup cleanup;
  Cerebro_metric_get_metric_name get_metric_name;
  Cerebro_metric_get_metric_period get_metric_period;
  Cerebro_metric_get_metric_flags get_metric_flags;
  Cerebro_metric_get_metric_value get_metric_value;
  Cerebro_metric_destroy_metric_value destroy_metric_value;
  Cerebro_metric_get_metric_thread get_metric_thread;
  Cerebro_metric_send_heartbeat_function_pointer send_heartbeat_function_pointer; 
};

#endif /* _CEREBRO_METRIC_MODULE_H */
