/*****************************************************************************\
 *  $Id: cerebrod_daemon.c,v 1.2 2004-12-27 16:48:27 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <syslog.h>
#include <assert.h>

#include "cerebrod_daemon.h"
#include "error.h"
#include "wrappers.h"

void
cerebrod_daemon_init(const char *progname)
{
  /* Based on code in Unix network programming by R. Stevens */
  pid_t pid;
  int i;
 
  assert(progname);

  pid = Fork();
  if (pid != 0)			/* Terminate Parent */
    exit(0);
 
  setsid();
 
  Signal(SIGHUP, SIG_IGN);
   
  pid = Fork();
  if (pid != 0)			/* Terminate 1st Child */
    exit(0);

  Chdir("/");
 
  Umask(0);
   
  /* Don't use Close() wrapper, we don't want to exit on error */
  for (i = 0; i < 64; i++)
    close(i);			
 
  openlog(progname, LOG_ODELAY | LOG_PID, LOG_DAEMON);
}
