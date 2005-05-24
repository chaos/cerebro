/*****************************************************************************\
 *  $Id: cerebro.h,v 1.21 2005-05-24 00:07:45 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_H
#define _CEREBRO_H

#define CEREBRO_ERR_SUCCESS                0
#define CEREBRO_ERR_NULLHANDLE             1
#define CEREBRO_ERR_MAGIC_NUMBER           2
#define CEREBRO_ERR_PARAMETERS             3
#define CEREBRO_ERR_HOSTNAME               4
#define CEREBRO_ERR_CONNECT                5
#define CEREBRO_ERR_CONNECT_TIMEOUT        6
#define CEREBRO_ERR_PROTOCOL               7
#define CEREBRO_ERR_PROTOCOL_TIMEOUT       8
#define CEREBRO_ERR_VERSION_INCOMPATIBLE   9
#define CEREBRO_ERR_NOT_LOADED            10
#define CEREBRO_ERR_OVERFLOW              11
#define CEREBRO_ERR_NODE_NOTFOUND         12
#define CEREBRO_ERR_CONFIG_FILE           13
#define CEREBRO_ERR_CONFIG_MODULE         14
#define CEREBRO_ERR_CONFIG_INPUT          15
#define CEREBRO_ERR_CLUSTERLIST_MODULE    16
#define CEREBRO_ERR_OUTMEM                17
#define CEREBRO_ERR_INTERNAL              18
#define CEREBRO_ERR_ERRNUMRANGE           19

#define CEREBRO_NODELIST_ERR_SUCCESS       0
#define CEREBRO_NODELIST_ERR_NULLNODELIST  1
#define CEREBRO_NODELIST_ERR_MAGIC_NUMBER  2
#define CEREBRO_NODELIST_ERR_PARAMETERS    3
#define CEREBRO_NODELIST_ERR_OUTMEM        4
#define CEREBRO_NODELIST_ERR_INTERNAL      5
#define CEREBRO_NODELIST_ERR_ERRNUMRANGE   6

#define CEREBRO_NODELIST_ITERATOR_ERR_SUCCESS       0
#define CEREBRO_NODELIST_ITERATOR_ERR_NULLITERATOR  1
#define CEREBRO_NODELIST_ITERATOR_ERR_MAGIC_NUMBER  2
#define CEREBRO_NODELIST_ITERATOR_ERR_PARAMETERS    3
#define CEREBRO_NODELIST_ITERATOR_ERR_OUTMEM        4
#define CEREBRO_NODELIST_ITERATOR_ERR_INTERNAL      5
#define CEREBRO_NODELIST_ITERATOR_ERR_ERRNUMRANGE   6

typedef struct cerebro *cerebro_t;

typedef struct cerebro_nodelist *cerebro_nodelist_t;

typedef struct cerebro_nodelist_iterator *cerebro_nodelist_iterator_t;

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
 * Nodelist API
 *
 */

/* 
 * cerebro_nodelist_count
 *
 * Determine the length of the nodelist.
 *
 * Returns the length of the nodelist on success, -1 on error
 */
int cerebro_nodelist_count(cerebro_nodelist_t nodelist);

/* 
 * cerebro_nodelist_find
 *
 * Determine if 'node' exists in the list.
 *
 * Returns 1 if 'node' is found, 0 if not, -1 on error
 */
int cerebro_nodelist_find(cerebro_nodelist_t nodelist, const char *node);

/* 
 * Cerebro_for_each
 *
 * Function prototype for operating on each node stored in
 * a cerebro_nodelist_t.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_for_each)(char *nodename, void *arg);

/* 
 * cerebro_nodelist_for_each
 *
 * For each node in 'nodelist' invoke 'for_each', passing 'arg'.
 *
 * Return 0 on success, -1 on error
 */
int cerebro_nodelist_for_each(cerebro_nodelist_t nodelist,
                              Cerebro_for_each for_each,
                              void *arg);

/* 
 * cerebro_nodelist_destroy
 *
 * Destroy a nodelist
 *
 * Return 0 on success, -1 on error
 */
