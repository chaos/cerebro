/*****************************************************************************\
 *  $Id: cerebrod_updown.c,v 1.16 2005-03-25 19:44:05 achu Exp $
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
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "cerebrod_updown.h"
#include "cerebrod_clusterlist.h"
#include "cerebrod_config.h"
#include "cerebrod_util.h"
#include "cerebrod.h"
#include "error.h"
#include "fd.h"
#include "wrappers.h"

#define CEREBROD_UPDOWN_BACKLOG                        10

#define CEREBROD_UPDOWN_NODE_DATA_INDEX_SIZE_DEFAULT   1024
#define CEREBROD_UPDOWN_NODE_DATA_INDEX_SIZE_INCREMENT 1024
#define CEREBROD_UPDOWN_REHASH_LIMIT                   (updown_node_data_index_size*2)

extern struct cerebrod_config conf;
#ifndef NDEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* NDEBUG */

/*
 * cerebrod_updown_initialization_complete
 * cerebrod_updown_initialization_complete_cond
 * cerebrod_updown_initialization_complete_lock
 *
 * variables for synchronizing initialization between different pthreads
 * and signaling when it is complete
 */
int cerebrod_updown_initialization_complete = 0;
pthread_cond_t cerebrod_updown_initialization_complete_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t cerebrod_updown_initialization_complete_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * updown_fd
 *
 * updown server file descriptor
 */
int updown_fd;

/* 
 * updown_node_data
 *
 * contains list of nodes with last received times
 */
List updown_node_data = NULL;

/*
 * updown_node_data_index
 * updown_node_data_index_numnodes
 * updown_node_data_index_size
 *
 * hash index into updown_node_data list for faster access, number of
 * currently indexed entries, and index size
 */
hash_t updown_node_data_index = NULL;
int updown_node_data_index_numnodes;
int updown_node_data_index_size;

/*  
 * updown_node_data_lock
 *
 * lock to protect pthread access to both updown node data list and
 * index
 */
pthread_mutex_t updown_node_data_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * _cerebrod_updown_create_and_setup_socket
 *
 * Create and setup the updown server socket.  Do not use wrappers in
 * this function.  We want to give the server additional chances to
 * "survive" an error condition.
 *
 * Returns file descriptor on success, -1 on error
 */
static int
_cerebrod_updown_create_and_setup_socket(void)
{
  struct sockaddr_in server_addr;
  int temp_fd;

  if ((temp_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      err_debug("_cerebrod_updownd_create_and_setup_socket: socket: %s",
		strerror(errno));
      return -1;
    }

  /* Configuration checks ensure destination ip is on this machine if
   * it is a non-multicast address.
   */
  memset(&server_addr, '\0', sizeof(struct sockaddr_in));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(conf.updown_server_port);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(temp_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) < 0)
    {
      err_debug("_cerebrod_updownd_create_and_setup_socket: bind: %s",
		strerror(errno));
      return -1;
    }

  if (listen(temp_fd, CEREBROD_UPDOWN_BACKLOG) < 0)
    {
      err_debug("_cerebrod_updownd_create_and_setup_socket: listen: %s",
		strerror(errno));
      return -1;
    }

  return temp_fd;
}

/* 
 * _updown_node_data_strcmp
 *
 * callback function for list_sort to sort updown node names
 */
static int 
_updown_node_data_strcmp(void *x, void *y)
{
  assert(x && y);

  return strcmp(((struct cerebrod_updown_node_data *)x)->node,
                ((struct cerebrod_updown_node_data *)y)->node);
}

/*
 * _cerebrod_updown_initialize
 *
 * perform updown server initialization
 */
