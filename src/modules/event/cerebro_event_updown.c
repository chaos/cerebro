/*****************************************************************************\
 *  $Id: cerebro_event_updown.c,v 1.1.2.7 2006-11-04 01:35:18 chu11 Exp $
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

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#include <stdarg.h>
#endif /* STDC_HEADERS */
#include <time.h>
#include <errno.h>

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_event_module.h"

#include "debug.h"
#include "hash.h"
#include "list.h"

#define UPDOWN_EVENT_MODULE_NAME     "updown"
#define UPDOWN_EVENT_NAMES           "updown"
#define UPDOWN_EVENT_NAME            "updown"
#define UPDOWN_EVENT_METRIC_NAMES    "boottime"
#define UPDOWN_EVENT_TIMEOUT_LENGTH  60
#define UPDOWN_HASH_SIZE             1024

#define UPDOWN_EVENT_STATE_INIT     -1
#define UPDOWN_EVENT_STATE_DOWN      0
#define UPDOWN_EVENT_STATE_UP        1

/*
 * node_states
 *
 * Maintain node state information
 */
static hash_t node_states = NULL;

/*
 * node_states_nodenames
 *
 * Cache of nodenames used as keys for the node_states
 */
static List node_states_nodenames = NULL;

/*
 * updown_event_setup
 *
 * updown event module setup function.
 */
static int
updown_event_setup(void)
{
  if (!(node_states = hash_create(UPDOWN_HASH_SIZE,
                                  (hash_key_f)hash_key_string,
                                  (hash_cmp_f)strcmp,
                                  (hash_del_f)free)))
    {
      CEREBRO_DBG(("hash_create: %s", strerror(errno)));
      goto cleanup;
    }
  
  if (!(node_states_nodenames = list_create((ListDelF)free)))
    {
      CEREBRO_DBG(("list_create: %s", strerror(errno)));
      goto cleanup;
    }

  return 0;

 cleanup:
  if (node_states)
    {
      hash_destroy(node_states);
      node_states = NULL;
    }
  if (node_states_nodenames)
    {
      list_destroy(node_states_nodenames);
      node_states_nodenames = NULL;
    }
  return -1;
}

/*
 * updown_event_cleanup
 *
 * updown event module cleanup function
 */
static int
updown_event_cleanup(void)
{
  return 0;
}

/*
 * updown_event_event_names
 *
 * updown event module event_names function
 */
static char *
updown_event_event_names(void)
{
  return UPDOWN_EVENT_NAMES;
}

/*
 * updown_event_metric_names
 *
 * updown event module metric_names function
 */
static char *
updown_event_metric_names(void)
{
  return UPDOWN_EVENT_METRIC_NAMES;
}

/*
 * updown_event_timeout_length
 *
 * updown event module timeout_length function
 */
static int
updown_event_timeout_length(void)
{
  return UPDOWN_EVENT_TIMEOUT_LENGTH;
}

/* 
 * _create_entry
 *
 * Create an entry in the node_states hash
 */
static int *
_create_entry(const char *nodename)
{
  char *nodePtr = NULL;
  int *state = NULL;

  if (!(state = (int *)malloc(sizeof(int))))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      goto cleanup;
    }
  
  if (!(nodePtr = (char *)malloc(CEREBRO_MAX_NODENAME_LEN + 1)))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      goto cleanup;
    }

  strncpy(nodePtr, nodename, CEREBRO_MAX_NODENAME_LEN);
  
  if (!list_append(node_states_nodenames, nodePtr))
    {
      CEREBRO_DBG(("list_append: %s", strerror(errno)));
      goto cleanup;
    }

  if (!hash_insert(node_states, nodePtr, state))
    {
      CEREBRO_DBG(("hash_insert: %s", strerror(errno)));
      goto cleanup;
    }

  *state = UPDOWN_EVENT_STATE_INIT;
  return state;

 cleanup:
  free(nodePtr);
  free(state);
  return NULL;
}

/* 
 * _create_event
 *
 * Create an event
 */
