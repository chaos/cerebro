/*****************************************************************************\
 *  $Id: cerebrod_updown.c,v 1.35 2005-04-27 18:11:35 achu Exp $
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

#include "cerebro_defs.h"
#include "cerebro_error.h"
#include "cerebro_marshalling.h"
#include "cerebro_updown_protocol.h"

#include "cerebrod_updown.h"
#include "cerebrod_clusterlist.h"
#include "cerebrod_config.h"
#include "cerebrod_util.h"
#include "cerebrod.h"
#include "fd.h"
#include "list.h"
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
      cerebro_err_debug("%s(%s:%d): socket: %s",
                        __FILE__, __FUNCTION__, __LINE__,
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

  if (bind(temp_fd, 
	   (struct sockaddr *)&server_addr, 
	   sizeof(struct sockaddr_in)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): bind: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      return -1;
    }

  if (listen(temp_fd, CEREBROD_UPDOWN_BACKLOG) < 0)
    {
      cerebro_err_debug("%s(%s:%d): listen: %s",
                        __FILE__, __FUNCTION__, __LINE__,
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

  return strcmp(((struct cerebrod_updown_node_data *)x)->nodename,
                ((struct cerebrod_updown_node_data *)y)->nodename);
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
    cerebro_err_exit("%s(%s:%d): updown_fd setup failed",
                     __FILE__, __FUNCTION__, __LINE__);

  if ((numnodes = cerebrod_clusterlist_numnodes()) < 0)
    cerebro_err_exit("%s(%s:%d): cerebrod_clusterlist_numnodes",
                     __FILE__, __FUNCTION__, __LINE__);

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
        cerebro_err_exit("%s(%s:%d): cerebrod_clusterlist_get_all_nodes",
                         __FILE__, __FUNCTION__, __LINE__);

      for (i = 0; i < numnodes; i++)
        {
          struct cerebrod_updown_node_data *ud;

          ud = Malloc(sizeof(struct cerebrod_updown_node_data));

          ud->nodename = Strdup(nodes[i]);
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
 * _cerebrod_updown_response_marshall
 *
 * marshall contents of a updown response packet buffer
 *
 * Returns length written to buffer on success, -1 on error
 */
static int
_cerebrod_updown_response_marshall(struct cerebro_updown_response *res,
				   char *buffer, 
                                   int bufferlen)
{
  int ret, c = 0;
 
  assert(res && buffer && bufferlen > 0);
  assert(bufferlen >= CEREBRO_UPDOWN_RESPONSE_LEN);

  memset(buffer, '\0', bufferlen);

  if ((ret = cerebro_marshall_int32(res->version, 
				    buffer + c, 
				    bufferlen - c)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): cerebro_marshall_int32: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      return -1;
    }
  c += ret;
 
  if ((ret = cerebro_marshall_uint32(res->updown_err_code, 
				     buffer + c, 
				     bufferlen - c)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): cerebro_marshall_uint32: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      return -1;
    }
  c += ret;

  if ((ret = cerebro_marshall_uint8(res->end_of_responses, 
				    buffer + c, 
				    bufferlen - c)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): cerebro_marshall_uint8: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      return -1;
    }
  c += ret;

  if ((ret = cerebro_marshall_buffer(res->nodename,
                                     sizeof(res->nodename),
                                     buffer + c,
                                     bufferlen - c)) < 0)
    cerebro_err_exit("%s(%s:%d): cerebro_marshall_buffer: %s",
                     __FILE__, __FUNCTION__, __LINE__,
                     strerror(errno));
  c += ret;


  if ((ret = cerebro_marshall_uint8(res->updown_state, 
				    buffer + c, 
				    bufferlen - c)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): cerebro_marshall_uint8: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      return -1;
    }
  c += ret;

  return c;
}

/*
 * _cerebrod_updown_request_unmarshall
 *
 * unmarshall contents of a updown request packet buffer
 *
 * Returns 0 on success, -1 on error
 */
