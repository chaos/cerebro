/*****************************************************************************\
 *  $Id: cerebrod_error.h,v 1.1 2005-03-29 21:30:29 achu Exp $
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

/* 
 * cerebrod_err_debug
 *
 * Calls err_debug() with appropriately locks.  Should not be called
 * if debug_output_mutex is already held.  In that situation, err_debug()
 * should simply be called.
 */
void cerebrod_err_debug(const char *fmt, ...);

#endif /* _CEREBROD_ERROR_H */
