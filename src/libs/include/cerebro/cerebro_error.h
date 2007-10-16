/*****************************************************************************\
 *  $Id: cerebro_error.h,v 1.4 2007-10-16 22:43:16 chu11 Exp $
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

#ifndef _CEREBRO_ERROR_H
#define _CEREBRO_ERROR_H

#define CEREBRO_ERROR_STDOUT  0x0001
#define CEREBRO_ERROR_STDERR  0x0002
#define CEREBRO_ERROR_SYSLOG  0x0004

/*  
 * cerebro_err_init
 *
 * Initializes cerebro error lib
 */
void cerebro_err_init(char *prog);

/*
 * cerebro_err_get_flags
 *
 * Returns the currently set flags
 */
int cerebro_err_get_flags(void);

/*  
 * cerebro_err_set_flags
 *
 * Sets the error lib flags to 'flags'.
 */
void cerebro_err_set_flags(int flags);

/* 
 * cerebro_err_debug
 *
 * Outputs error debugging.  
 */
void cerebro_err_debug(const char *fmt, ...);

/* 
 * cerebro_err_output
 *
 * Calls error output.  
 */
void cerebro_err_output(const char *fmt, ...);

/* 
 * cerebro_err_exit
 *
 * Outputs error and exits
 */
void cerebro_err_exit(const char *fmt, ...);

#endif /* _CEREBRO_ERROR_H */
