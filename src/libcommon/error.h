/*****************************************************************************\
 *  $Id: error.h,v 1.5 2005-03-21 16:48:21 achu Exp $
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

#define ERROR_STDERR 0x01
#define ERROR_SYSLOG 0x02

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