static void
_cerebrod_updown_initialize(void)
{
  int numnodes;

  if (cerebrod_updown_initialization_complete)
    return;

  if ((updown_fd = _cerebrod_updown_create_and_setup_socket()) < 0)
    err_exit("_cerebrod_updown_initialize: updown_fd setup failed");

  if ((numnodes = cerebrod_clusterlist_numnodes()) < 0)
    err_exit("_cerebrod_updown_initialize: cerebrod_clusterlist_numnodes: %s",
             strerror(errno));

  if (numnodes > 0)
    {
      updown_node_data_index_numnodes = numnodes;
      updown_node_data_index_size = numnodes;
    }
  else
    {
      updown_node_data_index_numnodes = 0;
      updown_node_data_index_size = CEREBROD_UPDOWN_NODE_DATA_INDEX_SIZE_DEFAULT;
    }

  updown_node_data = List_create((ListDelF)_Free);
  updown_node_data_index = Hash_create(updown_node_data_index_size,
                                       (hash_key_f)hash_key_string,
                                       (hash_cmp_f)strcmp,
                                       (hash_del_f)_Free);
  
  /* If the clusterlist module contains nodes, retrieve all of these
   * nodes and put them into the updown_node_data list.  All updates
   * will simply involve updating the last_received time, rather than
   * involve insertion.
   */
  if (numnodes > 0)
    {
      int i;
      char **nodes;

      nodes = (char **)Malloc(sizeof(char *) * numnodes);

      if (cerebrod_clusterlist_get_all_nodes(nodes, numnodes) < 0)
        err_exit("_cerebrod_updown_initialize: cerebrod_clusterlist_get_all_nodes: %s",
                 strerror(errno));

      for (i = 0; i < numnodes; i++)
        {
          struct cerebrod_updown_node_data *ud;

          ud = Malloc(sizeof(struct cerebrod_updown_node_data));

          ud->node = Strdup(nodes[i]);
          ud->discovered = 0;
          ud->last_received = 0;
          Pthread_mutex_init(&(ud->updown_node_data_lock), NULL);

          List_append(updown_node_data, ud);
          Hash_insert(updown_node_data_index, nodes[i], ud);

          list_sort(updown_node_data, (ListCmpF)_updown_node_data_strcmp);
        }

      Free(nodes);
    }

  Pthread_mutex_lock(&cerebrod_updown_initialization_complete_lock);
  cerebrod_updown_initialization_complete++;
  Pthread_cond_signal(&cerebrod_updown_initialization_complete_cond);
  Pthread_mutex_unlock(&cerebrod_updown_initialization_complete_lock);
}

/* 
 * cerebrod_updown_service_connection
 *
 * Thread to service a connection from a client to retrieve updown node data
 *
 * Passed int * pointer to client TCP socket file descriptor
 *
 * Executed in detached state, no return value.
 */
static void *
cerebrod_updown_service_connection(void *arg)
{
  int client_fd;

  client_fd = *((int *)arg);

  Free(arg);
  Close(client_fd);
  return NULL;
}

void *
cerebrod_updown(void *arg)
{
  _cerebrod_updown_initialize();

  for (;;)
    {
      pthread_t thread;
      pthread_attr_t attr;
      int client_fd, client_addr_len, *arg;
      struct sockaddr_in client_addr;

      client_addr_len = sizeof(struct sockaddr_in);
      if ((client_fd = accept(updown_fd, 
                              (struct sockaddr *)&client_addr, 
                              &client_addr_len)) < 0)
	{
          /* For errnos EINVAL, EBADF, ENODEV, assume the device has
           * been temporarily brought down then back up.  For example,
           * this can occur if the administrator runs
           * '/etc/init.d/network restart'.  We just need to re-setup
           * the socket.
           *
           * If fd < 0, the network device just isn't back up yet from
           * the previous time we got an errno EINVAL, EBADF, or
           * ENODEV.
           */
          if (errno == EINVAL
	      || errno == EBADF
	      || errno == ENODEV
	      || updown_fd < 0)
            {
              if (!(updown_fd < 0))
		close(updown_fd);	/* no-wrapper, make best effort */

              if ((updown_fd = _cerebrod_updown_create_and_setup_socket()) < 0)
		{
		  err_debug("cerebrod_updown: error re-initializing socket");

		  /* Wait a bit, so we don't spin */
		  sleep(CEREBROD_UPDOWN_REINITIALIZE_WAIT);
		}
              else
                err_debug("cerebrod_updown: success re-initializing socket");
            }
          else if (errno == EINTR)
            err_debug("cerebrod_updown: recvfrom: %s", strerror(errno));
          else
            err_exit("cerebrod_updown: recvfrom: %s", strerror(errno));
	}

      if (client_fd < 0)
	continue;

      /* Pass off connection to thread */
      Pthread_attr_init(&attr);
      Pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      arg = Malloc(sizeof(int));
      *arg = client_fd;
      Pthread_create(&thread, 
                     &attr, 
                     cerebrod_updown_service_connection, 
                     (void *)arg);
      Pthread_attr_destroy(&attr);

    }

  return NULL;			/* NOT REACHED */
}

/*  
 * _cerebrod_updown_output_insert
 *
 * Output debugging info about a recently inserted node
 */
static void
_cerebrod_updown_output_insert(struct cerebrod_updown_node_data *ud)
{
#ifndef NDEBUG
  assert(ud);
 
  if (conf.debug && conf.updown_server_debug)
    {
      Pthread_mutex_lock(&debug_output_mutex);
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Updown Server Insertion: Node=%s\n", ud->node);
      fprintf(stderr, "**************************************\n");
      Pthread_mutex_unlock(&debug_output_mutex);
    }
#endif /* NDEBUG */
}

