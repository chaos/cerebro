/*****************************************************************************\
 *  $Id: cerebrod_listener.c,v 1.58 2005-05-09 16:02:11 achu Exp $
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

#include "cerebro_module.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_error.h"
#include "cerebro/cerebro_marshalling.h"
#include "cerebro/cerebrod_heartbeat_protocol.h"

#include "cerebrod.h"
#include "cerebrod_config.h"
#include "cerebrod_heartbeat.h"
#include "cerebrod_listener.h"
#include "cerebrod_updown.h"
#include "cerebrod_util.h"
#include "wrappers.h"

#define CEREBROD_LISTENER_HASH_SIZE_DEFAULT   1024
#define CEREBROD_LISTENER_HASH_SIZE_INCREMENT 1024
#define CEREBROD_LISTENER_REHASH_LIMIT        (cluster_data_hash_size*2)

extern struct cerebrod_config conf;
#if CEREBRO_DEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* CEREBRO_DEBUG */

/* 
 * cerebrod_listener_initialization_complete
 * cerebrod_listener_initialization_complete_cond
 * cerebrod_listener_initialization_complete_lock
 *
 * variables for synchronizing initialization between different pthreads
 * and signaling when it is complete
 */
int cerebrod_listener_initialization_complete = 0;
pthread_cond_t cerebrod_listener_initialization_complete_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t cerebrod_listener_initialization_complete_lock = PTHREAD_MUTEX_INITIALIZER;

/* 
 * listener_fd
 * listener_fd_lock
 *
 * listener file descriptor and lock to protect concurrent access
 */
int listener_fd = 0;
pthread_mutex_t listener_fd_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * cluster_data_hash
 * cluster_data_hash_numnodes
 * cluster_data_hash_size
 * cluster_data_hash_lock
 *
 * cluster node data, number of currently hashed entries, hash size, and 
 * lock to protect concurrent access
 */
hash_t cluster_data_hash = NULL;
int cluster_data_hash_numnodes = 0;
int cluster_data_hash_size = 0;
pthread_mutex_t cluster_data_hash_lock = PTHREAD_MUTEX_INITIALIZER;

/* 
 * _cerebrod_listener_create_and_setup_socket
 *
 * Create and setup the listener socket.  Do not use wrappers in this
 * function.  We want to give the daemon additional chances to
 * "survive" an error condition.
 *
 * Returns file descriptor on success, -1 on error
 */
static int
_cerebrod_listener_create_and_setup_socket(void)
{
  struct sockaddr_in heartbeat_addr;
  int temp_fd;

  if ((temp_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): socket: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      return -1;
    }

  if (conf.multicast)
     {
      /* XXX: Probably lots of portability problems here */
      struct ip_mreqn imr;

      memset(&imr, '\0', sizeof(struct ip_mreqn));
      memcpy(&imr.imr_multiaddr,
             &conf.heartbeat_destination_ip_in_addr,
             sizeof(struct in_addr));
      memcpy(&imr.imr_address,
             &conf.heartbeat_network_interface_in_addr,
             sizeof(struct in_addr));
      imr.imr_ifindex = conf.heartbeat_interface_index;

      if (setsockopt(temp_fd,
		     SOL_IP,
		     IP_ADD_MEMBERSHIP,
		     &imr,
		     sizeof(struct ip_mreqn)) < 0)
	{
	  cerebro_err_debug("%s(%s:%d): setsockopt: %s",
                            __FILE__, __FUNCTION__, __LINE__,
                            strerror(errno));
	  return -1;
	}
    }

  /* Configuration checks ensure destination ip is on this machine if
   * it is a non-multicast address.
   */
  memset(&heartbeat_addr, '\0', sizeof(struct sockaddr_in));
  heartbeat_addr.sin_family = AF_INET;
  heartbeat_addr.sin_port = htons(conf.heartbeat_destination_port);
  memcpy(&heartbeat_addr.sin_addr,
         &conf.heartbeat_destination_ip_in_addr,
         sizeof(struct in_addr));
  if (bind(temp_fd, 
	   (struct sockaddr *)&heartbeat_addr, 
	   sizeof(struct sockaddr_in)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): bind: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        strerror(errno));
      return -1;
    }

  return temp_fd;
}

