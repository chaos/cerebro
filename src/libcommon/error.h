/*****************************************************************************\
 *  $Id: error.h,v 1.4 2005-03-17 00:24:25 achu Exp $
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

void err_init(char *prog);
int err_get_flags(void);
void err_set_flags(int flags);

void err_debug(const char *fmt, ...);
void err_output(const char *fmt, ...);
void err_exit(const char *fmt, ...);

#endif /* _ERROR_H */
