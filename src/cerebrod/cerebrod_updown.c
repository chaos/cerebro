/*****************************************************************************\
 *  $Id: cerebrod_updown.c,v 1.1 2005-03-15 23:14:39 achu Exp $
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
#include "error.h"
#include "fd.h"
#include "wrappers.h"

#define CEREBROD_UPDOWN_BACKLOG                  10

#define CEREBROD_UPDOWN_NODE_DATA_SIZE_DEFAULT   1024
#define CEREBROD_UPDOWN_NODE_DATA_SIZE_INCREMENT 1024
#define CEREBROD_UPDOWN_REHASH_LIMIT             (updown_node_data_size*2)

extern struct cerebrod_config conf;
#ifndef NDEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* NDEBUG */

int cerebrod_updown_initialization_complete = 0;

int updown_fd;

List updown_node_data = NULL;
hash_t updown_node_data_hash = NULL;

int updown_node_data_size;
int updown_node_data_numnodes;
pthread_mutex_t updown_node_data_list_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t updown_node_data_hash_lock = PTHREAD_MUTEX_INITIALIZER;

static int
_cerebrod_updown_create_and_setup_socket(void)
{
  struct sockaddr_in server_addr;
  int temp_fd;

  if ((temp_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
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

static int 
_updown_node_data_strcmp(void *x, void *y)
{
  assert(x && y);

  return strcmp(((struct cerebrod_updown_node_data *)x)->node,
                ((struct cerebrod_updown_node_data *)y)->node);
}

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
      updown_node_data_size = numnodes;
      updown_node_data_numnodes = numnodes;
    }
  else
    {
      updown_node_data_size = CEREBROD_UPDOWN_NODE_DATA_SIZE_DEFAULT;
      updown_node_data_numnodes = 0;
    }

  updown_node_data = List_create((ListDelF)_Free);
  updown_node_data_hash = Hash_create(updown_node_data_size,
                                      (hash_key_f)hash_key_string,
                                      (hash_cmp_f)strcmp,
                                      (hash_del_f)_Free);

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
          Hash_insert(updown_node_data_hash, nodes[i], ud);

          list_sort(updown_node_data, (ListCmpF)_updown_node_data_strcmp);
        }

      Free(nodes);
    }

  cerebrod_updown_initialization_complete++;
}

void *
cerebrod_updown_service_connection(void *arg)
{
  int client_fd;

  client_fd = *((int *)arg);

  Close(client_fd);
  exit(0);
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