/* 
 * _cerebrod_listener_initialize
 *
 * perform listener initialization
 */
static void
_cerebrod_listener_initialize(void)
{
  Pthread_mutex_lock(&cerebrod_listener_initialization_complete_lock);
  if (cerebrod_listener_initialization_complete)
    goto out;

  Pthread_mutex_lock(&listener_fd_lock);
  if ((listener_fd = _cerebrod_listener_create_and_setup_socket()) < 0)
    cerebro_err_exit("%s(%s:%d): listener_fd setup failed",
                     __FILE__, __FUNCTION__, __LINE__);
  Pthread_mutex_unlock(&listener_fd_lock);
  
  /* Use the clusterlist numnodes count as the  initializing hash size */
  if ((cluster_data_hash_size = cerebro_clusterlist_module_numnodes()) < 0)
    cerebro_err_exit("%s(%s:%d): cerebro_clusterlist_module_numnodes",
		     __FILE__, __FUNCTION__, __LINE__);
  
  if (cluster_data_hash_size <= 0) 
    cluster_data_hash_size = CEREBROD_LISTENER_HASH_SIZE_DEFAULT;

  cluster_data_hash_numnodes = 0;
  cluster_data_hash = Hash_create(cluster_data_hash_size,
				  (hash_key_f)hash_key_string,
				  (hash_cmp_f)strcmp,
				  (hash_del_f)_Free);
  cerebrod_listener_initialization_complete++;
  Pthread_cond_signal(&cerebrod_listener_initialization_complete_cond);
 out:
  Pthread_mutex_unlock(&cerebrod_listener_initialization_complete_lock);
}

/*
 * _cerebrod_heartbeat_unmarshall
 *
 * unmarshall contents of a heartbeat packet buffer
 *
 * Returns length of data unmarshalled on success, -1 on error
 */
int
_cerebrod_heartbeat_unmarshall(struct cerebrod_heartbeat *hb,
			       const char *buf,
			       unsigned int buflen)
{
  int len, count = 0;

  assert(hb);
  assert(buf);
  
  memset(hb, '\0', sizeof(struct cerebrod_heartbeat));
  
  if ((len = cerebro_unmarshall_int32(&(hb->version),
				     buf + count,
				     buflen - count)) < 0)
    cerebro_err_exit("%s(%s:%d): cerebro_unmarshall_int32",
		     __FILE__, __FUNCTION__, __LINE__);
  if (!len)
    return count;
  
  count += len;

  if ((len = cerebro_unmarshall_buffer(hb->nodename,
				      sizeof(hb->nodename),
				      buf + count,
				      buflen - count)) < 0)
    cerebro_err_exit("%s(%s:%d): cerebro_unmarshall_buffer",
                     __FILE__, __FUNCTION__, __LINE__);
  if (!len)
    return count;
  
  count += len;
  
  if ((len = cerebro_unmarshall_uint32(&(hb->starttime),
				      buf + count,
				      buflen - count)) < 0)
    cerebro_err_exit("%s(%s:%d): cerebro_unmarshall_uint32",
                     __FILE__, __FUNCTION__, __LINE__);
  if (!len)
    return count;

  count += len;

  if ((len = cerebro_unmarshall_uint32(&(hb->boottime),
				      buf + count,
				      buflen - count)) < 0)
    cerebro_err_exit("%s(%s:%d): cerebro_unmarshall_uint32",
                     __FILE__, __FUNCTION__, __LINE__);
  if (!len)
    return count;

  count += len;

  return count;
}


/* 
 * _cerebrod_heartbeat_dump
 *
 * Dump contents of heartbeat packet
 */
static void
_cerebrod_heartbeat_dump(struct cerebrod_heartbeat *hb)
{
#if CEREBRO_DEBUG
  assert(hb);

  if (conf.debug && conf.listen_debug)
    {
      time_t t;
      struct tm tm;
      char strbuf[CEREBROD_STRING_BUFLEN];

      t = Time(NULL);
      Localtime_r(&t, &tm);
      strftime(strbuf, CEREBROD_STRING_BUFLEN, "%H:%M:%S", &tm);

      Pthread_mutex_lock(&debug_output_mutex);
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Received Heartbeat: %s\n", strbuf);     
      fprintf(stderr, "* -----------------------\n");
      cerebrod_heartbeat_dump(hb);
      fprintf(stderr, "**************************************\n");
      Pthread_mutex_unlock(&debug_output_mutex);
    }
#endif /* CEREBRO_DEBUG */
}