static struct cerebro_event *
_create_event(const char *nodename, int state)
{
  struct cerebro_event *event = NULL;

  if (!(event = (struct cerebro_event *)malloc(sizeof(struct cerebro_event))))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      goto cleanup;
    }

  event->version = CEREBRO_EVENT_PROTOCOL_VERSION;
  strncpy(event->nodename, nodename, CEREBRO_MAX_NODENAME_LEN);
  strncpy(event->event_name, UPDOWN_EVENT_NAME, CEREBRO_MAX_EVENT_NAME_LEN);
  event->event_value_type = CEREBRO_METRIC_VALUE_TYPE_INT32;
  event->event_value_len = sizeof(int32_t);
  if (!(event->event_value = malloc(sizeof(int32_t))))
    {
      CEREBRO_DBG(("malloc: %s", strerror(errno)));
      goto cleanup;
    }
  
  *((int *)event->event_value) = state;

  return event;

 cleanup:
  if (event)
    {
      if (event->event_value)
        free(event->event_value);
      free(event);
    }
  return NULL;
}

/*
 * updown_event_node_timeout
 *
 * updown event module node_timeout function
 */
static int
updown_event_node_timeout(const char *nodename,
                          struct cerebro_event **event)
{
  int *state = NULL;
  int rv = 0;

  /* If the node isn't recorded, it's the first notification
   * that it's down.
   */
  if (!(state = hash_find(node_states, nodename)))
    {
      if (!(state = _create_entry(nodename)))
        return -1;
    }

#if 1
  /* XXX - in here for debugging */
  if (*state == UPDOWN_EVENT_STATE_INIT)
    {
      struct cerebro_event *eventPtr = NULL;
      if ((eventPtr = _create_event(nodename, UPDOWN_EVENT_STATE_DOWN)))
        {
          *event = eventPtr;
          rv = 1;
        }
    }
#endif

  if (*state == UPDOWN_EVENT_STATE_UP)
    {
      struct cerebro_event *eventPtr = NULL;
      if ((eventPtr = _create_event(nodename, UPDOWN_EVENT_STATE_DOWN)))
        {
          *event = eventPtr;
          rv = 1;
        }
    }
  
  *state = UPDOWN_EVENT_STATE_DOWN;
  return rv;
}

/*
 * updown_event_metric_update
 *
 * updown event module metric_update function.  Store results the
 * updown cache appropriately.
 */
static int 
updown_event_metric_update(const char *nodename,
                           const char *metric_name,
                           unsigned int metric_value_type,
                           unsigned int metric_value_len,
                           void *metric_value,
                           struct cerebro_event **event)
{
  int *state = NULL;
  int rv = 0;

  /* If the node isn't recorded, it's the first notification
   * that it's down.
   */
  if (!(state = hash_find(node_states, nodename)))
    {
      if (!(state = _create_entry(nodename)))
        return -1;
    }

  if (*state == UPDOWN_EVENT_STATE_DOWN)
    {
      struct cerebro_event *eventPtr = NULL;
      if ((eventPtr = _create_event(nodename, UPDOWN_EVENT_STATE_UP)))
        {
          *event = eventPtr;
          rv = 1;
        }
    }
  
  *state = UPDOWN_EVENT_STATE_UP;
  return rv;
}

/*
 * updown_event_destroy
 *
 * updown event module destroy function
 */
static void
updown_event_destroy(struct cerebro_event *event)
{
  if (!event)
    {
      CEREBRO_DBG(("invalid parameters"));
      return;
    }

  free(event->event_value);
  free(event);
}

#if WITH_STATIC_MODULES
struct cerebro_event_module_info updown_event_module_info =
#else  /* !WITH_STATIC_MODULES */
struct cerebro_event_module_info event_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    UPDOWN_EVENT_MODULE_NAME,
    &updown_event_setup,
    &updown_event_cleanup,
    &updown_event_event_names,
    &updown_event_metric_names,
    &updown_event_timeout_length,
    &updown_event_node_timeout,
    &updown_event_metric_update,
    &updown_event_destroy,
  };
