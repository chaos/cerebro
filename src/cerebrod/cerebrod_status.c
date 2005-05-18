/*****************************************************************************\
 *  $Id: cerebrod_status.c,v 1.5 2005-05-18 18:53:00 achu Exp $
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

#include "cerebro_marshalling.h"
#include "cerebro_module.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_error.h"
#include "cerebro/cerebro_status_protocol.h"

#include "cerebrod.h"
#include "cerebrod_config.h"
#include "cerebrod_status.h"
#include "cerebrod_util.h"
#include "fd.h"
#include "list.h"
#include "wrappers.h"

#define CEREBROD_STATUS_BACKLOG                        10

#define CEREBROD_STATUS_NODE_DATA_INDEX_SIZE_DEFAULT   1024
#define CEREBROD_STATUS_NODE_DATA_INDEX_SIZE_INCREMENT 1024
#define CEREBROD_STATUS_REHASH_LIMIT                   (status_node_data_index_size*2)

extern struct cerebrod_config conf;
#if CEREBRO_DEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* CEREBRO_DEBUG */

/*
 * cerebrod_status_initialization_complete
 * cerebrod_status_initialization_complete_cond
 * cerebrod_status_initialization_complete_lock
 *
 * variables for synchronizing initialization between different pthreads
 * and signaling when it is complete
 */
int cerebrod_status_initialization_complete = 0;
pthread_cond_t cerebrod_status_initialization_complete_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t cerebrod_status_initialization_complete_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * status_fd
 *
 * status server file descriptor
 */
int status_fd = 0;

/* 
 * status_node_data
 *
 * contains list of nodes with last received times
 */
List status_node_data = NULL;

/*
 * status_node_data_index
 * status_node_data_index_numnodes
 * status_node_data_index_size
 *
 * hash index into status_node_data list for faster access, number of
 * currently indexed entries, and index size
 */
hash_t status_node_data_index = NULL;
int status_node_data_index_numnodes;
int status_node_data_index_size;

/*  
 * status_node_data_lock
 *
 * lock to protect pthread access to both status node data list and
 * index
 */
pthread_mutex_t status_node_data_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * _cerebrod_status_create_and_setup_socket
 *
 * Create and setup the status server socket.  Do not use wrappers in
 * this function.  We want to give the server additional chances to
 * "survive" an error condition.
 *
 * Returns file descriptor on success, -1 on error
 */
static int
_cerebrod_status_create_and_setup_socket(void)
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
  server_addr.sin_port = htons(conf.status_server_port);
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

  if (listen(temp_fd, CEREBROD_STATUS_BACKLOG) < 0)
    {
      cerebro_err_debug("%s(%s:%d): listen: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      return -1;
    }

#if CEREBRO_DEBUG
  if (conf.debug)
    {
      int optval = 1;
        
      /* For quick start/restart debugging purposes */
      if (setsockopt(temp_fd,
                     SOL_SOCKET,
                     SO_REUSEADDR,
                     &optval,
                     sizeof(int)) < 0)
        {
          cerebro_err_debug("%s(%s:%d): setsockopt: %s",
                            __FILE__, __FUNCTION__, __LINE__,
                            strerror(errno));
          return -1;
        }
                 
    }
#endif /* CEREBRO_DEBUG */

  return temp_fd;
}

/* 
 * _status_node_data_strcmp
 *
 * callback function for list_sort to sort status node names
 */
static int 
_status_node_data_strcmp(void *x, void *y)
{
  assert(x);
  assert(y);

  return strcmp(((struct cerebrod_status_node_data *)x)->nodename,
                ((struct cerebrod_status_node_data *)y)->nodename);
}

/*
 * _cerebrod_status_initialize
 *
 * perform status server initialization
 */