/* 
 * _cerebrod_listener_dump_cluster_node_data_item
 *
 * callback function from hash_for_each to dump cluster node data
 */
#if CEREBRO_DEBUG
static int
_cerebrod_listener_dump_cluster_node_data_item(void *data, 
					       const void *key, 
					       void *arg)
{
  struct cerebrod_node_data *nd;

  assert(data);
  assert(key);

  nd = (struct cerebrod_node_data *)data;

  Pthread_mutex_lock(&(nd->node_data_lock));
  fprintf(stderr, "* %s: starttime=%u boottime=%u last_received=%u\n", 
          (char *)key, nd->starttime, nd->boottime, nd->last_received);
  Pthread_mutex_unlock(&(nd->node_data_lock));

  return 1;
}
#endif /* CEREBRO_DEBUG */

/* 
 * _cerebrod_listener_dump_cluster_node_data_hash
 *
 * Dump contents of cluster node data hash
 */
static void
_cerebrod_listener_dump_cluster_node_data_hash(void)
{
#if CEREBRO_DEBUG
  if (conf.debug && conf.listen_debug)
    {
      int num;

      Pthread_mutex_lock(&cluster_data_hash_lock);
      Pthread_mutex_lock(&debug_output_mutex);
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Cluster Node Hash State\n");
      fprintf(stderr, "* -----------------------\n");
      fprintf(stderr, "* Hashed Nodes: %d\n", cluster_data_hash_numnodes);
      fprintf(stderr, "* -----------------------\n");
      if (cluster_data_hash_numnodes > 0)
        {          
          num = Hash_for_each(cluster_data_hash, 
			      _cerebrod_listener_dump_cluster_node_data_item,
			      NULL);
          if (num != cluster_data_hash_numnodes)
	    {
	      fprintf(stderr, "_cerebrod_listener_dump_cluster_node_data_hash: "
		      "invalid dump count: num=%d numnodes=%d",
		      num, cluster_data_hash_numnodes);
	      exit(1);
	    }
        }
      else
        fprintf(stderr, "_cerebrod_listener_dump_cluster_node_data_hash: "
		"called with empty hash\n");
      fprintf(stderr, "**************************************\n");
      Pthread_mutex_unlock(&debug_output_mutex);
      Pthread_mutex_unlock(&cluster_data_hash_lock);
    }
#endif /* CEREBRO_DEBUG */
}