/*  
 * _cerebrod_updown_output_update
 *
 * Output debugging info about a recently updated node
 */
static void
_cerebrod_updown_output_update(struct cerebrod_updown_node_data *ud)
{
#ifndef NDEBUG
  assert(ud);
 
  if (conf.debug && conf.updown_server_debug)
    {
      struct tm tm;
      char strbuf[CEREBROD_STRING_BUFLEN];
 
      Localtime_r((time_t *)&(ud->last_received), &tm);
      strftime(strbuf, CEREBROD_STRING_BUFLEN, "%H:%M:%S", &tm);
 
      Pthread_mutex_lock(&debug_output_mutex);
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Updown Server Update: Node=%s Last_Received=%s\n", 
              ud->node, strbuf);
      fprintf(stderr, "**************************************\n");
      Pthread_mutex_unlock(&debug_output_mutex);
    }
#endif /* NDEBUG */
}

/*
 * _cerebrod_updown_dump_updown_node_data_item
 *
 * callback function from hash_for_each to dump updown node data
 */
#ifndef NDEBUG
static int
_cerebrod_updown_dump_updown_node_data_item(void *x, void *arg)
{
  struct cerebrod_updown_node_data *ud;
 
  assert(x);
 
  ud = (struct cerebrod_updown_node_data *)x;
 
  Pthread_mutex_lock(&(ud->updown_node_data_lock));
  fprintf(stderr, "* %s: discovered=%d last_received=%u\n",
          ud->node, ud->discovered, ud->last_received);
  Pthread_mutex_unlock(&(ud->updown_node_data_lock));
 
  return 1;
}
#endif /* NDEBUG */

/*
 * _cerebrod_updown_dump_updown_node_data_list
 *
 * Dump contents of updown node data list
 */
static void
_cerebrod_updown_dump_updown_node_data_list(void)
{
#ifndef NDEBUG
  if (conf.debug && conf.updown_server_debug)
    {
      int rv;
 
      Pthread_mutex_lock(&updown_node_data_lock);
      Pthread_mutex_lock(&debug_output_mutex);
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Updown List State\n");
      fprintf(stderr, "* -----------------------\n");
      fprintf(stderr, "* Listed Nodes: %d\n", updown_node_data_index_numnodes);
      fprintf(stderr, "* -----------------------\n");
      if (updown_node_data_index_numnodes > 0)
        {
          rv = List_for_each(updown_node_data,
                             _cerebrod_updown_dump_updown_node_data_item,
                             NULL);
          if (rv != updown_node_data_index_numnodes)
            err_exit("_cerebrod_updown_dump_updown_node_data: "
                     "invalid dump count: rv=%d numnodes=%d",
                     rv, updown_node_data_index_numnodes);
        }
      else
        err_debug("_cerebrod_updown_dump_node_data: called with empty list");
      fprintf(stderr, "**************************************\n");
      Pthread_mutex_unlock(&debug_output_mutex);
      Pthread_mutex_unlock(&updown_node_data_lock);
    }
#endif /* NDEBUG */
}

void 
cerebrod_updown_update_data(char *node, u_int32_t last_received)
{
  struct cerebrod_updown_node_data *ud;

  if (!cerebrod_updown_initialization_complete)
    err_exit("cerebrod_updown_update_data: initialization not complete");

  Pthread_mutex_lock(&updown_node_data_lock);
  if (!(ud = Hash_find(updown_node_data_index, node)))
    {
      char *key;

      ud = Malloc(sizeof(struct cerebrod_updown_node_data));

      key = Strdup(node);

      ud->node = Strdup(node);
      Pthread_mutex_init(&(ud->updown_node_data_lock), NULL);

      /* Re-hash if our hash is getting too small */
      if ((updown_node_data_index_numnodes + 1) > CEREBROD_UPDOWN_REHASH_LIMIT)
	cerebrod_rehash(&updown_node_data_index,
			&updown_node_data_index_size,
			CEREBROD_UPDOWN_NODE_DATA_INDEX_SIZE_INCREMENT,
			updown_node_data_index_numnodes,
			&updown_node_data_lock);

      List_append(updown_node_data, ud);
      Hash_insert(updown_node_data_index, key, ud);
      updown_node_data_index_numnodes++;

      _cerebrod_updown_output_insert(ud);
    }
  Pthread_mutex_unlock(&updown_node_data_lock);
  
  Pthread_mutex_lock(&(ud->updown_node_data_lock));
  if (last_received >= ud->last_received)
    {
      ud->discovered = 1;
      ud->last_received = last_received;

      _cerebrod_updown_output_update(ud);
    }
  Pthread_mutex_unlock(&(ud->updown_node_data_lock));

  _cerebrod_updown_dump_updown_node_data_list();
}
