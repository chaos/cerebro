/*****************************************************************************\
 *  $Id: cerebrod.c,v 1.74 2005-07-07 21:48:34 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include "cerebro/cerebro_error.h"

#include "cerebrod.h"
#include "cerebrod_daemon.h"
#include "cerebrod_config.h"
#include "cerebrod_listener.h"
#include "cerebrod_metric_server.h"
#include "cerebrod_speaker.h"

#include "wrappers.h"

#if CEREBRO_DEBUG
/*  
 * debug_output_mutex
 *
 * To coordinate output of debugging info to stderr.
 *
 * Locking Rule: Always lock data structure locks before grabbing
 * debugging locks.
 *
 * Locking Rule: Only lock around fprintf or similar statements.  Do
 * not lock around cerebro_err_debug or similar statements.
 */
pthread_mutex_t debug_output_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif /* CEREBRO_DEBUG */

extern struct cerebrod_config conf;

extern int listener_init;
extern pthread_cond_t listener_init_cond;
extern pthread_mutex_t listener_init_lock;

extern int metric_server_init;
extern pthread_cond_t metric_server_init_cond;
extern pthread_mutex_t metric_server_init_lock;

int 
main(int argc, char **argv)
{
  cerebro_err_init(argv[0]);
  cerebro_err_set_flags(CEREBRO_ERROR_STDERR | CEREBRO_ERROR_SYSLOG);

  cerebrod_config_setup(argc, argv);

#if CEREBRO_DEBUG
  if (!conf.debug)
    {
      cerebrod_daemon_init();
      cerebro_err_set_flags(CEREBRO_ERROR_SYSLOG);
    }
  else
    cerebro_err_set_flags(CEREBRO_ERROR_STDERR);
#else  /* !CEREBRO_DEBUG */
  cerebrod_daemon_init();
  cerebro_err_set_flags(CEREBRO_ERROR_SYSLOG);
#endif /* !CEREBRO_DEBUG */

  /* Call after daemonization, since daemonization closes currently
   * open fds 
   */
  openlog(argv[0], LOG_ODELAY | LOG_PID, LOG_DAEMON);

  /* Start metric server before the listener begins receiving data. */
  if (conf.metric_server)
    {
      pthread_t thread;
      pthread_attr_t attr;

      Pthread_attr_init(&attr);
      Pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      Pthread_attr_setstacksize(&attr, CEREBROD_THREAD_STACKSIZE);
      Pthread_create(&thread, &attr, cerebrod_metric_server, NULL);
      Pthread_attr_destroy(&attr);

      /* Wait for initialization to complete */
      Pthread_mutex_lock(&metric_server_init_lock);
      while (!metric_server_init)
        Pthread_cond_wait(&metric_server_init_cond, &metric_server_init_lock);
      Pthread_mutex_unlock(&metric_server_init_lock);
    }

  /* Start listening server before speaker so that listener
   * can receive packets from a later created speaker
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
          Pthread_attr_setstacksize(&attr, CEREBROD_THREAD_STACKSIZE);
          Pthread_create(&thread, &attr, cerebrod_listener, NULL);
          Pthread_attr_destroy(&attr);
        }

      /* Wait for initialization to complete */
      Pthread_mutex_lock(&listener_init_lock);
      while (!listener_init)
        Pthread_cond_wait(&listener_init_cond, &listener_init_lock);
      Pthread_mutex_unlock(&listener_init_lock);
    }

  /* Start speaker */
  if (conf.speak)
    {
      pthread_t thread;
      pthread_attr_t attr;

      Pthread_attr_init(&attr);
      Pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      Pthread_attr_setstacksize(&attr, CEREBROD_THREAD_STACKSIZE);
      Pthread_create(&thread, &attr, cerebrod_speaker, NULL);
      Pthread_attr_destroy(&attr);
    }

  for (;;) 
    sleep(INT_MAX);

  return 0;			/* NOT REACHED */
}
