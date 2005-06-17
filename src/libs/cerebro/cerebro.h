/*****************************************************************************\
 *  $Id: cerebro.h,v 1.1 2005-06-17 20:54:08 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_H
#define _CEREBRO_H

/* 
 * Cerebro Error Codes
 */
#define CEREBRO_ERR_SUCCESS                   0
#define CEREBRO_ERR_NULLHANDLE                1
#define CEREBRO_ERR_NULLNODELIST              2
#define CEREBRO_ERR_NULLITERATOR              3
#define CEREBRO_ERR_MAGIC_NUMBER              4
#define CEREBRO_ERR_PARAMETERS                5
#define CEREBRO_ERR_HOSTNAME                  6
#define CEREBRO_ERR_CONNECT                   7
#define CEREBRO_ERR_CONNECT_TIMEOUT           8
#define CEREBRO_ERR_PROTOCOL                  9
#define CEREBRO_ERR_PROTOCOL_TIMEOUT         10 
#define CEREBRO_ERR_VERSION_INCOMPATIBLE     11 
#define CEREBRO_ERR_OVERFLOW                 12
#define CEREBRO_ERR_NODE_NOTFOUND            13
#define CEREBRO_ERR_METRIC_UNKNOWN           14
#define CEREBRO_ERR_END_OF_LIST              15
#define CEREBRO_ERR_CONFIG_FILE              16
#define CEREBRO_ERR_CONFIG_MODULE            17
#define CEREBRO_ERR_CONFIG_INPUT             18
#define CEREBRO_ERR_CLUSTERLIST_MODULE       19
#define CEREBRO_ERR_OUTMEM                   20
#define CEREBRO_ERR_INTERNAL                 21
#define CEREBRO_ERR_ERRNUMRANGE              22

/* 
 * Cerebro Flags
 */

#define CEREBRO_METRIC_FLAGS_UP_ONLY              0x00000001
#define CEREBRO_METRIC_FLAGS_NONE_IF_NOEXIST      0x00000002
#define CEREBRO_METRIC_FLAGS_NONE_IF_DOWN         0x00000004
#define CEREBRO_METRIC_FLAGS_MASK                 0x00000007

/* 
 * Default available metrics
 */
#define CEREBRO_METRIC_CLUSTER_NODES              "cluster_nodes"
#define CEREBRO_METRIC_UP_NODES                   "up_nodes"
#define CEREBRO_METRIC_DOWN_NODES                 "down_nodes"
#define CEREBRO_METRIC_UPDOWN_STATE               "updown_state"
#define CEREBRO_METRIC_LAST_RECEIVED_TIME         "last_received_time"

#define CEREBRO_METRIC_UPDOWN_STATE_NODE_UP       1
#define CEREBRO_METRIC_UPDOWN_STATE_NODE_DOWN     0

/* 
 * Cerebro Metric Value Types
 */
#define CEREBRO_METRIC_VALUE_TYPE_NONE     0
#define CEREBRO_METRIC_VALUE_TYPE_INT32    1
#define CEREBRO_METRIC_VALUE_TYPE_U_INT32  2
#define CEREBRO_METRIC_VALUE_TYPE_FLOAT    3
#define CEREBRO_METRIC_VALUE_TYPE_DOUBLE   4
#define CEREBRO_METRIC_VALUE_TYPE_STRING   5
#define CEREBRO_METRIC_VALUE_TYPE_RAW      6

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
 * Return the most recent error number from a cerebro_t handle
 *
 * Returns error number on success
 */
int cerebro_errnum(cerebro_t handle);

/*
 * cerebro_nodelist_errnum
 *
 * Return the most recent error number from a cerebro_nodelist_t
 * nodelist
 *
 * Returns error number on success
 */
int cerebro_nodelist_errnum(cerebro_nodelist_t nodelist);

/*
 * cerebro_nodelist_iterator_errnum
 *
 * Return the most recent error number from a
 * cerebro_nodelist_iterator_t iterator
 *
 * Returns error number on success
 */
int cerebro_nodelist_iterator_errnum(cerebro_nodelist_iterator_t nodelistItr);

/*
 * cerebro_strerror
 *
 * Return a string message describing an error number
 *
 * Returns pointer to message on success
 */
char *cerebro_strerror(int errnum);

/* 
 * Parameter Get/Set Routines 
 */

/* 
 * cerebro_get_hostname
 *
 * Get a string pointer to the currently set hostname of the cerebrod
 * server.  The string returned may be the empty string, indicating no
 * hostname has been set.
 * 
 * Returns pointer on success, NULL on error
 */
char *cerebro_get_hostname(cerebro_t handle);

/* 
 * cerebro_set_hostname
 *
 * Set the hostname of the cerebrod server
 * 
 * Returns 0 on success, -1 on error
 */
int cerebro_set_hostname(cerebro_t handle, const char *hostname);

