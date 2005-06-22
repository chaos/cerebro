/*****************************************************************************\
 *  $Id: cerebrod.c,v 1.67 2005-06-22 23:28:08 achu Exp $
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
#include "cerebrod_metric.h"
#include "cerebrod_speaker.h"

#include "clusterlist_module.h"

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

/*
 * clusterlist_handle
 *
 * Handle for clusterlist module
 */
clusterlist_module_t clusterlist_handle;

extern struct cerebrod_config conf;

extern int cerebrod_listener_initialization_complete;
extern pthread_cond_t cerebrod_listener_initialization_complete_cond;
extern pthread_mutex_t cerebrod_listener_initialization_complete_lock;

extern int cerebrod_metric_initialization_complete;
extern pthread_cond_t cerebrod_metric_initialization_complete_cond;
extern pthread_mutex_t cerebrod_metric_initialization_complete_lock;

/*
 * _cerebrod_post_config_setup_initialization
 *
 * Perform initialization routines after configuration is determined
 */
static void
_cerebrod_post_config_setup_initialization(void)
{
  if (!(clusterlist_handle = clusterlist_module_load()))
    cerebro_err_exit("%s(%s:%d): clusterlist_module_load",
                     __FILE__, __FUNCTION__, __LINE__);
  
  if (clusterlist_module_setup(clusterlist_handle) < 0)
    cerebro_err_exit("%s(%s:%d): clusterlist_module_setup",
                     __FILE__, __FUNCTION__, __LINE__);

  Signal(SIGPIPE, SIG_IGN);
}

int 
main(int argc, char **argv)
{
  cerebro_err_init(argv[0]);
  cerebro_err_set_flags(CEREBRO_ERROR_STDERR | CEREBRO_ERROR_SYSLOG);

  cerebrod_config_setup(argc, argv);

  _cerebrod_post_config_setup_initialization();

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

  /* Start metric before the listener begins receiving data. */
  if (conf.metric_server)
    {
      pthread_t thread;
      pthread_attr_t attr;

      Pthread_attr_init(&attr);
      Pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      Pthread_create(&thread, &attr, cerebrod_metric, NULL);
      Pthread_attr_destroy(&attr);

      /* Wait for initialization to complete */
      Pthread_mutex_lock(&cerebrod_metric_initialization_complete_lock);
      while (cerebrod_metric_initialization_complete == 0)
        Pthread_cond_wait(&cerebrod_metric_initialization_complete_cond,
                          &cerebrod_metric_initialization_complete_lock);
      Pthread_mutex_unlock(&cerebrod_metric_initialization_complete_lock);
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
          Pthread_create(&thread, &attr, cerebrod_listener, NULL);
          Pthread_attr_destroy(&attr);
        }

      /* Wait for initialization to complete */
      Pthread_mutex_lock(&cerebrod_listener_initialization_complete_lock);
      while (cerebrod_listener_initialization_complete == 0)
        Pthread_cond_wait(&cerebrod_listener_initialization_complete_cond,
                          &cerebrod_listener_initialization_complete_lock);
      Pthread_mutex_unlock(&cerebrod_listener_initialization_complete_lock);
    }

  /* Start speaker */
  if (conf.speak)
    {
      pthread_t thread;
      pthread_attr_t attr;

      Pthread_attr_init(&attr);
      Pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      Pthread_create(&thread, &attr, cerebrod_speaker, NULL);
      Pthread_attr_destroy(&attr);
    }

  for (;;) 
    sleep(INT_MAX);

  return 0;			/* NOT REACHED */
}
