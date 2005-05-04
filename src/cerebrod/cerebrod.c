/*****************************************************************************\
 *  $Id: cerebrod.c,v 1.41 2005-05-04 01:00:55 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include "cerebro_error.h"
#include "cerebro_module.h"

#include "cerebrod.h"
#include "cerebrod_clusterlist.h"
#include "cerebrod_config.h"
#include "cerebrod_daemon.h"
#include "cerebrod_data.h"
#include "cerebrod_listener.h"
#include "cerebrod_speaker.h"
#include "cerebrod_updown.h"
#include "wrappers.h"

#ifndef NDEBUG
/*  
 * debug_output_mutex
 *
 * To coordinate output of debugging info to stderr.
 *
 * Locking Rule: Always lock data structure locks before grabbing
 * debugging locks.
 *
 * Lucking Rule: Only lock around fprintf or similar statements.
 */
pthread_mutex_t debug_output_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif /* NDEBUG */

extern struct cerebrod_config conf;

extern int cerebrod_listener_initialization_complete;
extern pthread_cond_t cerebrod_listener_initialization_complete_cond;
extern pthread_mutex_t cerebrod_listener_initialization_complete_lock;

extern int cerebrod_updown_initialization_complete;
extern pthread_cond_t cerebrod_updown_initialization_complete_cond;
extern pthread_mutex_t cerebrod_updown_initialization_complete_lock;

/* 
 * _cerebrod_pre_config_initialization
 *
 * Perform initialization routines prior to configuring cerebrod
 */
static void
_cerebrod_pre_config_initialization(void)
{
  if (cerebro_module_setup() < 0)
    cerebro_err_exit("%s(%s:%d): cerebro_module_setup",
		     __FILE__, __FUNCTION__, __LINE__);

  cerebrod_load_data();
}

/* 
 * _cerebrod_post_config_initialization
 *
 * Perform initialization routines after configuration is determined
 */
static void
_cerebrod_post_config_initialization(void)
{
  if (cerebrod_clusterlist_module_setup() < 0)
    cerebro_err_exit("%s(%s:%d): cerebrod_clusterlist_module_setup",
                     __FILE__, __FUNCTION__, __LINE__);

  if (conf.updown_server)
    Signal(SIGPIPE, SIG_IGN);
}

/* 
 * _cerebrod_cleanup
 *
 * Perform cerebrod cleanup.  Will never be called.  Used as place holder
 * to indicate appropriate functions to call in the future.
 */
#if 0
static void
_cerebrod_cleanup(void)
{
  if (cerebrod_clusterlist_module_found)
    cerebro_module_cleanup();
}
#endif

int 
main(int argc, char **argv)
{
  cerebro_err_init(argv[0]);
#ifdef NDEBUG
  cerebro_err_set_flags(CEREBRO_ERROR_STDERR 
			| CEREBRO_ERROR_SYSLOG);
#else  /* !NDEBUG */
  cerebro_err_set_flags(CEREBRO_ERROR_STDERR 
			| CEREBRO_ERROR_SYSLOG 
			| CEREBRO_ERROR_LIB 
			| CEREBRO_ERROR_MODULE);
#endif /* !NDEBUG */

  _cerebrod_pre_config_initialization();

  cerebrod_config_setup(argc, argv);

  _cerebrod_post_config_initialization();

#ifdef NDEBUG
  cerebrod_daemon_init();
  cerebro_err_set_flags(CEREBRO_ERROR_SYSLOG);
#else  /* !NDEBUG */
  if (!conf.debug)
    {
      cerebrod_daemon_init();
      cerebro_err_set_flags(CEREBRO_ERROR_SYSLOG
			    | CEREBRO_ERROR_LIB
			    | CEREBRO_ERROR_MODULE);
    }
  else
    {
      cerebro_err_register_mutex(&debug_output_mutex);
      cerebro_err_set_flags(CEREBRO_ERROR_STDERR 
			    | CEREBRO_ERROR_LIB
			    | CEREBRO_ERROR_MODULE);
    }
#endif /* !NDEBUG */

  /* Call after daemonization, since daemonization closes currently
   * open fds 
   */
  openlog(argv[0], LOG_ODELAY | LOG_PID, LOG_DAEMON);

  /* Start updown server.  Start before the listener begins receiving
   * data.
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
        Pthread_cond_wait(&cerebrod_updown_initialization_complete_cond,
                          &cerebrod_updown_initialization_complete_lock);
      Pthread_mutex_unlock(&cerebrod_updown_initialization_complete_lock);
    }

  /* Start listening server.  Start before speaker, since the listener
   * may need to listen for packets from a later created speaker
   * thread
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