static int
_cerebrod_updown_request_unmarshall(struct cerebro_updown_request *req,
				    char *buffer, 
                                    int bufferlen)
{
  int ret, c = 0;
  int invalid_size = 0;

  assert(req && buffer && bufferlen >= 0);
 
  if (bufferlen != CEREBRO_UPDOWN_REQUEST_LEN)
    {
      cerebro_err_debug("%s(%s:%d): received buffer length "
                        "unexpected size: expect %d, bufferlen %d", 
                        CEREBRO_UPDOWN_REQUEST_LEN,
                        __FILE__, __FUNCTION__, __LINE__,
                        bufferlen);
      invalid_size++;
    }
  
  if (invalid_size && bufferlen < sizeof(req->version))
    return -1;

  if ((ret = cerebro_unmarshall_int32(&(req->version), 
				      buffer + c, 
				      bufferlen - c)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): cerebro_unmarshall_int32: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      return -1;
    }
  c += ret;

  if (invalid_size)
    {
      /* Invalid version to be handled by later code */
      if (req->version != CEREBRO_UPDOWN_PROTOCOL_VERSION)
        return 0;
      else
        return -1;
    }
 
  if ((ret = cerebro_unmarshall_uint32(&(req->updown_request), 
				       buffer + c, 
				       bufferlen - c)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): cerebro_unmarshall_uint32: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      return -1;
    }
  c += ret;
 
  if ((ret = cerebro_unmarshall_uint32(&(req->timeout_len), 
				       buffer + c, 
				       bufferlen - c)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): cerebro_unmarshall_uint32: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      return -1;
    }
  c += ret;
   
  return 0;
}

/*
 * cerebrod_updown_receive_request
 *
 * Receive updown server request
 * 
 * Return req packet and 0 on success, -1 on error
 */
static int
_cerebrod_updown_receive_request(int client_fd,	
				 struct cerebro_updown_request *req)
{
  int rv, bytes_read = 0;
  char buffer[CEREBRO_PACKET_BUFLEN];

  assert(client_fd >= 0);
  assert(req);

  memset(buffer, '\0', CEREBRO_PACKET_BUFLEN);

  /* Wait for request from client */
  while (bytes_read < CEREBRO_UPDOWN_REQUEST_LEN)
    {
      fd_set rfds;
      struct timeval tv;
      tv.tv_sec = CEREBRO_UPDOWN_PROTOCOL_TIMEOUT_LEN;
      tv.tv_usec = 0;
      
      FD_ZERO(&rfds);
      FD_SET(client_fd, &rfds);
  
      if ((rv = select(client_fd + 1, &rfds, NULL, NULL, &tv)) < 0)
	{
	  cerebro_err_debug("%s(%s:%d): select: %s", 
                            __FILE__, __FUNCTION__, __LINE__,
                            strerror(errno));
	  goto cleanup;
	}

      if (!rv)
	{
	  /* Timed out.  If atleast some bytes were read, unmarshall
	   * the received bytes.  Its possible we are expecting more
	   * bytes than the client is sending, perhaps because we are
	   * using a different protocol version.  This will allow the
	   * server to return a invalid version number back to the
	   * user.
	   */
	  if (!bytes_read)
	    goto cleanup;
	  else
	    goto unmarshall_received;
	}
      
      if (FD_ISSET(client_fd, &rfds))
	{
	  int n;

	  if ((n = fd_read_n(client_fd, 
			     buffer + bytes_read, 
			     CEREBRO_UPDOWN_REQUEST_LEN - bytes_read)) < 0)
	    {
	      cerebro_err_debug("%s(%s:%d): fd_read_n: %s", 
                                __FILE__, __FUNCTION__, __LINE__,
                                strerror(errno));
	      goto cleanup;
	    }

	  if (!n)
	    /* Pipe closed */
	    goto cleanup;

	  bytes_read += n;
	}
      else
	{
	  cerebro_err_debug("%s(%s:%d): rv != 0 but fd not set",
                            __FILE__, __FUNCTION__, __LINE__);
	  goto cleanup;
	}
    }

 unmarshall_received:
  if (_cerebrod_updown_request_unmarshall(req, buffer, bytes_read) < 0)
    goto cleanup;

  return 0;

 cleanup:
  return -1;
}

