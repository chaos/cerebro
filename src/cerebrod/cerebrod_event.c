/*****************************************************************************\
 *  $Id: cerebrod_event.c,v 1.1.2.1 2006-10-31 06:26:36 chu11 Exp $
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
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if !WITH_CEREBROD_SPEAKER_ONLY

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <errno.h>
#include <assert.h>

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_event_protocol.h"

#include "cerebrod_config.h"
#include "cerebrod_event.h"
#include "cerebrod_util.h"

#include "event_module.h"
#include "debug.h"
#include "fd.h"
#include "hash.h"
#include "list.h"
#include "network_util.h"
#include "wrappers.h"

extern struct cerebrod_config conf;
#if CEREBRO_DEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* CEREBRO_DEBUG */

#define CEREBROD_EVENT_SERVER_BACKLOG 10

/* 
 * event_manager_init
 * event_manager_init_cond
 * event_manager_init_lock
 * event_server_init
 * event_server_init_cond
 * event_server_init_lock
 *
 * variables for synchronizing initialization between different pthreads
 * and signaling when it is complete
 */
int event_manager_init = 0;
pthread_cond_t event_manager_init_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t event_manager_init_lock = PTHREAD_MUTEX_INITIALIZER;
int event_server_init = 0;
pthread_cond_t event_server_init_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t event_server_init_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * event_handle
 *
 * Handle for event modules;
 */
event_modules_t event_handle = NULL;

/*
 * event_index
 * event_index_count
 *
 * hash indexes to quickly determine what metrics are needed
 * by which modules.
 */
hash_t event_index = NULL;
int event_index_count = 0;

/* 
 * event_names
 *
 * List of event names supported by the modules
 */
List event_names = NULL;

/*
 * _cerebrod_event_module_destroy
 *
 * Destroy a event_module struct.
 */
static void
_cerebrod_event_module_destroy(void *data)
{
  struct cerebrod_event_module *event_module;

  assert(data);

  event_module = (struct cerebrod_event_module *)data;
  Free(event_module->metric_names);
  Free(event_module);
}

/* 
 * _setup_event_modules
 * 
 * Setup event modules.  Under almost any circumstance, don't return
 * a -1 error, cerebro can go on without loading event modules.
 *
 * Return 1 if modules are loaded, 0 if not, -1 on error
 */
static int
_setup_event_modules(void)
{
  int i, event_module_count, event_index_len;
  List event_list = NULL;
  char *eventnamePtr;

  if (!(event_handle = event_modules_load()))
    {
      CEREBRO_DBG(("event_modules_load"));
      goto cleanup;
    }

  if ((event_module_count = event_modules_count(event_handle)) < 0)
    {
      CEREBRO_DBG(("event_modules_count"));
      goto cleanup;
    }
  
  if (!event_module_count)
    {
#if CEREBRO_DEBUG
      if (conf.debug && conf.event_server_debug)
        {
          Pthread_mutex_lock(&debug_output_mutex);
          fprintf(stderr, "**************************************\n");
          fprintf(stderr, "* No Event Modules Found\n");
          fprintf(stderr, "**************************************\n");
          Pthread_mutex_unlock(&debug_output_mutex);
        }
#endif /* CEREBRO_DEBUG */
      goto cleanup;
    }
  
  /* Each event module may want multiple metrics and/or offer multiple
   * event names.  We'll assume there will never be more than 2 per
   * event module, and that will be enough to avoid all hash
   * collisions.
   */
  event_index_len = event_module_count * 2;

  event_index = Hash_create(event_index_len, 
                            (hash_key_f)hash_key_string, 
                            (hash_cmp_f)strcmp, 
                            (hash_del_f)list_destroy);

  event_names = List_create((ListDelF)_Free);

  for (i = 0; i < event_module_count; i++)
    {
      struct cerebrod_event_module *event_module;
      char *module_name, *module_metric_names, *module_event_names;

      module_name = event_module_name(event_handle, i);

#if CEREBRO_DEBUG
      if (conf.debug && conf.listen_debug)
        {
          Pthread_mutex_lock(&debug_output_mutex);
          fprintf(stderr, "**************************************\n");
          fprintf(stderr, "* Setup Event Module: %s\n", module_name);
          fprintf(stderr, "**************************************\n");
          Pthread_mutex_unlock(&debug_output_mutex);
        }
#endif /* CEREBRO_DEBUG */

      if (event_module_setup(event_handle, i) < 0)
        {
          CEREBRO_DBG(("event_module_setup failed: %s", module_name));
          continue;
        }

      if (!(module_metric_names = event_module_metric_names(event_handle, i)) < 0)
        {
          CEREBRO_DBG(("event_module_metric_names failed: %s", module_name));
          event_module_cleanup(event_handle, i);
          continue;
        }

      if (!(module_event_names = event_module_event_names(event_handle, i)) < 0)
        {
          CEREBRO_DBG(("event_module_event_names failed: %s", module_name));
          event_module_cleanup(event_handle, i);
          continue;
        }

      event_module = Malloc(sizeof(struct cerebrod_event_module));
      event_module->metric_names = Strdup(module_metric_names);
      event_module->index = i;
      Pthread_mutex_init(&(event_module->event_lock), NULL);

      if (!strchr(event_module->metric_names, ','))
        {
	  if (!(event_list = Hash_find(event_index, event_module->metric_names)))
	    {
	      event_list = List_create((ListDelF)_cerebrod_event_module_destroy);
	      List_append(event_list, event_module);
	      Hash_insert(event_index, event_module->metric_names, event_list);
	      event_index_count++;
	    }
	  else
	    List_append(event_list, event_module);
        }
      else
        {
          char *metric, *metricbuf;
          
          /* This eventing module supports multiple metrics, must
	   * parse out each one
	   */
          
          metric = strtok_r(event_module->metric_names, ",", &metricbuf);
          while (metric)
            {
	      if (!(event_list = Hash_find(event_index, metric)))
		{
		  event_list = List_create((ListDelF)_cerebrod_event_module_destroy);
		  List_append(event_list, event_module);
		  Hash_insert(event_index, metric, event_list);
		  event_index_count++;
		}
	      else
		List_append(event_list, event_module);
              
              metric = strtok_r(NULL, ",", &metricbuf);
            }
        }

      if (!strchr(module_event_names, ','))
        {
          if (!list_find_first(event_names, 
                               (ListFindF)strcmp, 
                               module_event_names))
            {
              eventnamePtr = Strdup(module_event_names);
              List_append(event_names, eventnamePtr);
            }
        }
      else
        {
          char *eventname, *eventbuf;
          
          /* This event module supports multiple events, must parse
	   * out each one
	   */
          
          eventname = strtok_r(module_event_names, ",", &eventbuf);
          while (eventname)
            {
              if (!list_find_first(event_names, 
                                   (ListFindF)strcmp, 
                                   eventname))
                {
                  eventnamePtr = Strdup(eventname);
                  List_append(event_names, eventnamePtr);
                }
              eventname = strtok_r(NULL, ",", &eventbuf);
            }
        }
    }
  
  if (!event_index_count)
    goto cleanup;
  
  return 1;
  
 cleanup:
  if (event_handle)
    {
      event_modules_unload(event_handle);
      event_handle = NULL;
    }
  if (event_index)
    {
      Hash_destroy(event_index);
      event_index = NULL;
    }
  if (event_names)
    {
      List_destroy(event_names);
      event_names = NULL;
    }
  event_index_count = 0;
  return 0;
}

