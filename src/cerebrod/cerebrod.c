/*****************************************************************************\
 *  $Id: cerebrod.c,v 1.84 2007-10-12 23:23:30 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2005 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <http://www.llnl.gov/linux/cerebro/>.
 *
 *  Cerebro is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  Cerebro is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with Genders; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <syslog.h>
#include <errno.h>

#include "cerebro/cerebro_error.h"

#include "cerebrod.h"
#include "cerebrod_daemon.h"
#include "cerebrod_config.h"
#include "cerebrod_event_node_timeout_monitor.h"
#include "cerebrod_event_server.h"
#include "cerebrod_listener.h"
#include "cerebrod_metric_controller.h"
#include "cerebrod_metric_server.h"
#include "cerebrod_speaker.h"

#include "wrappers.h"

#if CEREBRO_DEBUG
#if !WITH_CEREBROD_NO_THREADS
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
#endif /* !WITH_CEREBROD_NO_THREADS */
#endif /* CEREBRO_DEBUG */

#if !WITH_CEREBROD_SPEAKER_ONLY

extern int listener_init;
extern pthread_cond_t listener_init_cond;
extern pthread_mutex_t listener_init_lock;

extern int metric_controller_init;
extern pthread_cond_t metric_controller_init_cond;
extern pthread_mutex_t metric_controller_init_lock;

extern int metric_server_init;
extern pthread_cond_t metric_server_init_cond;
extern pthread_mutex_t metric_server_init_lock;

extern int event_node_timeout_monitor_init;
extern pthread_cond_t event_node_timeout_monitor_init_cond;
extern pthread_mutex_t event_node_timeout_monitor_init_lock;

extern int event_queue_monitor_init;
extern pthread_cond_t event_queue_monitor_init_cond;
extern pthread_mutex_t event_queue_monitor_init_lock;

extern int event_server_init;
extern pthread_cond_t event_server_init_cond;
extern pthread_mutex_t event_server_init_lock;

#endif /* !WITH_CEREBROD_SPEAKER_ONLY */

extern struct cerebrod_config conf;

/* for hostlist library */
void 
lsd_fatal_error(char *file, int line, char *mesg)
{
  cerebro_err_exit("LSD FATAL ERROR(%s:%d) %s: %s", file, line, mesg, strerror(errno));
}

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

#if !WITH_CEREBROD_SPEAKER_ONLY

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

  /* Start all the event server, queue monitor, and node timeout
   * threads after the listener thread, since they use data created by
   * the listener thread.
   */
  if (conf.event_server)
    {
      pthread_t thread;
      pthread_attr_t attr;

      Pthread_attr_init(&attr);
      Pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      Pthread_attr_setstacksize(&attr, CEREBROD_THREAD_STACKSIZE);
      Pthread_create(&thread, &attr, cerebrod_event_queue_monitor, NULL);
      Pthread_attr_destroy(&attr);

      /* Wait for initialization to complete */
      Pthread_mutex_lock(&event_queue_monitor_init_lock);
      while (!event_queue_monitor_init)
        Pthread_cond_wait(&event_queue_monitor_init_cond, &event_queue_monitor_init_lock);
      Pthread_mutex_unlock(&event_queue_monitor_init_lock);

      Pthread_attr_init(&attr);
      Pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      Pthread_attr_setstacksize(&attr, CEREBROD_THREAD_STACKSIZE);
      Pthread_create(&thread, &attr, cerebrod_event_server, NULL);
      Pthread_attr_destroy(&attr);

      /* Wait for initialization to complete */
      Pthread_mutex_lock(&event_server_init_lock);
      while (!event_server_init)
        Pthread_cond_wait(&event_server_init_cond, &event_server_init_lock);
      Pthread_mutex_unlock(&event_server_init_lock);

      Pthread_attr_init(&attr);
      Pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      Pthread_attr_setstacksize(&attr, CEREBROD_THREAD_STACKSIZE);
      Pthread_create(&thread, &attr, cerebrod_event_node_timeout_monitor, NULL);
      Pthread_attr_destroy(&attr);

      /* Wait for initialization to complete */
      Pthread_mutex_lock(&event_node_timeout_monitor_init_lock);
      while (!event_node_timeout_monitor_init)
        Pthread_cond_wait(&event_node_timeout_monitor_init_cond, 
                          &event_node_timeout_monitor_init_lock);
      Pthread_mutex_unlock(&event_node_timeout_monitor_init_lock);
    }

  /* Start metric controller - see comments at speaker below */ 
  if (conf.metric_controller)
    {
      pthread_t thread;
      pthread_attr_t attr;

      Pthread_attr_init(&attr);
      Pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      Pthread_attr_setstacksize(&attr, CEREBROD_THREAD_STACKSIZE);
      Pthread_create(&thread, &attr, cerebrod_metric_controller, NULL);
      Pthread_attr_destroy(&attr);

      /* Wait for initialization to complete */
      Pthread_mutex_lock(&metric_controller_init_lock);
      while (!metric_controller_init)
        Pthread_cond_wait(&metric_controller_init_cond, 
                          &metric_controller_init_lock);
      Pthread_mutex_unlock(&metric_controller_init_lock);
    }

#endif /* !WITH_CEREBROD_SPEAKER_ONLY */

  /* Start speaker  
   *
   * It may make more logical sense to start the metric controller
   * after the speaker since metric data cannot be propogated until
   * after the speaker has finished being setup.  We run the speaker
   * last b/c it is the common case.  Most machines (particularly
   * compute nodes in a cluster) will only speak, and do nothing else.
   * By having the speaker last, it does not need to run in a thread.
   * We run it out of "main" instead to minimize memory usage by
   * not needing to start the speaker in a thread.
   */
  if (conf.speak)
    cerebrod_speaker(NULL);

  /* If speak is set, we do not reach this point */

  for (;;) 
    sleep(INT_MAX);

  return 0;			/* NOT REACHED */
}
