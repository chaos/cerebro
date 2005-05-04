/*****************************************************************************\
 *  $Id: cerebro.h,v 1.12 2005-05-04 20:58:08 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_H
#define _CEREBRO_H

#define CEREBRO_ERR_SUCCESS                0
#define CEREBRO_ERR_NULLHANDLE             1
#define CEREBRO_ERR_MAGIC_NUMBER           2
#define CEREBRO_ERR_PARAMETERS             3
#define CEREBRO_ERR_NOT_LOADED             4
#define CEREBRO_ERR_HOSTNAME               5
#define CEREBRO_ERR_ADDRESS                6
#define CEREBRO_ERR_CONNECT                7
#define CEREBRO_ERR_CONNECT_TIMEOUT        8
#define CEREBRO_ERR_PROTOCOL               9
#define CEREBRO_ERR_PROTOCOL_TIMEOUT      10 
#define CEREBRO_ERR_VERSION_INCOMPATIBLE  11
#define CEREBRO_ERR_OVERFLOW              12
#define CEREBRO_ERR_NODE_NOTFOUND         13
#define CEREBRO_ERR_CONFIG_FILE           14
#define CEREBRO_ERR_CONFIG_MODULE         15
#define CEREBRO_ERR_CONFIG_INPUT          16
#define CEREBRO_ERR_CLUSTERLIST_MODULE    17
#define CEREBRO_ERR_OUTMEM                18 
#define CEREBRO_ERR_INTERNAL              19
#define CEREBRO_ERR_ERRNUMRANGE           20

typedef struct cerebro *cerebro_t;

/*
 * cerebro_handle_create
 *
 * Create a cerebro handle
 *
 * Returns handle on success, NULL on error
 */
cerebro_t cerebro_handle_create(void);

/* cerebro_handle_destroy
 *
 * Destroy a cerebro handle
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_handle_destroy(cerebro_t handle);

/*
 * cerebro_errnum
 *
 * Return the most recent error number
 *
 * Returns error number on success
 */
int cerebro_errnum(cerebro_t handle);

/*
 * cerebro_strerror
 *
 * Return a string message describing an error number
 *
 * Returns pointer to message on success
 */
char *cerebro_strerror(int errnum);

/*
 * cerebro_errormsg
 *
 * Return a string message describing the most recent error
 *
 * Returns pointer to message on success
 */
char *cerebro_errormsg(cerebro_t handle);

/*
 * cerebro_perror
 *
 * Output a message to standard error
 */
void cerebro_perror(cerebro_t handle, const char *msg);

/* 
 * cerebro_load_config
 *
 * Read and load config file data
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_load_config(cerebro_t handle);

/* 
 * cerebro_api_load_config
 *
 * Unload config file info
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_unload_config(cerebro_t handle);

/* 
 * cerebro_load_clusterlist_module
 *
 * Find and load clusterlist module
 *
 * Returns 1 if clusterlist module was loaded, 0 if not, -1 on fatal error
 */
int cerebro_load_clusterlist_module(cerebro_t handle);

/* 
 * cerebro_unload_clusterlist_module
 *
 * Unload clusterlist module
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_unload_clusterlist_module(cerebro_t handle);

/* 
 * Up-Down API
 */

#define CEREBRO_UPDOWN_UP_NODES           0x0001
#define CEREBRO_UPDOWN_DOWN_NODES         0x0002
#define CEREBRO_UPDOWN_UP_AND_DOWN_NODES  0x0003

/* 
 * cerebro_updown_load_data
 *
 * Retrieve all updown data from a cerebro updown server and locally
 * cache the data in the cerebro_t handle for use by the remainder of
 * the cerebro updown library.  If updown data has been previously
 * retrieved, the latest data will be retrieved and updated in the
 * local cache.
 *
 * hostname - server to connect to, if NULL defaults to localhost
 *
 * port - port to connect to, if 0 defaults to default port
 *
 * timeout_len - timeout length to use to evaluate up vs. down, if 0
 * defaults to default timeout length
 *
 * flags - indicate the data to be loaded.  Possible flags:
 *
 *   CEREBRO_UPDOWN_UP_NODES
 *   CEREBRO_UPDOWN_DOWN_NODES
 *   CEREBRO_UPDOWN_UP_AND_DOWN_NODES
 * 
 * if 0 defaults to CEREBRO_UPDOWN_UP_AND_DOWN_NODES
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_updown_load_data(cerebro_t handle, 
                             const char *hostname, 
                             unsigned int port, 
                             unsigned int timeout_len,
                             int flags);

/* 
 * cerebro_updown_unload_data
 *
 * Cleanup allocated updown data
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_updown_unload_data(cerebro_t handle);

/*
 * cerebro_updown_get_up_nodes
 *
 * Retrieve a ranged string of up nodes and store it in the buffer.
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_updown_get_up_nodes(cerebro_t handle, char *buf, unsigned int buflen);

/*
 * cerebro_updown_get_down_nodes
 *
 * Retrieve a ranged string of down nodes and store it in the buffer
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_updown_get_down_nodes(cerebro_t handle, char *buf, unsigned int buflen);

/*
 * cerebro_updown_is_node_up
 *
 * Check if a node is up.
 *
 * Returns 1 if up, 0 if down, -1 on error
 */
int cerebro_updown_is_node_up(cerebro_t handle, const char *node);

/*
 * cerebro_updown_is_node_down
 *
 * Check if a node is down.
 *
 * Returns 1 if down, 0 if up, -1 on error
 */
int cerebro_updown_is_node_down(cerebro_t handle, const char *node);

/*
 * cerebro_updown_up_count
 *
 * Determine the number of up nodes.
 *
 * Returns up count on success, -1 on error
 */
int cerebro_updown_up_count(cerebro_t handle);

/*
 * cerebro_updown_down_count
 *
 * Determine the number of down nodes. 
 *
 * Returns down count on success, -1 on error
 */
int cerebro_updown_down_count(cerebro_t handle);

#endif /* _CEREBRO_H */