static void
_cerebrod_status_initialize(void)
{
  int numnodes = 0;

  if (cerebrod_status_initialization_complete)
    return;

  if ((status_fd = _cerebrod_status_create_and_setup_socket()) < 0)
    cerebro_err_exit("%s(%s:%d): status_fd setup failed",
                     __FILE__, __FUNCTION__, __LINE__);

  if ((numnodes = _cerebro_clusterlist_module_numnodes()) < 0)
    cerebro_err_exit("%s(%s:%d): _cerebro_clusterlist_module_numnodes",
		     __FILE__, __FUNCTION__, __LINE__);

  if (numnodes > 0)
    {
      status_node_data_index_numnodes = numnodes;
      status_node_data_index_size = numnodes;
    }
  else
    {
      status_node_data_index_numnodes = 0;
      status_node_data_index_size = CEREBROD_STATUS_NODE_DATA_INDEX_SIZE_DEFAULT;
    }

  status_node_data = List_create((ListDelF)_Free);
  status_node_data_index = Hash_create(status_node_data_index_size,
                                       (hash_key_f)hash_key_string,
                                       (hash_cmp_f)strcmp,
                                       (hash_del_f)_Free);
  
  /* If the clusterlist module contains nodes, retrieve all of these
   * nodes and put them into the updown_node_data list.  All updates
   * will involve updating data rather than involve insertion.
   */
  if (numnodes > 0)
    {
      int i;
      char **nodes;

      if (_cerebro_clusterlist_module_get_all_nodes(&nodes) < 0)
        cerebro_err_exit("%s(%s:%d): _cerebro_clusterlist_module_get_all_nodes",
                         __FILE__, __FUNCTION__, __LINE__);

      for (i = 0; i < numnodes; i++)
        {
          struct cerebrod_status_node_data *sd;

          sd = Malloc(sizeof(struct cerebrod_status_node_data));

          sd->nodename = Strdup(nodes[i]);
          sd->status_data = Hash_create(CEREBRO_STATUS_MAX,
                                        (hash_key_f)hash_key_string,
                                        (hash_cmp_f)strcmp,
                                        (hash_del_f)_Free);
          sd->status_data_count = 0;
          sd->last_received_time = 0;
          Pthread_mutex_init(&(sd->status_node_data_lock), NULL);

          List_append(status_node_data, sd);
          Hash_insert(status_node_data_index, Strdup(nodes[i]), sd);

          list_sort(status_node_data, (ListCmpF)_status_node_data_strcmp);

          free(nodes[i]);
        }

      free(nodes);
    }

  Pthread_mutex_lock(&cerebrod_status_initialization_complete_lock);
  cerebrod_status_initialization_complete++;
  Pthread_cond_signal(&cerebrod_status_initialization_complete_cond);
  Pthread_mutex_unlock(&cerebrod_status_initialization_complete_lock);
}

#if 0

/*
 * _cerebrod_updown_response_marshall
 *
 * marshall contents of a updown response packet buffer
 *
 * Returns length written to buffer on success, -1 on error
 */
