/*****************************************************************************\
 *  $Id: cerebrod.c,v 1.16 2005-03-16 21:06:45 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include "cerebrod.h"
#include "cerebrod_cache.h"
#include "cerebrod_clusterlist.h"
#include "cerebrod_config.h"
#include "cerebrod_daemon.h"
#include "cerebrod_listener.h"
#include "cerebrod_speaker.h"
#include "cerebrod_updown.h"
#include "error.h"
#include "wrappers.h"

#ifndef NDEBUG
pthread_mutex_t debug_output_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif /* NDEBUG */

extern struct cerebrod_config conf;

extern int cerebrod_listener_initialization_complete;
extern pthread_cond_t cerebrod_listener_initialization_cond;
extern pthread_mutex_t cerebrod_listener_initialization_complete_lock;

extern int cerebrod_updown_initialization_complete;
extern pthread_cond_t cerebrod_updown_initialization_cond;
extern pthread_mutex_t cerebrod_updown_initialization_complete_lock;

static void
_cerebrod_pre_config_initialization(void)
{
  cerebrod_cache();
}

static void
_cerebrod_post_config_initialization(void)
{
  cerebrod_clusterlist_setup();
  cerebrod_clusterlist_init();
}

int 
main(int argc, char **argv)
{
  err_init(argv[0]);
  err_set_flags(ERROR_STDERR | ERROR_SYSLOG);

  _cerebrod_pre_config_initialization();

  cerebrod_config(argc, argv);

  _cerebrod_post_config_initialization();

  if (!conf.debug)
    {
      cerebrod_daemon_init();
      err_set_flags(ERROR_SYSLOG);
    }
  else
    err_set_flags(ERROR_STDERR);

  /* Call after daemonization, since daemonization closes currently open fds */
  openlog(argv[0], LOG_ODELAY | LOG_PID, LOG_DAEMON);

  /* Start servers first, because they need to be initialized before
   * the listener my begin receiving data.
   */
  if (conf.updown_server)
    {
      pthread_t thread;
      pthread_attr_t attr;

      Pthread_attr_init(&attr);
      Pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      Pthread_create(&thread, &attr, cerebrod_updown, NULL);
      Pthread_attr_destroy(&attr);

      /* Wait for initialization to complete */
      Pthread_mutex_lock(&cerebrod_updown_initialization_complete_lock);
      while (cerebrod_updown_initialization_complete == 0)
        Pthread_cond_wait(&cerebrod_updown_initialization_cond,
                          &cerebrod_updown_initialization_complete_lock);
      Pthread_mutex_unlock(&cerebrod_updown_initialization_complete_lock);
    }

  /* Start listener before speaker, since the listener may need to
   * listen for packets from a later created speaker thread
   */
  if (conf.listen)
    {
      int i;

      for (i = 0; i < conf.listen_threads; i++)
        {
          pthread_t thread;
          pthread_attr_t attr;

          Pthread_attr_init(&attr);
          Pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
          Pthread_create(&thread, &attr, cerebrod_listener, NULL);
          Pthread_attr_destroy(&attr);
        }

      /* Wait for initialization to complete */
      Pthread_mutex_lock(&cerebrod_listener_initialization_complete_lock);
      while (cerebrod_listener_initialization_complete == 0)
        Pthread_cond_wait(&cerebrod_listener_initialization_cond,
                          &cerebrod_listener_initialization_complete_lock);
      Pthread_mutex_unlock(&cerebrod_listener_initialization_complete_lock);
    }

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
  return 0;			/* NOT REACHED */
}