/*  
 * _cerebrod_updown_request_dump
 *
 * dump contents of an updown request
 */
static void
_cerebrod_updown_request_dump(struct cerebro_updown_request *req)
{
#ifndef NDEBUG
  assert(req);

  if (conf.debug && conf.updown_server_debug)
    {
      Pthread_mutex_lock(&debug_output_mutex);
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Updown Request Received:\n");
      fprintf(stderr, "* ------------------------\n");
      fprintf(stderr, "* Version: %d\n", req->version);
      fprintf(stderr, "* Updown_Request: %d\n", req->updown_request);
      fprintf(stderr, "* Timeout_len: %d\n", req->timeout_len);
      fprintf(stderr, "**************************************\n");
      Pthread_mutex_unlock(&debug_output_mutex);
    }
#endif /* NDEBUG */
}

/*
 * _cerebrod_updown_send_response
 *
 * send a response packet to the client
 *
 * Return 0 on success, -1 on error
 */
static int
_cerebrod_updown_send_response(int client_fd, 
			       struct cerebro_updown_response *res)
{
  char buffer[CEREBRO_PACKET_BUFLEN];
  int res_len;

  assert(client_fd >= 0);
  assert(res);

  if ((res_len = _cerebrod_updown_response_marshall(res, 
						    buffer, 
						    CEREBRO_PACKET_BUFLEN)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebrod_updown_response_marshall: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      return -1;
    }

  if (fd_write_n(client_fd, buffer, res_len) < 0)
    {
      cerebro_err_debug("%s(%s:%d): fd_write_n: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      return -1;
    }

  return 0;
}


/* 
 * _cerebrod_updown_respond_with_error
 *
 * respond to the updown_request with an error
 *
 * Return 0 on success, -1 on error
 */
static int
_cerebrod_updown_respond_with_error(int client_fd, unsigned int updown_err_code)
{
  struct cerebro_updown_response res;
  
  assert(client_fd >= 0);
  assert(updown_err_code == CEREBRO_UPDOWN_PROTOCOL_ERR_VERSION_INVALID
	 || updown_err_code == CEREBRO_UPDOWN_PROTOCOL_ERR_UPDOWN_REQUEST_INVALID
	 || updown_err_code == CEREBRO_UPDOWN_PROTOCOL_ERR_TIMEOUT_INVALID
	 || updown_err_code == CEREBRO_UPDOWN_PROTOCOL_ERR_INTERNAL_SYSTEM_ERROR);
  
  memset(&res, '\0', CEREBRO_UPDOWN_RESPONSE_LEN);

  res.version = CEREBRO_UPDOWN_PROTOCOL_VERSION;
  res.updown_err_code = updown_err_code;
  res.end_of_responses = CEREBRO_UPDOWN_PROTOCOL_IS_LAST_RESPONSE;

  if (_cerebrod_updown_send_response(client_fd, &res) < 0)
    return -1;
      
  return 0;
}

/* 
 * _cerebrod_updown_respond_with_version_error
 *
 * respond to the updown_request with a version error, a special case
 * error response
 *
 * Return 0 on success, -1 on error
 */
static int
_cerebrod_updown_respond_with_version_error(int client_fd, int version)
{
  /* 
   * Currently only one protocol version in this tool's history, so
   * this is easy
   */

  if (version == 1)
    {
      struct cerebro_updown_response res;

      memset(&res, '\0', CEREBRO_UPDOWN_RESPONSE_LEN);

      res.version = version;
      res.updown_err_code = CEREBRO_UPDOWN_PROTOCOL_ERR_VERSION_INVALID;
      res.end_of_responses = CEREBRO_UPDOWN_PROTOCOL_IS_LAST_RESPONSE;
      
      if (_cerebrod_updown_send_response(client_fd, &res) < 0)
        return -1;
    }
  else
    {
      /* We shouldn't end up here ... eek! */

      if (_cerebrod_updown_respond_with_error(client_fd,
                                              CEREBRO_UPDOWN_PROTOCOL_ERR_INTERNAL_SYSTEM_ERROR) < 0)
        return -1;
    }

  return 0;
}

/*  
 * _cerebrod_updown_evaluate_updown_state
 *
 * Callback function for list_for_each, to determine if a node is
 * up or down and if state data should be sent.
 *
 * Return 0 on success, -1 on error
 */
static int
_cerebrod_updown_evaluate_updown_state(void *x, void *arg)
{
  struct cerebrod_updown_node_data *ud;
  struct cerebrod_updown_evaluation_data *ed;
  struct cerebro_updown_response *res = NULL;
  u_int8_t updown_state;
#ifndef NDEBUG
  int rv;
#endif /* NDEBUG */

  assert(x);
  assert(arg);

  ud = (struct cerebrod_updown_node_data *)x;
  ed = (struct cerebrod_updown_evaluation_data *)arg;
  
#ifndef NDEBUG
  /* Should be called with lock already set */
  rv = Pthread_mutex_trylock(&updown_node_data_lock);
  if (rv != EBUSY)
    cerebro_err_exit("%s(%s:%d): mutex not locked",	
                     __FILE__, __FUNCTION__, __LINE__);

  /* With locking, it shouldn't be possible for local time to be
   * greater than the time stored in any last_received time.
   */
  if (ed->time_now < ud->last_received)
    cerebro_err_debug("%s(%s:%d): last_received time later than time_now time",
                      __FILE__, __FUNCTION__, __LINE__);
#endif /* NDEBUG */

  if ((ed->time_now - ud->last_received) < ed->timeout_len)
    updown_state = CEREBRO_UPDOWN_PROTOCOL_STATE_NODE_UP;
  else
    updown_state = CEREBRO_UPDOWN_PROTOCOL_STATE_NODE_DOWN;

  if ((ed->updown_request == CEREBRO_UPDOWN_PROTOCOL_REQUEST_UP_NODES
       && updown_state != CEREBRO_UPDOWN_PROTOCOL_STATE_NODE_UP)
      || (ed->updown_request == CEREBRO_UPDOWN_PROTOCOL_REQUEST_DOWN_NODES
          && updown_state != CEREBRO_UPDOWN_PROTOCOL_STATE_NODE_DOWN))
    return 0;

  res = Malloc(sizeof(struct cerebro_updown_response));
  res->version = CEREBRO_UPDOWN_PROTOCOL_VERSION;
  res->updown_err_code = CEREBRO_UPDOWN_PROTOCOL_ERR_SUCCESS;
  res->end_of_responses = CEREBRO_UPDOWN_PROTOCOL_IS_NOT_LAST_RESPONSE;
#ifndef NDEBUG
  if (ud->nodename && strlen(ud->nodename) > CEREBRO_MAXNODENAMELEN)
    cerebro_err_debug("%s(%s:%d): invalid node name length: %s", 
                      __FILE__, __FUNCTION__, __LINE__,
                      ud->nodename);
#endif /* NDEBUG */
  /* strncpy, b/c terminating character not required */
  strncpy(res->nodename, ud->nodename, CEREBRO_MAXNODENAMELEN);
  res->updown_state = updown_state;

  if (!list_append(ed->node_responses, res))
    {
      cerebro_err_debug("%s(%s:%d): list_append: %s", 
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      Free(res);
      return -1;
    }

  return 0;
}

/*
 * _cerebrod_updown_send_node_responses
 *
 * respond to the updown_request with a node
 *
 * Return 0 on success, -1 on error
 */
static int
_cerebrod_updown_send_node_responses(void *x, void *arg)
{
  struct cerebro_updown_response *res;
  int client_fd;

  assert(x);
  assert(arg);

  res = (struct cerebro_updown_response *)x;
  client_fd = *((int *)arg);

  if (_cerebrod_updown_send_response(client_fd, res) < 0)
    return -1;

  return 0;
}

/*  
 * _cerebrod_updown_respond_with_updown_nodes
 *
 * Return 0 on success, -1 on error
 */
static int
_cerebrod_updown_respond_with_updown_nodes(int client_fd,
					   unsigned int updown_request, 
					   unsigned int timeout_len)
{
  struct cerebrod_updown_evaluation_data ed;
  struct cerebro_updown_response end_res;
  struct timeval tv;
  List node_responses = NULL;

  assert(client_fd >= 0);
  assert(updown_request == CEREBRO_UPDOWN_PROTOCOL_REQUEST_UP_NODES
	 || updown_request == CEREBRO_UPDOWN_PROTOCOL_REQUEST_DOWN_NODES
	 || updown_request == CEREBRO_UPDOWN_PROTOCOL_REQUEST_UP_AND_DOWN_NODES);
  assert(timeout_len);
  assert(updown_node_data);

  memset(&ed, '\0', sizeof(struct cerebrod_updown_evaluation_data));

  Pthread_mutex_lock(&updown_node_data_lock);
  
  if (!List_count(updown_node_data))
    {
      Pthread_mutex_unlock(&updown_node_data_lock);
      goto end_response;
    }

  if (!(node_responses = list_create((ListDelF)_Free)))
    {
      cerebro_err_debug("%s(%s:%d): list_create failed: %s", 
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      Pthread_mutex_unlock(&updown_node_data_lock);
      _cerebrod_updown_respond_with_error(client_fd,
					  CEREBRO_UPDOWN_PROTOCOL_ERR_INTERNAL_SYSTEM_ERROR);
      goto cleanup;
    }

  Gettimeofday(&tv, NULL);
  ed.client_fd = client_fd;
  ed.updown_request = updown_request;
  ed.timeout_len = timeout_len;
  ed.time_now = tv.tv_sec;
  ed.node_responses = node_responses;

  if (list_for_each(updown_node_data, 
		    _cerebrod_updown_evaluate_updown_state, 
		    &ed) < 0)
    {
      Pthread_mutex_unlock(&updown_node_data_lock);
      _cerebrod_updown_respond_with_error(client_fd,
					  CEREBRO_UPDOWN_PROTOCOL_ERR_INTERNAL_SYSTEM_ERROR);
      goto cleanup;
    }

  /* Evaluation is done.  Transmission of the results can be done
   * without this lock.
   */
  Pthread_mutex_unlock(&updown_node_data_lock);

  if (List_count(node_responses))
    {
      if (list_for_each(node_responses,
			_cerebrod_updown_send_node_responses, 
			&client_fd) < 0)
        {
          Pthread_mutex_unlock(&updown_node_data_lock);
          _cerebrod_updown_respond_with_error(client_fd,
                                              CEREBRO_UPDOWN_PROTOCOL_ERR_INTERNAL_SYSTEM_ERROR);
          goto cleanup;
        }
    }

 end_response:
  /* Send end response */
  memset(&end_res, '\0', sizeof(struct cerebro_updown_response));
  end_res.version = CEREBRO_UPDOWN_PROTOCOL_VERSION;
  end_res.updown_err_code = CEREBRO_UPDOWN_PROTOCOL_ERR_SUCCESS;
  end_res.end_of_responses = CEREBRO_UPDOWN_PROTOCOL_IS_LAST_RESPONSE;

  if (_cerebrod_updown_send_response(client_fd, &end_res) < 0)
    return -1;

  list_destroy(node_responses);
  return 0;

 cleanup:
  if (node_responses)
    list_destroy(node_responses);
  return -1;
}

/* 
 * _cerebrod_updown_service_connection
 *
 * Thread to service a connection from a client to retrieve updown
 * node data.  Use wrapper functions minimally, b/c we want to return
 * errors to the user instead of exitting with errors.
 *
 * Passed int * pointer to client TCP socket file descriptor
 *
 * Executed in detached state, no return value.
 */
static void *
_cerebrod_updown_service_connection(void *arg)
{
  int client_fd;
  struct cerebro_updown_request req;
  client_fd = *((int *)arg);

  if (_cerebrod_updown_receive_request(client_fd, &req) < 0)
    /* At this point, there is no successfully read request, therefore
     * there is no reason to respond w/ a response and an error code.
     */
    goto done;
  
  _cerebrod_updown_request_dump(&req);

  if (req.version != CEREBRO_UPDOWN_PROTOCOL_VERSION)
    {
      _cerebrod_updown_respond_with_version_error(client_fd, req.version);
      goto done;
    }

  if (!(req.updown_request == CEREBRO_UPDOWN_PROTOCOL_REQUEST_UP_NODES
	|| req.updown_request == CEREBRO_UPDOWN_PROTOCOL_REQUEST_DOWN_NODES
	|| req.updown_request == CEREBRO_UPDOWN_PROTOCOL_REQUEST_UP_AND_DOWN_NODES))
    {
      _cerebrod_updown_respond_with_error(client_fd,
					  CEREBRO_UPDOWN_PROTOCOL_ERR_UPDOWN_REQUEST_INVALID);
      goto done;
    }

  if (!req.timeout_len)
    req.timeout_len = CEREBRO_UPDOWN_TIMEOUT_LEN_DEFAULT;

  if (_cerebrod_updown_respond_with_updown_nodes(client_fd, 
						 req.updown_request, 
						 req.timeout_len) < 0)
    goto done;

 done:
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
		  cerebro_err_debug("%s(%s:%d): error re-initializing socket",
                                    __FILE__, __FUNCTION__, __LINE__);

		  /* Wait a bit, so we don't spin */
		  sleep(CEREBROD_UPDOWN_REINITIALIZE_WAIT);
		}
              else
                cerebro_err_debug("%s(%s:%d): success re-initializing socket",
                                  __FILE__, __FUNCTION__, __LINE__);
            }
          else if (errno == EINTR)
            cerebro_err_debug("%s(%s:%d): accept: %s", 
                              __FILE__, __FUNCTION__, __LINE__,
                              strerror(errno));
          else
            cerebro_err_exit("%s(%s:%d): accept: %s", 
                             __FILE__, __FUNCTION__, __LINE__,
                             strerror(errno));
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
                     _cerebrod_updown_service_connection, 
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
      fprintf(stderr, "* Updown Server Insertion: Node=%s\n", ud->nodename);
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
      fprintf(stderr, "* Updown Server Update: Node=%s Last_Received=%s\n", ud->nodename, strbuf);
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
          ud->nodename, ud->discovered, ud->last_received);
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
	    {
	      fprintf(stderr, "_cerebrod_updown_dump_updown_node_data: "
		      "invalid dump count: rv=%d numnodes=%d",
		      rv, updown_node_data_index_numnodes);
	      exit(1);
	    }
        }
      else
        fprintf(stderr, "_cerebrod_updown_dump_node_data: "
                "called with empty list\n");
      fprintf(stderr, "**************************************\n");
      Pthread_mutex_unlock(&debug_output_mutex);
      Pthread_mutex_unlock(&updown_node_data_lock);
    }
