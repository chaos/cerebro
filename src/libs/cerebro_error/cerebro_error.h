/*****************************************************************************\
 *  $Id: cerebro_error.h,v 1.1 2005-06-17 22:02:28 achu Exp $
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
