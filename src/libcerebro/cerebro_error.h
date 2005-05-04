/*****************************************************************************\
 *  $Id: cerebro_error.h,v 1.5 2005-05-04 23:54:06 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_ERROR_H
#define _CEREBRO_ERROR_H

#define CEREBRO_ERROR_STDOUT  0x0001
#define CEREBRO_ERROR_STDERR  0x0002
#define CEREBRO_ERROR_SYSLOG  0x0004
#define CEREBRO_ERROR_LIB     0x0008
#define CEREBRO_ERROR_MODULE  0x0010

/*
 * Cerebro_err_lock
 *
 * function prototype for error lib to call before output of
 * a message to stderr or stdout.
 */
typedef void (*Cerebro_err_lock)(void);

/*
 * Cerebro_err_unlock
 *
 * function prototype for error lib to call after output of
 * a message to stderr or stdout.
 */
typedef void (*Cerebro_err_unlock)(void);

/*  
 * cerebro_err_init
 *
 * Initializes cerebro error lib
 */
void cerebro_err_init(char *prog);

/* 
 * cerebro_err_register_locking
 *
 * Register locking functions with the cerebro error lib.  All error
 * outputs to stderr or stdout will require the locks to be held
 * 
 */
void cerebro_err_register_locking(Cerebro_err_lock lock,
				  Cerebro_err_unlock unlock);

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
 * if debug_output_mutex is already held.  In that situation,
 * fprintf() should simply be called.
 */
void cerebro_err_debug(const char *fmt, ...);

/* 
 * cerebro_err_debug_lib
 *
 * Calls err_debug() with appropriately locks and if CEREBRO_ERROR_LIB
 * is set.  Should not be called if mutex is already held.  In that
 * situation, fprintf() should simply be called.
 */
void cerebro_err_debug_lib(const char *fmt, ...);

/* 
 * cerebro_err_debug_module
 *
 * Calls err_debug() with appropriately locks and if CEREBRO_ERROR_MODULE
 * is set.  Should not be called if mutex is already held.  In that
 * situation, fprintf() should simply be called.
 */
void cerebro_err_debug_module(const char *fmt, ...);

/* 
 * cerebro_err_output
 *
 * Calls err_output() with appropriately locks.  Should not be called
 * if mutex is already held.  In that situation, fprintf() should
 * simply be called.
 */
void cerebro_err_output(const char *fmt, ...);

/* 
 * cerebro_err_exit
 *
 * Calls err_exit() with appropriately locks.  Should not be called if
 * mutex is already held.  In that situation, fprintf() and exit()
 * should simply be called.
 */
void cerebro_err_exit(const char *fmt, ...);

#endif /* _CEREBRO_ERROR_H */
