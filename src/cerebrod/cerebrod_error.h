/*****************************************************************************\
 *  $Id: cerebrod_error.h,v 1.2 2005-03-30 05:41:45 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_ERROR_H
#define _CEREBROD_ERROR_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#include <stdarg.h>
#endif /* STDC_HEADERS */

#include "error.h"

#define CEREBROD_ERROR_STDERR ERROR_STDERR
#define CEREBROD_ERROR_SYSLOG ERROR_SYSLOG

/*  
 * cerebrod_err_init
 *
 * Initializes cerebrod error lib
 */
void cerebrod_err_init(char *prog);

/*
 * cerebrod_err_get_flags
 *
 * Returns the currently set flags
 */
int cerebrod_err_get_flags(void);

/*  
 * cerebrod_err_set_flags
 *
 * Sets the error lib flags to 'flags'.
 */
void cerebrod_err_set_flags(int flags);

/* 
 * cerebrod_err_debug
 *
 * Calls err_debug() with appropriately locks.  Should not be called
 * if debug_output_mutex is already held.  In that situation, fprintf()
 * should simply be called.
 */
void cerebrod_err_debug(const char *fmt, ...);

/* 
 * cerebrod_err_output
 *
 * Calls err_output() with appropriately locks.  Should not be called
 * if debug_output_mutex is already held.  In that situation, fprintf()
 * should simply be called.
 */
void cerebrod_err_output(const char *fmt, ...);

/* 
 * cerebrod_err_exit
 *
 * Calls err_exit() with appropriately locks.  Should not be called
 * if debug_output_mutex is already held.  In that situation, fprintf()
 * and exit() should simply be called.
 */
void cerebrod_err_exit(const char *fmt, ...);

#endif /* _CEREBROD_ERROR_H */
