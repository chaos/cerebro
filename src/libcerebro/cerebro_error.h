/*****************************************************************************\
 *  $Id: cerebro_error.h,v 1.2 2005-05-03 22:46:34 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_ERROR_H
#define _CEREBRO_ERROR_H

#include <pthread.h>

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
 * cerebro_err_register_mutex
 *
 * Register a mutex with the cerebro error lib.  All error outputs to
 * stderr or stdout will require this mutex before output.
 */
void cerebro_err_register_mutex(pthread_mutex_t *mutex);

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
 * Calls err_debug() with appropriately locks.  Should not be called
 * if debug_output_mutex is already held.  In that situation, fprintf()
 * should simply be called.
 */
void cerebro_err_debug(const char *fmt, ...);

/* 
 * cerebro_err_output
 *
 * Calls err_output() with appropriately locks.  Should not be called
 * if debug_output_mutex is already held.  In that situation, fprintf()
 * should simply be called.
 */
void cerebro_err_output(const char *fmt, ...);

/* 
 * cerebro_err_exit
 *
 * Calls err_exit() with appropriately locks.  Should not be called
 * if debug_output_mutex is already held.  In that situation, fprintf()
 * and exit() should simply be called.
 */
void cerebro_err_exit(const char *fmt, ...);

#endif /* _CEREBRO_ERROR_H */