void *
cerebrod_listener(void *arg)
{
  struct timeval tv;

  _cerebrod_listener_initialize();

  for (;;)
    {
      struct cerebrod_heartbeat hb;
      struct cerebrod_node_data *nd;
      char buf[CEREBRO_PACKET_BUFLEN];
      char nodename_buf[CEREBRO_MAXNODENAMELEN+1];
      char nodename_key[CEREBRO_MAXNODENAMELEN+1];
      int recv_len, heartbeat_len, flag, cluster_data_updated_flag = 0;
      
      Pthread_mutex_lock(&listener_fd_lock);
      if ((recv_len = recvfrom(listener_fd, 
			       buf, 
			       CEREBRO_PACKET_BUFLEN, 
			       0, 
			       NULL, 
			       NULL)) < 0)
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
	      || listener_fd < 0)
            {
              if (!(listener_fd < 0))
		close(listener_fd);	/* no-wrapper, make best effort */

              if ((listener_fd = _cerebrod_listener_create_and_setup_socket()) < 0)
		{
		  cerebro_err_debug("%s(%s:%d): error re-initializing socket",
                                    __FILE__, __FUNCTION__, __LINE__);
                  
		  /* Wait a bit, so we don't spin */
		  sleep(CEREBROD_LISTENER_REINITIALIZE_WAIT);
		}
              else
                cerebro_err_debug("%s(%s:%d): success re-initializing socket",
                                  __FILE__, __FUNCTION__, __LINE__);
            }
          else if (errno == EINTR)
            cerebro_err_debug("%s(%s:%d): recvfrom: %s", 
                              __FILE__, __FUNCTION__, __LINE__,
                              strerror(errno));
          else
            cerebro_err_exit("%s(%s:%d): recvfrom: %s", 
                             __FILE__, __FUNCTION__, __LINE__,
                             strerror(errno));
	}
      Pthread_mutex_unlock(&listener_fd_lock);

      /* No packet read */
      if (recv_len <= 0)
	continue;

      if ((heartbeat_len = _cerebrod_heartbeat_unmarshall(&hb, 
							  buf, 
							  recv_len)) < 0)
	continue;

      _cerebrod_heartbeat_dump(&hb);

      if (heartbeat_len != CEREBROD_HEARTBEAT_LEN)
        {
          cerebro_err_debug("%s(%s:%d): received buf length "
                            "unexpected size: expect %d, heartbeat_len %d",
                            __FILE__, __FUNCTION__, __LINE__,
                            CEREBROD_HEARTBEAT_LEN, heartbeat_len);
          continue;
        }

      if (hb.version != CEREBROD_HEARTBEAT_PROTOCOL_VERSION)
	{
	  cerebro_err_debug("%s(%s:%d): invalid cerebrod packet version read:"
                            "expect %d, version %d",
                            __FILE__, __FUNCTION__, __LINE__,
                            CEREBROD_HEARTBEAT_PROTOCOL_VERSION, hb.version);
	  continue;
	}
      
      if ((flag = cerebro_clusterlist_module_node_in_cluster(hb.nodename)) < 0)
	cerebro_err_exit("%s(%s:%d): cerebro_clusterlist_module_node_in_cluster: %s",
			 __FILE__, __FUNCTION__, __LINE__, hb.nodename);
      
      if (!flag)
	{
	  cerebro_err_debug("%s(%s:%d): received non-cluster packet from: %s",
			    __FILE__, __FUNCTION__, __LINE__,
			    hb.nodename);
	  continue;
	}
      
      /* Guarantee ending '\0' character */
      memset(nodename_buf, '\0', CEREBRO_MAXNODENAMELEN+1);
      memcpy(nodename_buf, hb.nodename, CEREBRO_MAXNODENAMELEN);

      memset(nodename_key, '\0', CEREBRO_MAXNODENAMELEN+1);

      if (cerebro_clusterlist_module_get_nodename(nodename_buf,
						  nodename_key, 
						  CEREBRO_MAXNODENAMELEN+1) < 0)
	{
	  cerebro_err_debug("%s(%s:%d): cerebro_clusterlist_module_get_nodename: %s",
			    __FILE__, __FUNCTION__, __LINE__,
			    hb.nodename);
	  continue;
	}

      Pthread_mutex_lock(&cluster_data_hash_lock);
      nd = Hash_find(cluster_data_hash, nodename_key);
      if (!nd)
        {
          char *key;

          key = Strdup(nodename_key);
          nd = (struct cerebrod_node_data *)Malloc(sizeof(struct cerebrod_node_data));
          Pthread_mutex_init(&(nd->node_data_lock), NULL);

          /* Re-hash if our hash is getting too small */
          if ((cluster_data_hash_numnodes + 1) > CEREBROD_LISTENER_REHASH_LIMIT)
            cerebrod_rehash(&cluster_data_hash, 
			    &cluster_data_hash_size,
			    CEREBROD_LISTENER_HASH_SIZE_INCREMENT,
			    cluster_data_hash_numnodes,
			    &cluster_data_hash_lock);

          Hash_insert(cluster_data_hash, key, nd);
          cluster_data_hash_numnodes++;
        }
      Pthread_mutex_unlock(&cluster_data_hash_lock);

      Pthread_mutex_lock(&(nd->node_data_lock));
      Gettimeofday(&tv, NULL);
      if (tv.tv_sec >= nd->last_received)
        {
          nd->starttime = hb.starttime;
          nd->boottime = hb.boottime;
          nd->last_received = tv.tv_sec;
	  cluster_data_updated_flag++;
        }
      Pthread_mutex_unlock(&(nd->node_data_lock));

      if (conf.updown_server && cluster_data_updated_flag)
	cerebrod_updown_update_data(nodename_key, nd->last_received);

      _cerebrod_listener_dump_cluster_node_data_hash();
    }

  return NULL;			/* NOT REACHED */
}