static int
_cerebrod_updown_response_marshall(struct cerebro_updown_response *res,
				   char *buf, 
                                   unsigned int buflen)
{
  int len, count = 0;
 
  assert(res);
  assert(buf);
  assert(buflen >= CEREBRO_UPDOWN_RESPONSE_LEN);

  memset(buf, '\0', buflen);

  if ((len = _cerebro_marshall_int32(res->version, 
                                     buf + count, 
                                     buflen - count)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_marshall_int32",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  count += len;
 
  if ((len = _cerebro_marshall_uint32(res->updown_err_code, 
                                      buf + count, 
                                      buflen - count)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_marshall_uint32",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  count += len;

  if ((len = _cerebro_marshall_uint8(res->end_of_responses, 
                                     buf + count, 
                                     buflen - count)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_marshall_uint8",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  count += len;

  if ((len = _cerebro_marshall_buffer(res->nodename,
                                      sizeof(res->nodename),
                                      buf + count,
                                      buflen - count)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_marshall_buffer",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  count += len;


  if ((len = _cerebro_marshall_uint8(res->updown_state, 
                                     buf + count, 
                                     buflen - count)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_marshall_uint8",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  count += len;

  return count;
}

/*
 * _cerebrod_updown_err_response_marshall
 *
 * marshall contents of a updown err response packet buffer
 *
 * Returns length written to buffer on success, -1 on error
 */
static int
_cerebrod_updown_err_response_marshall(struct cerebro_updown_err_response *err_res,
                                       char *buf, 
                                       unsigned int buflen)
{
  int len, count = 0;
 
  assert(err_res);
  assert(buf);
  assert(buflen >= CEREBRO_UPDOWN_ERR_RESPONSE_LEN);

  memset(buf, '\0', buflen);

  if ((len = _cerebro_marshall_int32(err_res->version, 
                                     buf + count, 
                                     buflen - count)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_marshall_int32",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  count += len;
 
  if ((len = _cerebro_marshall_uint32(err_res->updown_err_code, 
                                      buf + count, 
                                      buflen - count)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_marshall_uint32",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  count += len;

  return count;
}

/*
 * _cerebrod_updown_request_unmarshall
 *
 * unmarshall contents of a updown request packet buffer
 *
 * Returns length of data unmarshalled on success, -1 on error
 */
static int
_cerebrod_updown_request_unmarshall(struct cerebro_updown_request *req,
				    const char *buf, 
                                    unsigned int buflen)
{
  int len, count = 0;

  assert(req);
  assert(buf);
 
  if ((len = _cerebro_unmarshall_int32(&(req->version), 
                                       buf + count, 
                                       buflen - count)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_unmarshall_int32",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!len)
    return count;

  count += len;

  if ((len = _cerebro_unmarshall_uint32(&(req->updown_request), 
                                        buf + count, 
                                        buflen - count)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_unmarshall_uint32",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!len)
    return count;

  count += len;
 
  if ((len = _cerebro_unmarshall_uint32(&(req->timeout_len), 
                                        buf + count, 
                                        buflen - count)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebro_unmarshall_uint32",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!len)
    return count;

  count += len;
   
  return count;
}

/*
 * _cerebrod_updown_request_receive
 *
 * Receive updown server request
 * 
 * Return request packet and length of packet unmarshalled on success,
 * -1 on error
 */
static int
_cerebrod_updown_request_receive(int client_fd,	
				 struct cerebro_updown_request *req)
{
  int rv, bytes_read = 0;
  char buf[CEREBRO_PACKET_BUFLEN];

  assert(client_fd >= 0);
  assert(req);

  memset(buf, '\0', CEREBRO_PACKET_BUFLEN);

  /* Wait for request from client */
  while (bytes_read < CEREBRO_UPDOWN_REQUEST_LEN)
    {
      fd_set rfds;
      struct timeval tv;
      int num;

      tv.tv_sec = CEREBRO_UPDOWN_PROTOCOL_SERVER_TIMEOUT_LEN;
      tv.tv_usec = 0;
      
      FD_ZERO(&rfds);
      FD_SET(client_fd, &rfds);
  
      if ((num = select(client_fd + 1, &rfds, NULL, NULL, &tv)) < 0)
	{
	  cerebro_err_debug("%s(%s:%d): select: %s", 
                            __FILE__, __FUNCTION__, __LINE__,
                            strerror(errno));
	  goto cleanup;
	}

      if (!num)
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

          /* Don't use fd_read_n b/c it loops until exactly
           * CEREBRO_UPDOWN_REQUEST_LEN is read.  Due to version
           * incompatability, we may want to read a smaller packet.
           */
	  if ((n = read(client_fd, 
                        buf + bytes_read, 
                        CEREBRO_UPDOWN_REQUEST_LEN - bytes_read)) < 0)
	    {
	      cerebro_err_debug("%s(%s:%d): read: %s", 
                                __FILE__, __FUNCTION__, __LINE__,
                                strerror(errno));
	      goto cleanup;
	    }

	  if (!n)
            {
              /* Pipe closed */
              goto cleanup;
            }

	  bytes_read += n;
	}
      else
	{
	  cerebro_err_debug("%s(%s:%d): num != 0 but fd not set",
                            __FILE__, __FUNCTION__, __LINE__);
	  goto cleanup;
	}
    }

 unmarshall_received:
  if ((rv = _cerebrod_updown_request_unmarshall(req, buf, bytes_read)) < 0)
    goto cleanup;

  return rv;

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
#if CEREBRO_DEBUG
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
#endif /* CEREBRO_DEBUG */
}

/*
 * _cerebrod_updown_response_send_one
 *
 * send a response packet to the client
 *
 * Return 0 on success, -1 on error
 */
static int
_cerebrod_updown_response_send_one(int client_fd, 
				   struct cerebro_updown_response *res)
{
  char buf[CEREBRO_PACKET_BUFLEN];
  int res_len;

  assert(client_fd >= 0);
  assert(res);

  if ((res_len = _cerebrod_updown_response_marshall(res, 
						    buf, 
						    CEREBRO_PACKET_BUFLEN)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebrod_updown_response_marshall",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (fd_write_n(client_fd, buf, res_len) < 0)
    {
      cerebro_err_debug("%s(%s:%d): fd_write_n: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      return -1;
    }

  return 0;
}

/*
 * _cerebrod_updown_err_response_send
 *
 * send an error response packet to the client
 *
 * Return 0 on success, -1 on error
 */
static int
_cerebrod_updown_err_response_send(int client_fd, 
                                   struct cerebro_updown_err_response *err_res)
{
  char buf[CEREBRO_PACKET_BUFLEN];
  int err_res_len;

  assert(client_fd >= 0);
  assert(err_res);

  if ((err_res_len = _cerebrod_updown_err_response_marshall(err_res, 
                                                            buf, 
                                                            CEREBRO_PACKET_BUFLEN)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): _cerebrod_updown_err_response_marshall",
                        __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
  
  if (fd_write_n(client_fd, buf, err_res_len) < 0)
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
_cerebrod_updown_respond_with_error(int client_fd, 
                                    int32_t version,
                                    u_int32_t updown_err_code)
{
  struct cerebro_updown_response res;

  assert(client_fd >= 0);
  assert(updown_err_code == CEREBRO_UPDOWN_PROTOCOL_ERR_VERSION_INVALID
	 || updown_err_code == CEREBRO_UPDOWN_PROTOCOL_ERR_UPDOWN_REQUEST_INVALID
	 || updown_err_code == CEREBRO_UPDOWN_PROTOCOL_ERR_TIMEOUT_INVALID
         || updown_err_code == CEREBRO_UPDOWN_PROTOCOL_ERR_PACKET_INVALID
	 || updown_err_code == CEREBRO_UPDOWN_PROTOCOL_ERR_INTERNAL_SYSTEM_ERROR);
  
  memset(&res, '\0', CEREBRO_UPDOWN_RESPONSE_LEN);

  /* 
   * If the version sent is an older version, send a packet in
   * that older verion's packet format.  If it is a version that
   * we don't know of, fall through to the err_response section.
   */

  if (updown_err_code == CEREBRO_UPDOWN_PROTOCOL_ERR_VERSION_INVALID)
    {
      if (version == 1)
        {
          res.version = version;
          res.updown_err_code = CEREBRO_UPDOWN_PROTOCOL_ERR_VERSION_INVALID;
          res.end_of_responses = CEREBRO_UPDOWN_PROTOCOL_IS_LAST_RESPONSE;
          
          if (_cerebrod_updown_response_send_one(client_fd, &res) < 0)
            return -1;

          return 0;
        }
    }

  if (updown_err_code == CEREBRO_UPDOWN_PROTOCOL_ERR_VERSION_INVALID
      || updown_err_code == CEREBRO_UPDOWN_PROTOCOL_ERR_PACKET_INVALID)
    {
      struct cerebro_updown_err_response err_res;
      memset(&err_res, '\0', CEREBRO_UPDOWN_ERR_RESPONSE_LEN);
      
      err_res.version = version;
      err_res.updown_err_code = updown_err_code;

      if (_cerebrod_updown_err_response_send(client_fd, &err_res) < 0)
        return -1;

      return 0;
    }

  res.version = version;
  res.updown_err_code = updown_err_code;
  res.end_of_responses = CEREBRO_UPDOWN_PROTOCOL_IS_LAST_RESPONSE;
  
  if (_cerebrod_updown_response_send_one(client_fd, &res) < 0)
    return -1;
      
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
#if CEREBRO_DEBUG
  int rv;
#endif /* CEREBRO_DEBUG */

  assert(x);
  assert(arg);

  ud = (struct cerebrod_updown_node_data *)x;
  ed = (struct cerebrod_updown_evaluation_data *)arg;
  
#if CEREBRO_DEBUG
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
#endif /* CEREBRO_DEBUG */

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
#if CEREBRO_DEBUG
  if (ud->nodename && strlen(ud->nodename) > CEREBRO_MAXNODENAMELEN)
    cerebro_err_debug("%s(%s:%d): invalid node name length: %s", 
                      __FILE__, __FUNCTION__, __LINE__,
                      ud->nodename);
#endif /* CEREBRO_DEBUG */
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
 * _cerebrod_updown_response_send_all
 *
 * respond to the updown_request with a node
 *
 * Return 0 on success, -1 on error
 */
static int
_cerebrod_updown_response_send_all(void *x, void *arg)
{
  struct cerebro_updown_response *res;
  int client_fd;

  assert(x);
  assert(arg);

  res = (struct cerebro_updown_response *)x;
  client_fd = *((int *)arg);

  if (_cerebrod_updown_response_send_one(client_fd, res) < 0)
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
                                           int32_t version,
					   u_int32_t updown_request, 
					   u_int32_t timeout_len)
{
  struct cerebrod_updown_evaluation_data ed;
  struct cerebro_updown_response end_res;
  struct timeval tv;
  List node_responses = NULL;

  assert(client_fd >= 0);
  assert(version == CEREBRO_UPDOWN_PROTOCOL_VERSION);
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
      cerebro_err_debug("%s(%s:%d): list_create: %s", 
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      Pthread_mutex_unlock(&updown_node_data_lock);
      _cerebrod_updown_respond_with_error(client_fd,
                                          version,
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
                                          version,
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
			_cerebrod_updown_response_send_all, 
			&client_fd) < 0)
        {
          Pthread_mutex_unlock(&updown_node_data_lock);
          _cerebrod_updown_respond_with_error(client_fd,
                                              version,
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

  if (_cerebrod_updown_response_send_one(client_fd, &end_res) < 0)
    return -1;

  list_destroy(node_responses);
  return 0;

 cleanup:
  if (node_responses)
    list_destroy(node_responses);
  return -1;
}

#endif /* 0 */

/* 
 * _cerebrod_status_service_connection
 *
 * Thread to service a connection from a client to retrieve status
 * node data.  Use wrapper functions minimally, b/c we want to return
 * errors to the user instead of exitting with errors.
 *
 * Passed int * pointer to client TCP socket file descriptor
 *
 * Executed in detached state, no return value.
 */
static void *
_cerebrod_status_service_connection(void *arg)
{
#if 0
  int client_fd, req_len;
  struct cerebro_status_request req;
  client_fd = *((int *)arg);

  memset(&req, '\0', sizeof(struct cerebro_status_request));
  if ((req_len = _cerebrod_status_request_receive(client_fd, &req)) < 0)
    /* At this point, there is no successfully read request, therefore
     * there is no reason to respond w/ a response and an error code.
     */
    goto out;
  
  _cerebrod_status_request_dump(&req);

  if (req_len != CEREBRO_STATUS_REQUEST_LEN)
    {
      cerebro_err_debug("%s(%s:%d): received packet unexpected size: "
                        "expect %d, req_len %d", 
                        __FILE__, __FUNCTION__, __LINE__,
                        CEREBRO_STATUS_REQUEST_LEN, req_len);

      if (req_len >= sizeof(req.version)
          && req.version != CEREBRO_STATUS_PROTOCOL_VERSION)
        {
          _cerebrod_status_respond_with_error(client_fd, 
                                              req.version,
                                              CEREBRO_STATUS_PROTOCOL_ERR_VERSION_INVALID);
          goto out;
        }

      _cerebrod_status_respond_with_error(client_fd, 
                                          req.version,
                                          CEREBRO_STATUS_PROTOCOL_ERR_PACKET_INVALID);
      goto out;
    }

  if (req.version != CEREBRO_STATUS_PROTOCOL_VERSION)
    {
      _cerebrod_status_respond_with_error(client_fd, 
                                          req.version,
                                          CEREBRO_STATUS_PROTOCOL_ERR_VERSION_INVALID);
      goto out;
    }

  if (!(req.status_request == CEREBRO_STATUS_PROTOCOL_REQUEST_UP_NODES
	|| req.status_request == CEREBRO_STATUS_PROTOCOL_REQUEST_DOWN_NODES
	|| req.status_request == CEREBRO_STATUS_PROTOCOL_REQUEST_UP_AND_DOWN_NODES))
    {
      _cerebrod_status_respond_with_error(client_fd,
                                          req.version,
					  CEREBRO_STATUS_PROTOCOL_ERR_STATUS_REQUEST_INVALID);
      goto out;
    }

  if (!req.timeout_len)
    req.timeout_len = CEREBRO_STATUS_TIMEOUT_LEN_DEFAULT;

  if (_cerebrod_status_respond_with_status_nodes(client_fd, 
                                                 req.version,
						 req.status_request, 
						 req.timeout_len) < 0)
    goto out;

 out:
  Free(arg);
  Close(client_fd);
#endif /* 0 */
  return NULL;
}

void *
cerebrod_status(void *arg)
{
  _cerebrod_status_initialize();

  for (;;)
    {
      pthread_t thread;
      pthread_attr_t attr;
      int client_fd, client_addr_len, *arg;
      struct sockaddr_in client_addr;

      client_addr_len = sizeof(struct sockaddr_in);
      if ((client_fd = accept(status_fd, 
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
	      || status_fd < 0)
            {
              if (!(status_fd < 0))
		close(status_fd);	/* no-wrapper, make best effort */

              if ((status_fd = _cerebrod_status_create_and_setup_socket()) < 0)
		{
		  cerebro_err_debug("%s(%s:%d): error re-initializing socket",
                                    __FILE__, __FUNCTION__, __LINE__);

		  /* Wait a bit, so we don't spin */
		  sleep(CEREBROD_STATUS_REINITIALIZE_WAIT);
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
                     _cerebrod_status_service_connection, 
                     (void *)arg);
      Pthread_attr_destroy(&attr);

    }

  return NULL;			/* NOT REACHED */
}

/*  
 * _cerebrod_status_output_insert
 *
 * Output debugging info about a recently inserted node
 */
static void
_cerebrod_status_output_insert(struct cerebrod_status_node_data *ud)
{
#if CEREBRO_DEBUG
  assert(ud);
 
  if (conf.debug && conf.status_server_debug)
    {
      Pthread_mutex_lock(&debug_output_mutex);
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Status Server Insertion: Node=%s\n", ud->nodename);
      fprintf(stderr, "**************************************\n");
      Pthread_mutex_unlock(&debug_output_mutex);
    }
#endif /* CEREBRO_DEBUG */
}

/*  
 * _cerebrod_status_output_update
 *
 * Output debugging info about a recently updated node
 */
static void
_cerebrod_status_output_update(struct cerebrod_status_node_data *sd,
                               char *status_name)
{
#if CEREBRO_DEBUG
  assert(sd);
 
  if (conf.debug && conf.status_server_debug)
    {
      Pthread_mutex_lock(&debug_output_mutex);
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Status Server Update: Node=%s Status_Name=\"%s\"\n", 
	      sd->nodename, status_name);
      fprintf(stderr, "**************************************\n");
      Pthread_mutex_unlock(&debug_output_mutex);
    }
#endif /* CEREBRO_DEBUG */
}

#if CEREBRO_DEBUG
/*
 * _cerebrod_status_dump_status_data_item
 *
 * callback function from hash_for_each to dump status node data
 */
static int
_cerebrod_status_dump_status_data_item(void *data, const void *key, void *arg)
{
  struct cerebrod_status_data *status_data;
  char *nodename;

  assert(data);
  assert(key);
  assert(arg);

  status_data = (struct cerebrod_status_data *)data;
  nodename = (char *)arg;

  fprintf(stderr, "* %s: status_name=%s status_type=%d ",
          nodename,
          status_data->status_name,
          status_data->status_type);
  if (status_data->status_type == CEREBROD_STATUS_TYPE_INT32_T)
    fprintf(stderr, "status_value=%d", status_data->status_value.val_int32);
  else if (status_data->status_type == CEREBROD_STATUS_TYPE_U_INT32_T)
    fprintf(stderr, "status_value=%u", status_data->status_value.val_u_int32);
  else
    cerebro_err_debug("%s(%s:%d): nodename=%s invalid status_type=%d",
                      __FILE__, __FUNCTION__, __LINE__,
                      (char *)key,
                      status_data->status_type);
  fprintf(stderr, "\n");
 
  return 1;
}


/*
 * _cerebrod_status_dump_status_node_data_item
 *
 * callback function from hash_for_each to dump status node data
 */
static int
_cerebrod_status_dump_status_node_data_item(void *x, void *arg)
{
  struct cerebrod_status_node_data *sd;
  int num;

  assert(x);
 
  sd = (struct cerebrod_status_node_data *)x;
 
  Pthread_mutex_lock(&(sd->status_node_data_lock));
  num = Hash_for_each(sd->status_data,
                      _cerebrod_status_dump_status_data_item,
                      sd->nodename);
  if (num != sd->status_data_count)
    {
      fprintf(stderr, "_cerebrod_status_dump_status_node_data_item: "
              "invalid dump count: num=%d numnodes=%d",
              num, sd->status_data_count);
      exit(1);
    }
  Pthread_mutex_unlock(&(sd->status_node_data_lock));
 
  return 1;
}
#endif /* CEREBRO_DEBUG */

/*
 * _cerebrod_status_dump_status_node_data_list
 *
 * Dump contents of status node data list
 */
static void
_cerebrod_status_dump_status_node_data_list(void)
{
#if CEREBRO_DEBUG
  if (conf.debug && conf.status_server_debug)
    {
      int num;
 
      Pthread_mutex_lock(&status_node_data_lock);
      Pthread_mutex_lock(&debug_output_mutex);
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Status List State\n");
      fprintf(stderr, "* -----------------------\n");
      fprintf(stderr, "* Listed Nodes: %d\n", status_node_data_index_numnodes);
      fprintf(stderr, "* -----------------------\n");
      if (status_node_data_index_numnodes > 0)
        {
          num = List_for_each(status_node_data,
			      _cerebrod_status_dump_status_node_data_item,
			      NULL);
          if (num != status_node_data_index_numnodes)
	    {
	      fprintf(stderr, "_cerebrod_status_dump_status_node_data: "
		      "invalid dump count: num=%d numnodes=%d",
		      num, status_node_data_index_numnodes);
	      exit(1);
	    }
        }
      else
        fprintf(stderr, "_cerebrod_status_dump_node_data: "
                "called with empty list\n");
      fprintf(stderr, "**************************************\n");
      Pthread_mutex_unlock(&debug_output_mutex);
      Pthread_mutex_unlock(&status_node_data_lock);
    }
#endif /* CEREBRO_DEBUG */
}

/* 
 * _cerebrod_status_update_status_data
 *
 * Update the data of a particular status
 */
static void
_cerebrod_status_update_status_data(char *nodename,
                                    char *status_name,
                                    cerebrod_status_type_t status_type,
                                    cerebrod_status_value_t status_value,
                                    u_int32_t received_time)
{
  struct cerebrod_status_node_data *sd;
  int update_output_flag = 0;

  assert(nodename);
  assert(status_name);
  assert(status_type == CEREBROD_STATUS_TYPE_INT32_T
         || status_type == CEREBROD_STATUS_TYPE_U_INT32_T);

  Pthread_mutex_lock(&status_node_data_lock);
  if (!(sd = Hash_find(status_node_data_index, nodename)))
    {
      char *key;

      sd = Malloc(sizeof(struct cerebrod_status_node_data));

      key = Strdup(nodename);

      sd->nodename = Strdup(nodename);
      sd->status_data = Hash_create(CEREBRO_STATUS_MAX,
                                    (hash_key_f)hash_key_string,
                                    (hash_cmp_f)strcmp,
                                    (hash_del_f)_Free);
      sd->status_data_count = 0;
      sd->last_received_time = 0;
      Pthread_mutex_init(&(sd->status_node_data_lock), NULL);

      /* Re-hash if our hash is getting too small */
      if ((status_node_data_index_numnodes + 1) > CEREBROD_STATUS_REHASH_LIMIT)
	cerebrod_rehash(&status_node_data_index,
			&status_node_data_index_size,
			CEREBROD_STATUS_NODE_DATA_INDEX_SIZE_INCREMENT,
			status_node_data_index_numnodes,
			&status_node_data_lock);

      List_append(status_node_data, sd);
      Hash_insert(status_node_data_index, key, sd);
      status_node_data_index_numnodes++;

      /* Ok to call debug output function, since status_node_data_lock
       * is locked.
       */
      _cerebrod_status_output_insert(sd);
    }
  Pthread_mutex_unlock(&status_node_data_lock);
  
  Pthread_mutex_lock(&(sd->status_node_data_lock));
  if (received_time >= sd->last_received_time)
    {
      struct cerebrod_status_data *data;
      
      if (!(data = Hash_find(sd->status_data, status_name)))
        {
          char *key;

          if (sd->status_data_count >= CEREBRO_STATUS_MAX)
            {
              cerebro_err_debug("%s(%s:%d): too many status metrics: "
                                "nodename=%s",
                                __FILE__, __FUNCTION__, __LINE__,
                                nodename);
              goto max_status_data_count_out;
            }

          key = Strdup(status_name);
          data = (struct cerebrod_status_data *)Malloc(sizeof(struct cerebrod_status_data));
          data->status_name = Strdup(status_name);

          Hash_insert(sd->status_data, key, data);
          sd->status_data_count++;
        }
      else
        {
          if (data->status_type != status_type)
            cerebro_err_debug("%s(%s:%d): status type modified: old=%d new=%d",
                              __FILE__, __FUNCTION__, __LINE__,
                              data->status_type, status_type);
        }

      data->status_type = status_type;
      data->status_value = status_value;
      sd->last_received_time = received_time;
      update_output_flag++;
      
      /* Can't call a debug output function in here, it can cause a
       * deadlock b/c the status_node_data_lock is not locked.
       */
    }
 max_status_data_count_out:
  Pthread_mutex_unlock(&(sd->status_node_data_lock));

  if (update_output_flag)
    _cerebrod_status_output_update(sd, status_name);
}

void 
cerebrod_status_update_data(char *nodename,
                            struct cerebrod_heartbeat *hb,
                            u_int32_t received_time)
{
  cerebrod_status_value_t val;

  assert(nodename);
  assert(hb);
  
  if (!cerebrod_status_initialization_complete)
    cerebro_err_exit("%s(%s:%d): initialization not complete",
                     __FILE__, __FUNCTION__, __LINE__);

  val.val_u_int32 = hb->starttime;
  _cerebrod_status_update_status_data(nodename,
                                      CEREBRO_STATUS_STARTTIME,
                                      CEREBROD_STATUS_TYPE_U_INT32_T,
                                      val,
                                      received_time);
 
  val.val_u_int32 = hb->boottime;
  _cerebrod_status_update_status_data(nodename,
                                      CEREBRO_STATUS_BOOTTIME,
                                      CEREBROD_STATUS_TYPE_U_INT32_T,
                                      val,
                                      received_time);

  _cerebrod_status_dump_status_node_data_list();
}

