/*****************************************************************************\
 *  $Id: error.h,v 1.1.1.1 2004-07-02 22:31:29 achu Exp $
\*****************************************************************************/

#ifndef _ERROR_H
#define _ERROR_H

#include <stdio.h>
#include <stdarg.h>

#define ERROR_STDERR 0x01
#define ERROR_SYSLOG 0x02

void err_init(char *prog);
int err_get_flags(void);
void err_set_flags(int flags);

void err_debug(const char *fmt, ...);
void err_output(const char *fmt, ...);
void err_exit(const char *fmt, ...);

#endif /* _ERROR_H */