#endif /* NDEBUG */
}

void 
cerebrod_updown_update_data(char *nodename, u_int32_t last_received)
{
  struct cerebrod_updown_node_data *ud;
  int update_output_flag = 0;

  if (!cerebrod_updown_initialization_complete)
    cerebro_err_exit("%s(%s:%d): initialization not complete",
                     __FILE__, __FUNCTION__, __LINE__);

  Pthread_mutex_lock(&updown_node_data_lock);
  if (!(ud = Hash_find(updown_node_data_index, nodename)))
    {
      char *key;

      ud = Malloc(sizeof(struct cerebrod_updown_node_data));

      key = Strdup(nodename);

      ud->nodename = Strdup(nodename);
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

      /* Ok to call debug output function, since updown_node_data_lock
       * is locked.
       */
      _cerebrod_updown_output_insert(ud);
    }
  Pthread_mutex_unlock(&updown_node_data_lock);
  
  Pthread_mutex_lock(&(ud->updown_node_data_lock));
  if (last_received >= ud->last_received)
    {
      ud->discovered = 1;
      ud->last_received = last_received;
      update_output_flag++;

      /* Can't call a debug output function in here, it can cause a
       * deadlock b/c the updown_node_data_lock is not locked.
       */
    }
  Pthread_mutex_unlock(&(ud->updown_node_data_lock));

  if (update_output_flag)
    _cerebrod_updown_output_update(ud);

  _cerebrod_updown_dump_updown_node_data_list();
}
