/*****************************************************************************\
 *  $Id: error.h,v 1.7 2010-02-02 01:01:20 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2010 Lawrence Livermore National Security, LLC.
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
 *  with Cerebro.  If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#ifndef _ERROR_H
#define _ERROR_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#include <stdarg.h>
#endif /* STDC_HEADERS */

#define ERROR_STDOUT 0x0001
#define ERROR_STDERR 0x0002
#define ERROR_SYSLOG 0x0004

/*  
 * err_init
 *
 * Initialize error lib with program name.  This is usually the first
 * thing called from main, and is simply passed argv[0].
 */
void err_init(char *prog);

/* 
 * err_get_flags
 *
 * Returns the currently set flags
 */
int err_get_flags(void);

/* 
 * err_set_flags
 *
 * Sets the error lib flags to 'flags'.
 */
void err_set_flags(int flags);

/* 
 * err_debug
 *
 * Output a debug message
 */
void err_debug(const char *fmt, ...);

/*  
 * err_output
 *
 * Output an error message
 */
void err_output(const char *fmt, ...);

/* 
 * err_exit
 *
 * Output an error message and exit
 */
void err_exit(const char *fmt, ...);

#endif /* _ERROR_H */