/* 
 * cerebro_get_port
 *
 * Get the currently set cerebrod server port.  The port may be 0,
 * indicating no port has been set.
 *
 * Returns port on success, -1 on error
 */
int cerebro_get_port(cerebro_t handle);

/* 
 * cerebro_set_port
 *
 * Set the cerebrod server port to connect to
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_set_port(cerebro_t handle, unsigned int port);

/* 
 * cerebro_get_timeout_len
 *
 * Get the currently set timeout_len.  The timeout_len may be 0,
 * indicating no timeout_len has been set.
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_get_timeout_len(cerebro_t handle);

/* 
 * cerebro_set_timeout_len
 *
 * Set the cerebrod server timeout_len for up vs. down calculation
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_set_timeout_len(cerebro_t handle, unsigned int timeout_len);

/* 
 * cerebro_get_flags
 *
 * Get the currently set flags.  The flags may be 0, indicating no
 * flags have been set.
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_get_flags(cerebro_t handle);

/* 
 * cerebro_set_flags
 *
 * Set the cerebrod server flags to use
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_set_flags(cerebro_t handle, unsigned int flags);

/* 
 * Core API
 */

/* 
 * cerebro_get_metric_data
 *
 * Get nodelist of nodes and values for a certain metric
 *
 * Returns nodelist on success, -1 on error
 */
cerebro_nodelist_t cerebro_get_metric_data(cerebro_t handle,
					   const char *metric_name);

/* 
 * Nodelist API
 *
 */

/* 
 * cerebro_nodelist_metric_name
 *
 * Determine the name of the data stored in the nodelist
 *
 * Returns name on success, NULL on error
 */
char *cerebro_nodelist_metric_name(cerebro_nodelist_t nodelist);

/* 
 * cerebro_nodelist_length
 *
 * Determine the length of the nodelist.
 *
 * Returns the length of the nodelist on success, -1 on error
 */
int cerebro_nodelist_length(cerebro_nodelist_t nodelist);

/* 
 * cerebro_nodelist_find
 *
 * Determine if 'node' exists in the list.  If a value exists for the
 * node, the metric type, metric value length, and metric value are
 * returned in 'metric_value_type', 'metric_value_len', and 'metric_value'
 * respectively.
 *
 * Returns 1 if 'node' is found, 0 if not, -1 on error
 */
int cerebro_nodelist_find(cerebro_nodelist_t nodelist, 
			  const char *node, 
                          unsigned int *metric_value_type,
                          unsigned int *metric_value_len,
                          void **metric_value);

/* 
 * Cerebro_for_each
 *
 * Function prototype for operating on each node and metric value
 * stored in a nodelist.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_for_each)(char *nodename, 
                                unsigned int metric_value_type,
                                unsigned int metric_value_len,
                                void *metric_value,
                                void *arg);

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
 * cerebro_nodelist_iterator_create
 *
 * Create a nodelist iterator
 *
 * Return iterator on success, NULL on error
 */
cerebro_nodelist_iterator_t cerebro_nodelist_iterator_create(cerebro_nodelist_t nodelist);

/* 
 * cerebro_nodelist_iterator_nodename
 *
 * Retrieve a pointer to the current nodename
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_nodelist_iterator_nodename(cerebro_nodelist_iterator_t nodelistItr, 
                                       char **nodename);

/* 
 * cerebro_nodelist_iterator_metric_value
 *
 * If a value exists for the current point in the list, the metric
 * type, metric value length, and metric value are returned in
 * 'metric_value_type', 'metric_value_len', and 'metric_value' respectively.
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_nodelist_iterator_metric_value(cerebro_nodelist_iterator_t nodelistItr,
                                           unsigned int *metric_value_type,
                                           unsigned int *metric_value_len,
                                           void **metric_value);

/* 
 * cerebro_nodelist_iterator_next
 *
 * Move the iterator pointer forward
 *
 * Return 1 if more data exists, 0 if the end of list has been
 * reached, -1 on error
 */
int cerebro_nodelist_iterator_next(cerebro_nodelist_iterator_t nodelistItr);

/* 
 * cerebro_nodelist_iterator_reset
 *
 * Reset the nodelist iterator to the beginning.
 * 
 * Returns 0 on success, -1 on error
 */
int cerebro_nodelist_iterator_reset(cerebro_nodelist_iterator_t nodelistItr);

/* 
 * cerebro_nodelist_iterator_at_end
 *
 * Returns 1 if the end of the list has been reached, 0 if not, -1 on
 * error
 */
int cerebro_nodelist_iterator_at_end(cerebro_nodelist_iterator_t nodelistItr);

/* 
 * cerebro_nodelist_iterator_destroy
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_nodelist_iterator_destroy(cerebro_nodelist_iterator_t nodelistItr);

#endif /* _CEREBRO_H */
