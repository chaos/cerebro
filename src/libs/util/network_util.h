/*****************************************************************************\
 *  $Id: network_util.h,v 1.6 2007-10-17 22:04:50 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007 Lawrence Livermore National Security, LLC.
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

#ifndef _NETWORK_UTIL_H
#define _NETWORK_UTIL_H

/* 
 * receive_data
 *
 * Receive a certain amount of data
 *
 * Returns bytes read on success, -1 and errnum on error.
 */
int receive_data(int fd,
                 unsigned int bytes_to_read,
                 char *buf,
                 unsigned int buflen,
                 unsigned int timeout_len,
                 unsigned int *errnum);

/*
 * low_timeout_connect
 *
 * Setup a tcp connection to 'hostname' and 'port' using a connection
 * timeout of 'connect_timeout'.
 *
 * Return file descriptor on success, -1 on error.
 */
int low_timeout_connect(const char *hostname,
                        unsigned int port,
                        unsigned int connect_timeout,
                        unsigned int *errnum);


#endif /* _NETWORK_UTIL_H */
