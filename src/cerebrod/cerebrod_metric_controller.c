/*****************************************************************************\
 *  $Id: cerebrod_metric_controller.c,v 1.1 2005-07-11 20:35:34 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#include <sys/socket.h>

#include <assert.h>

#include "cerebro/cerebro_metric_control_protocol.h"

#include "cerebrod_metric_controller.h"

#include "debug.h"
#include "wrappers.h"

extern struct cerebrod_config conf;
#if CEREBRO_DEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* CEREBRO_DEBUG */

#if CEREBRO_DEBUG
#define CEREBROD_METRIC_CONTROLLER_UNIX_PATH  "/tmp/cerebro_metric_cerebrod_metric_controller"
#else  /* !CEREBRO_DEBUG */
#define CEREBROD_METRIC_CONTROLLER_UNIX_PATH  CEREBRO_MODULE_DIR "/cerebro_metric_cerebrod_metric_controller"
#endif  /* !CEREBRO_DEBUG */
#define CEREBROD_METRIC_CONTROLLER_BACKLOG    5

/*
 * metric_controller_init
 * metric_controller_init_cond
 * metric_controller_init_lock
 *
 * variables for synchronizing initialization between different pthreads
 * and signaling when it is complete
 */
int metric_controller_init = 0;
pthread_cond_t metric_controller_init_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t metric_controller_init_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * _metric_controller_initialize
 *
 * perform metric server initialization
 */
static void
_metric_controller_initialize(void)
{
  Pthread_mutex_lock(&metric_controller_init_lock);
  if (metric_controller_init)
    goto out;

  Signal(SIGPIPE, SIG_IGN);

  metric_controller_init++;
  Pthread_cond_signal(&metric_controller_init_cond);
 out:
  Pthread_mutex_unlock(&metric_controller_init_lock);
}

/*
 * _metric_controller_setup_socket
 *
 * Create and setup the controller socket.  Do not use wrappers in this
 * function.  We want to give the controller additional chances to
 * "survive" an error condition.
 *
 * Returns file descriptor on success, -1 on error
 */
static int
_metric_controller_setup_socket(void)
{
  struct sockaddr_un addr;
  int fd;
  
  if ((fd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
    {
      CEREBRO_DBG(("socket: %s", strerror(errno)));
      return -1;
    }

  if (strlen(CEREBROD_METRIC_CONTROLLER_UNIX_PATH) >= sizeof(addr.sun_path))
    {
      CEREBRO_DBG(("path '%s' too long", CEREBROD_METRIC_CONTROLLER_UNIX_PATH));
      goto cleanup;
    }

  /* unlink is allowed to fail */
  unlink(CEREBROD_METRIC_CONTROLLER_UNIX_PATH);
  
  memset(&addr, '\0', sizeof(struct sockaddr_un));
  addr.sun_family = AF_LOCAL;
  strncpy(addr.sun_path, 
          CEREBROD_METRIC_CONTROLLER_UNIX_PATH, 
          sizeof(addr.sun_path));
  
  if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) < 0)
    {
      CEREBRO_DBG(("bind: %s", strerror(errno)));
      goto cleanup;
    }
  
  if (listen(fd, CEREBROD_METRIC_CONTROLLER_BACKLOG) < 0)
    {
      CEREBRO_DBG(("listen: %s", strerror(errno)));
      goto cleanup;
    }
  
  return fd;

 cleanup:
  close(fd);
  return -1;
}

void *
cerebrod_metric_controller(void *arg)
{
  int controller_fd;

  _metric_controller_initialize();

  if ((controller_fd = _metric_controller_setup_socket()) < 0)
    CEREBRO_EXIT(("fd setup failed"));

  for (;;)
      sleep(1000);

  return NULL;			/* NOT REACHED */
}