int cerebro_nodelist_destroy(cerebro_nodelist_t nodelist);

/*
 * cerebro_nodelist_errnum
 *
 * Return the most recent error number
 *
 * Returns error number on success
 */
int cerebro_nodelist_errnum(cerebro_nodelist_t nodelist);

/*
 * cerebro_nodelist_strerror
 *
 * Return a string message describing an error number
 *
 * Returns pointer to message on success
 */
char *cerebro_nodelist_strerror(int errnum);

/*
 * cerebro_nodelist_errormsg
 *
 * Return a string message describing the most recent error
 *
 * Returns pointer to message on success
 */
char *cerebro_nodelist_errormsg(cerebro_nodelist_t nodelist);

/*
 * cerebro_nodelist_perror
 *
 * Output a message to standard error
 */
void cerebro_nodelist_perror(cerebro_nodelist_t nodelist, const char *msg);

/* 
 * cerebro_nodelist_iterator_create
 *
 * Create a nodelist iterator
 *
 * Return iterator on success, NULL on error
 */
cerebro_nodelist_iterator_t cerebro_nodelist_iterator_create(cerebro_nodelist_t itr);

/* 
 * cerebro_nodelist_iterator_next
 *
 * Retrieve next node from iterator and move the nodename pointer forward
 *
 * Return 1 if node returned, 0 at end of list, -1 on error
 */
int cerebro_nodelist_iterator_next(cerebro_nodelist_iterator_t itr, char **node);

/* 
 * cerebro_nodelist_iterator_peek
 *
 * Retrieve next node from iterator, but do not move the nodename
 * pointer forward
 *
 * Return 1 if node returned, 0 at end of list, -1 on error
 */
int cerebro_nodelist_iterator_peek(cerebro_nodelist_iterator_t itr, char **node);

/* 
 * cerebro_nodelist_iterator_reset
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_nodes_iterator_reset(cerebro_nodelist_iterator_t itr);

/* 
 * cerebro_nodelist_iterator_destroy
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_nodes_iterator_destroy(cerebro_nodelist_iterator_t itr);

/*
 * cerebro_nodelist_iterator_errnum
 *
 * Return the most recent error number
 *
 * Returns error number on success
 */
int cerebro_nodelist_iterator_errnum(cerebro_nodelist_iterator_t handle);

/*
 * cerebro_nodelist_iterator_strerror
 *
 * Return a string message describing an error number
 *
 * Returns pointer to message on success
 */
char *cerebro_nodelist_iterator_strerror(int errnum);

/*
 * cerebro_nodelist_iterator_errormsg
 *
 * Return a string message describing the most recent error
 *
 * Returns pointer to message on success
 */
char *cerebro_nodelist_iterator_errormsg(cerebro_nodelist_iterator_t handle);

/*
 * cerebro_nodelist_iterator_perror
 *
 * Output a message to standard error
 */
void cerebro_nodelist_iterator_perror(cerebro_nodelist_iterator_t handle, const char *msg);

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
 * cerebro_updown_get_up_nodes_string
 *
 * Retrieve a ranged string of up nodes and store it in the buffer.
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_updown_get_up_nodes_string(cerebro_t handle, char *buf, unsigned int buflen);

/*
 * cerebro_updown_get_down_nodes_string
 *
 * Retrieve a ranged string of down nodes and store it in the buffer
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_updown_get_down_nodes_string(cerebro_t handle, char *buf, unsigned int buflen);

#if 0
/*
 * cerebro_updown_get_up_nodes_iterator
 *
 * Retrieve a cerebro nodes iterator for all up nodes
 *
 * Returns an iterator on success, NULL on error
 */
cerebro_nodes_iterator_t cerebro_updown_get_up_nodes_iterator(cerebro_t handle);

/*
 * cerebro_updown_get_down_nodes_iterator
 *
 * Retrieve a cerebro nodes iterator for all down nodes
 *
 * Returns an iterator on success, NULL on error
 */
cerebro_nodes_iterator_t cerebro_updown_get_down_nodes_iterator(cerebro_t handle);

#endif /* 0 */

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
