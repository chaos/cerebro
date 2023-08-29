/*****************************************************************************\
 *  $Id: cerebrod_monitor_update.h,v 1.8 2010-02-02 01:01:20 chu11 Exp $
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

#ifndef _CEREBROD_MONITOR_UPDATE_H
#define _CEREBROD_MONITOR_UPDATE_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_PTHREAD_H
#include <pthread.h>
#endif /* HAVE_PTHREAD_H */

#include "cerebro/cerebrod_message_protocol.h"
#include "cerebrod_listener_data.h"

#include "list.h"

/*
 * struct cerebrod_monitor_module_info
 *
 * contains cerebrod monitor module metric information
 */
struct cerebrod_monitor_module_info
{
  char *metric_names;
  int index;
  pthread_mutex_t monitor_lock;
};

/*
 * struct cerebrod_monitor_module_list
 *
 * Contains list of monitor module info list and a lock for thread
 * safety.
 */
struct cerebrod_monitor_module_list
{
  List monitor_list;
  pthread_mutex_t monitor_list_lock;
};

/*
 * cerebrod_monitor_modules_setup
 *
 * Setup monitor modules.
 *
 * Return 1 if modules are loaded, 0 if not, -1 on error
 */
int cerebrod_monitor_modules_setup(void);

/*
 * cerebrod_monitor_update
 *
 * Accepts recently gathered metric data to be passed to monitor modules.
 */
void cerebrod_monitor_modules_update(const char *nodename,
                                     struct cerebrod_node_data *nd,
                                     const char *metric_name,
                                     struct cerebrod_message_metric *mm);

#endif /* _CEREBROD_MONITOR_UPDATE_H */
