/*****************************************************************************\
 *  $Id: cerebrod_daemon.c,v 1.3 2005-01-18 18:43:35 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <assert.h>

#include "cerebrod_daemon.h"
#include "error.h"
#include "wrappers.h"

void
cerebrod_daemon_init(void)
{
  /* Based on code in Unix network programming by R. Stevens */
  pid_t pid;
  int i;
 
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
}
