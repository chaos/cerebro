/*****************************************************************************\
 *  $Id: cerebro_metric_slurm_state.c,v 1.4 2005-06-24 20:42:28 achu Exp $
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
#include <sys/select.h>
#include <sys/un.h>
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else  /* !TIME_WITH_SYS_TIME */
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else /* !HAVE_SYS_TIME_H */
#  include <time.h>
# endif /* !HAVE_SYS_TIME_H */
#endif /* !TIME_WITH_SYS_TIME */
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#if HAVE_PTHREAD_H
#include <pthread.h>
#endif /* HAVE_PTHREAD_H */

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_error.h"
#include "cerebro/cerebro_metric_module.h"

#define SLURM_STATE_METRIC_MODULE_NAME      "slurm_state"
#define SLURM_STATE_METRIC_NAME             "slurm_state"
#if CEREBRO_DEBUG
#define SLURM_STATE_UNIX_PATH               "/tmp/cerebro_metric_slurm_state"
#else  /* !CEREBRO_DEBUG */
#define SLURM_STATE_UNIX_PATH               CEREBRO_MODULE_DIR "/cerebro_metric_slurm_state"
#endif  /* !CEREBRO_DEBUG */
#define SLURM_STATE_BACKLOG                 5
#define SLURM_STATE_REINITIALIZE_WAIT_TIME  5

/*
 * metric_slurm_state
 *
 * cached system slurm_state
 */
static u_int32_t metric_slurm_state = 0;

/* 
 * metric_slurm_state_lock
 *
 * Protect access to metric_slurm_state
 */
static pthread_mutex_t metric_slurm_state_lock = PTHREAD_MUTEX_INITIALIZER;

/* 
 * slurm_state_fd
 * 
 * Unix Domain socket in which slurm will connect to
 */
static int slurm_state_fd = -1;

/* 
 * _slurm_state_create_and_setup_socket
 *
 * Create and setup the server socket
 *
 * Returns file descriptor on success, -1 on error
 */