/*
 * _cerebrod_event_manager_initialize
 *
 * perform event initialization
 */
static void
_cerebrod_event_manager_initialize(void)
{
  Pthread_mutex_lock(&event_manager_init_lock);
  if (event_manager_init)
    goto out;

  if (_setup_event_modules() < 0)
    CEREBRO_EXIT(("_setup_event_modules"));

  event_manager_init++;
  Pthread_cond_signal(&event_manager_init_cond);
 out:
  Pthread_mutex_unlock(&event_manager_init_lock);
}

void *
cerebrod_event_manager(void *arg)
{
  _cerebrod_event_manager_initialize();

  for (;;)
    {
    }

  return NULL;			/* NOT REACHED */
}

/*
 * _event_server_initialize
 *
 * perform metric server initialization
 */
static void
_event_server_initialize(void)
{
  Pthread_mutex_lock(&event_server_init_lock);
  if (event_server_init)
    goto out;

  Signal(SIGPIPE, SIG_IGN);

  event_server_init++;
  Pthread_cond_signal(&event_server_init_cond);
 out:
  Pthread_mutex_unlock(&event_server_init_lock);
}

/*
 * _event_server_setup_socket
 *
 * Create and setup the server socket.  Do not use wrappers in this
 * function.  We want to give the server additional chances to
 * "survive" an error condition.
 *
 * Returns file descriptor on success, -1 on error
 *
 * Note: num parameter unused in this function, here only to match function prototype
 */
static int
_event_server_setup_socket(int num)
{
  struct sockaddr_in addr;
  int fd, optval = 1;

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      CEREBRO_DBG(("socket: %s", strerror(errno)));
      goto cleanup;
    }

  /* For quick start/restart */
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) < 0)
    {
      CEREBRO_DBG(("setsockopt: %s", strerror(errno)));
      goto cleanup;
    }

  memset(&addr, '\0', sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(conf.event_server_port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
    {
      CEREBRO_DBG(("bind: %s", strerror(errno)));
      goto cleanup;
    }

  if (listen(fd, CEREBROD_EVENT_SERVER_BACKLOG) < 0)
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
cerebrod_event_server(void *arg)
{
  int server_fd;

  _event_server_initialize();

  if ((server_fd = _event_server_setup_socket(0)) < 0)
    CEREBRO_EXIT(("event server fd setup failed"));

  for (;;)
    {
      int fd, client_addr_len;
      struct sockaddr_in client_addr;
      
      client_addr_len = sizeof(struct sockaddr_in);
      if ((fd = accept(server_fd,
                       (struct sockaddr *)&client_addr,
                       &client_addr_len)) < 0)
        server_fd = cerebrod_reinit_socket(server_fd,
                                           0,
                                           _event_server_setup_socket,
                                           "event_server: accept");
      if (fd < 0)
        continue;
      
    }

  return NULL;			/* NOT REACHED */
}

#endif /* !WITH_CEREBROD_SPEAKER_ONLY */
