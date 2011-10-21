/*****************************************************************************\
 *  $Id: cerebrod_message_protocol.h,v 1.6 2010-02-02 01:01:20 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2011 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2005-2007 The Regents of the University of California.
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
 *  with Cerebro. If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#ifndef _CEREBROD_MESSAGE_PROTOCOL_H
#define _CEREBROD_MESSAGE_PROTOCOL_H

#include <sys/types.h>

#include <cerebro/cerebro_constants.h>

#define CEREBROD_MESSAGE_PROTOCOL_VERSION 2

/*
 * struct cerebrod_message_metric
 *
 * defines message metric data
 */
struct cerebrod_message_metric
{
  char metric_name[CEREBRO_MAX_METRIC_NAME_LEN];
  u_int32_t metric_value_type;
  u_int32_t metric_value_len;
  void *metric_value;
};

#define CEREBROD_MESSAGE_METRIC_HEADER_LEN  (CEREBRO_MAX_METRIC_NAME_LEN \
                                             + sizeof(u_int32_t) \
                                             + sizeof(u_int32_t))

/* 
 * struct cerebrod_message
 *
 * defines message data sent/received from each cerebrod daemon
 */
struct cerebrod_message 
{
  int32_t version;
  char nodename[CEREBRO_MAX_NODENAME_LEN];
  u_int32_t metrics_len;
  struct cerebrod_message_metric **metrics;
};

#define CEREBROD_MESSAGE_HEADER_LEN  (sizeof(int32_t) \
                                      + CEREBRO_MAX_NODENAME_LEN \
                                      + sizeof(u_int32_t))

#endif /* _CEREBROD_MESSAGE_PROTOCOL_H */