static int
_slurm_state_create_and_setup_socket(void)
{
  struct sockaddr_un addr;
  int temp_fd;

  if ((temp_fd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
    {      
      cerebro_err_debug("%s(%s:%d): socket: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      goto cleanup;
    }
  
  if (strlen(SLURM_STATE_UNIX_PATH) >= sizeof(addr.sun_path))
    {
      cerebro_err_debug("%s(%s:%d): path '%s' too long",
                        __FILE__, __FUNCTION__, __LINE__,
                        SLURM_STATE_UNIX_PATH);
      goto cleanup;
    }
  
  unlink(SLURM_STATE_UNIX_PATH);

  memset(&addr, '\0', sizeof(struct sockaddr_un));
  addr.sun_family = AF_LOCAL;
  strncpy(addr.sun_path, SLURM_STATE_UNIX_PATH, sizeof(addr.sun_path));

  if (bind(temp_fd, 
           (struct sockaddr *)&addr, 
           sizeof(struct sockaddr_un)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): bind: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      goto cleanup;
    }
  
  if (listen(temp_fd, SLURM_STATE_BACKLOG) < 0)
    {
      cerebro_err_debug("%s(%s:%d): listen: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      goto cleanup;
    }
  
  return temp_fd;

 cleanup:
  close(temp_fd);
  return -1;
}

/*
 * slurm_state_metric_setup
 *
 * slurm_state metric module setup function.
 */
static int
slurm_state_metric_setup(void)
{
  if ((slurm_state_fd = _slurm_state_create_and_setup_socket()) < 0)
    return -1;

  return 0;
}

/*
 * slurm_state_metric_cleanup
 *
 * slurm_state metric module cleanup function
 */
static int
slurm_state_metric_cleanup(void)
{
  /* nothing to do */
  return 0;
}

/*
 * slurm_state_metric_get_metric_name
 *
 * slurm_state metric module get_metric_name function
 */
static char *
slurm_state_metric_get_metric_name(void)
{
  return SLURM_STATE_METRIC_NAME;
}

/*
 * slurm_state_metric_get_metric_period
 *
 * slurm_state metric module get_metric_period function
 */
static int
slurm_state_metric_get_metric_period(void)
{
  /* The slurm_state is propogated all of the time so that
   * programs/modules monitoring it will immediately know when slurm
   * has died and has woken back up.
   */
  return 0;
}

/*
 * slurm_state_metric_get_metric_value
 *
 * slurm_state metric module get_metric_value function
 */
static int
slurm_state_metric_get_metric_value(unsigned int *metric_value_type,
                                    unsigned int *metric_value_len,
                                    void **metric_value)
{
  int rv;

  if (!metric_value_type)
    {
      cerebro_err_debug("%s(%s:%d): metric_value_type null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!metric_value_len)
    {
      cerebro_err_debug("%s(%s:%d): metric_value_len null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!metric_value)
    {
      cerebro_err_debug("%s(%s:%d): metric_value null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  *metric_value_type = CEREBRO_METRIC_VALUE_TYPE_U_INT32;
  *metric_value_len = sizeof(u_int32_t);

  /* Its not the end of the world if the lock fails, just keep on
   * going
   */
  if ((rv = pthread_mutex_lock(&metric_slurm_state_lock)) != 0)
    cerebro_err_debug("%s(%s:%d): pthread_mutex_lock: %s",
                      __FILE__, __FUNCTION__, __LINE__,
                      strerror(rv));

  *metric_value = (void *)&metric_slurm_state;

  if (!rv)
    {
      if ((rv = pthread_mutex_unlock(&metric_slurm_state_lock)) != 0)
        cerebro_err_debug("%s(%s:%d): pthread_mutex_unlock: %s",
                          __FILE__, __FUNCTION__, __LINE__,
                          strerror(rv));
    }

  return 0;
}

/*
 * slurm_state_metric_destroy_metric_value
 *
 * slurm_state metric module destroy_metric_value function
 */
static int
slurm_state_metric_destroy_metric_value(void *metric_value)
{
  return 0;
}

/* 
 * slurm_state_metric_thread
 *
 * Thread that will continually monitor the state of slurm
 */
static void *
slurm_state_metric_thread(void *arg)
{

  while (1)
    {
      struct sockaddr_un addr;
      socklen_t addrlen;
      int slurm_fd, num, rv;
      
      addrlen = sizeof(sizeof(struct sockaddr_un));
      if ((slurm_fd = accept(slurm_state_fd,
                             (struct sockaddr *)&addr,
                             &addrlen)) < 0)
        {
          cerebro_err_debug("%s(%s:%d): accept: %s",
                            __FILE__, __FUNCTION__, __LINE__,
                            strerror(errno));
          if (errno == EINTR)
            continue;
          else
            {
              /* 
               * Make an attempt to set things up again and get
               * this working again.
               */
              slurm_state_fd = _slurm_state_create_and_setup_socket();
              sleep(SLURM_STATE_REINITIALIZE_WAIT_TIME);
            }
        }

      /* Its not the end of the world if the lock fails, just keep on
       * going
       */
      if ((rv = pthread_mutex_lock(&metric_slurm_state_lock)) != 0)
        cerebro_err_debug("%s(%s:%d): pthread_mutex_lock: %s",
                          __FILE__, __FUNCTION__, __LINE__,
                          strerror(rv));

      metric_slurm_state = 1;
      
      if (!rv)
        {
          if ((rv = pthread_mutex_unlock(&metric_slurm_state_lock)) != 0)
            cerebro_err_debug("%s(%s:%d): pthread_mutex_unlock: %s",
                              __FILE__, __FUNCTION__, __LINE__,
                              strerror(rv));
        }
      
      while (1)
        {
          char buffer[CEREBRO_MAX_PACKET_LEN];
          fd_set rfds;

          FD_ZERO(&rfds);
          FD_SET(slurm_fd, &rfds);
          
          if ((num = select(slurm_fd + 1, &rfds, NULL, NULL, NULL)) < 0)
            {
              cerebro_err_debug("%s(%s:%d): select: %s",
                                __FILE__, __FUNCTION__, __LINE__,
                                strerror(errno));
              if (errno == EINTR)
                continue;
              else
                {
                  /* Break and we'll try again */
                  sleep(SLURM_STATE_REINITIALIZE_WAIT_TIME);
                  close(slurm_fd);
                  break;
                }
            }

          if (!num)
            {
              /* Umm, I'm not sure how this would even happen, lets break */
              cerebro_err_debug("%s(%s:%d): select invalid return",
                                __FILE__, __FUNCTION__, __LINE__);
              sleep(SLURM_STATE_REINITIALIZE_WAIT_TIME);
              close(slurm_fd);
              break;
            }

          if (FD_ISSET(slurm_fd, &rfds))
            {
              int n;
              
              if ((n = read(slurm_fd,
                            buffer,
                            CEREBRO_MAX_PACKET_LEN)) < 0)
                {
                  cerebro_err_debug("%s(%s:%d): read: %s",
                                    __FILE__, __FUNCTION__, __LINE__,
                                    strerror(errno));
                  sleep(SLURM_STATE_REINITIALIZE_WAIT_TIME);
                  close(slurm_fd);
                  break;
                }

              /* Normally should be 0, but perhaps somehow something
               * could get read.  We'll assume the connection died if
               * partial data was read.
               */
              if (n > 0 && n < CEREBRO_MAX_PACKET_LEN)
                {
                  cerebro_err_debug("%s(%s:%d): unintended read: %d",
                                    __FILE__, __FUNCTION__, __LINE__,
                                    n);
                  n = 0;
                }

              if (!n)
                {
                  /* Its not the end of the world if the lock fails, just keep on
                   * going
                   */
                  if ((rv = pthread_mutex_lock(&metric_slurm_state_lock)) != 0)
                    cerebro_err_debug("%s(%s:%d): pthread_mutex_lock: %s",
                                      __FILE__, __FUNCTION__, __LINE__,
                                      strerror(rv));

                  metric_slurm_state = 0;

                  if (!rv)
                    {
                      if ((rv = pthread_mutex_unlock(&metric_slurm_state_lock)) != 0)
                        cerebro_err_debug("%s(%s:%d): pthread_mutex_unlock: %s",
                                          __FILE__, __FUNCTION__, __LINE__,
                                          strerror(rv));
                    }
                  
                  close(slurm_fd);
                  break;
                }
            }
          else
            {
              /* Umm, I'm not sure how this would even happen, lets break */
              cerebro_err_debug("%s(%s:%d): select invalid return",
                                __FILE__, __FUNCTION__, __LINE__);
              sleep(SLURM_STATE_REINITIALIZE_WAIT_TIME);
              close(slurm_fd);
              break;
            }
          
        }
    }

  /* NOT REACHED */
  return NULL;
}

/*
 * slurm_state_metric_get_metric_thread
 *
 * slurm_state metric module get_metric_thread function
 */
static Cerebro_metric_thread_pointer
slurm_state_metric_get_metric_thread(void)
{
  return &slurm_state_metric_thread;
}

struct cerebro_metric_module_info metric_module_info =
  {
    SLURM_STATE_METRIC_MODULE_NAME,
    &slurm_state_metric_setup,
    &slurm_state_metric_cleanup,
    &slurm_state_metric_get_metric_name,
    &slurm_state_metric_get_metric_period,
    &slurm_state_metric_get_metric_value,
    &slurm_state_metric_destroy_metric_value,
    &slurm_state_metric_get_metric_thread,
  };
