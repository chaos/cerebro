/*****************************************************************************\
 *  $Id: cerebrod.c,v 1.8 2005-01-18 18:43:35 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include "cerebrod.h"
#include "cerebrod_cache.h"
#include "cerebrod_config.h"
#include "cerebrod_daemon.h"
#include "cerebrod_speaker.h"
#include "error.h"
#include "wrappers.h"

struct cerebrod_config conf;
#ifndef NDEBUG
pthread_mutex_t debug_output_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif /* NDEBUG */

static void
_cerebrod_initialization(void)
{
  cerebrod_cache();
}

int 
main(int argc, char **argv)
{
  err_init(argv[0]);
  err_set_flags(ERROR_STDERR | ERROR_SYSLOG);

  _cerebrod_initialization();

  cerebrod_config(argc, argv);

  if (!conf.debug)
    {
      cerebrod_daemon_init();
      err_set_flags(ERROR_SYSLOG);
    }
  else
    err_set_flags(ERROR_STDERR);

  /* Call after daemonization, since daemonization closes currently open fds */
  openlog(argv[0], LOG_ODELAY | LOG_PID, LOG_DAEMON);

  if (conf.speak)
    {
      pthread_t thread;
      pthread_attr_t attr;

      Pthread_attr_init(&attr);
      Pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      Pthread_create(&thread, &attr, cerebrod_speaker, NULL);
      Pthread_attr_destroy(&attr);
    }

  for (;;) {}
  return 0;
}
