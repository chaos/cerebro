/*****************************************************************************\
 *  $Id: cerebrod_metric_server.h,v 1.9 2007-10-16 22:43:15 chu11 Exp $
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
 *  with Cerebro; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
\*****************************************************************************/

#ifndef _CEREBROD_METRIC_SERVER_H
#define _CEREBROD_METRIC_SERVER_H

#include "cerebro/cerebro_metric_server_protocol.h"

#include "list.h"
 
/*
 * struct cerebrod_metric_name_evaluation_data
 *
 * Holds data for callback functions
 */
struct cerebrod_metric_name_evaluation_data
{
  int fd;
  List responses;
};

/*
 * struct cerebrod_metric_data_evaluation_data
 *
 * Holds data for callback functions
 */
struct cerebrod_metric_data_evaluation_data
{
  int fd;
  struct cerebro_metric_server_request *req;
  u_int32_t time_now;
  char *metric_name;
  List responses;
};

/*
 * cerebrod_metric_server
 *
 * Runs the cerebrod metric server thread
 *
 * Passed no argument
 *
 * Executed in detached state, no return value.
 */
void *cerebrod_metric_server(void *);

#endif /* _CEREBROD_METRIC_SERVER_H */
