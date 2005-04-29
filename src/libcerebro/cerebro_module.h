/*****************************************************************************\
 *  $Id: cerebro_module.h,v 1.1 2005-04-29 00:37:06 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_MODULE_H
#define _CEREBRO_MODULE_H

/*
 * Cerebro_load_module
 *
 * function prototype for loading a module. Passed a module
 * file to load.
 *
 * Returns 1 on loading success, 0 on loading failure, -1 on fatal error
 */
typedef int (*Cerebro_load_module)(char *);

#endif /* _CEREBRO_MODULE_H */
