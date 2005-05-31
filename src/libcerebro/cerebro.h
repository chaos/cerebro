/*****************************************************************************\
 *  $Id: cerebro.h,v 1.33 2005-05-31 20:45:56 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_H
#define _CEREBRO_H

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
#define CEREBRO_ERR_NOT_LOADED               12
#define CEREBRO_ERR_OVERFLOW                 13
#define CEREBRO_ERR_NODE_NOTFOUND            14
#define CEREBRO_ERR_END_OF_LIST              15
#define CEREBRO_ERR_METRIC_VALUE_NOTFOUND    16
#define CEREBRO_ERR_METRIC_TYPE_INCONSISTENT 17
#define CEREBRO_ERR_CONFIG_FILE              18
#define CEREBRO_ERR_CONFIG_MODULE            19
#define CEREBRO_ERR_CONFIG_INPUT             20
#define CEREBRO_ERR_CLUSTERLIST_MODULE       21
#define CEREBRO_ERR_OUTMEM                   22
#define CEREBRO_ERR_INTERNAL                 23
#define CEREBRO_ERR_ERRNUMRANGE              24

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
 * Parameter Settings
 */

/* 
 * cerebro_set_hostname
 *
 * Set the hostname of the cerebrod server
 * 
 * Returns 0 on success, -1 on error
 */
int cerebro_set_hostname(cerebro_t handle, const char *hostname);

/* 
 * cerebro_set_port
 *
 * Set the cerebrod server port to connect to
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_set_port(cerebro_t handle, unsigned int port);

/* 
 * cerebro_set_timeout_len
 *
 * Set the cerebrod server timeout_len for up vs. down calculation
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_set_timeout_len(cerebro_t handle, unsigned int timeout_len);

/* 
 * cerebro_set_flags
 *
 * Set the cerebrod server flags to use
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_set_flags(cerebro_t handle, unsigned int flags);

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
 * cerebro_nodelist_length
 *
 * Determine the length of the nodelist.
 *
 * Returns the length of the nodelist on success, -1 on error
 */
int cerebro_nodelist_length(cerebro_nodelist_t nodelist);

/* 
 * cerebro_nodelist_metric_name
 *
 * Determine the name of the data stored in the nodelist
 *
 * Returns name on success, NULL on error
 */
char *cerebro_nodelist_metric_name(cerebro_nodelist_t nodelist);

/* 
 * cerebro_nodelist_metric_type
 *
 * Determine the type of the data stored in the nodelist
 *
 * Returns type on success, -1 on error
 */
int cerebro_nodelist_metric_type(cerebro_nodelist_t nodelist);

/* 
 * cerebro_nodelist_find
 *
 * Determine if 'node' exists in the list.  If a value exists
 * for the node, it is returned in 'metric_value'.
 *
 * Returns 1 if 'node' is found and the length of the data returned in
 * 'metric_value_size', 0 if not, -1 on error
 */
int cerebro_nodelist_find(cerebro_nodelist_t nodelist, 
			  const char *node, 
			  void **metric_value,
                          unsigned int *metric_value_size);

/* 
 * Cerebro_for_each
 *
 * Function prototype for operating on each node stored in
 * a cerebro_nodelist_t.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_for_each)(char *nodename, void *metric_value, void *arg);

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
 * cerebro_nodelist_iterator_create
 *
 * Create a nodelist iterator
 *
 * Return iterator on success, NULL on error
 */
cerebro_nodelist_iterator_t cerebro_nodelist_iterator_create(cerebro_nodelist_t nodelist);

/* 
 * cerebro_nodelist_iterator_end
 *
 * Returns 1 if the end of the list has been reached, 0 if not
 */
int cerebro_nodelist_iterator_end(cerebro_nodelist_iterator_t nodelistItr);

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
 * cerebro_nodelist_iterator_nodename
 *
 * Retrieve a pointer to the current nodename
 *
 * Returns pointer to nodename, NULL on error
 */
char *cerebro_nodelist_iterator_nodename(cerebro_nodelist_iterator_t nodelistItr);

/* 
 * cerebro_nodelist_iterator_metric_value
 *
 * Retrieve a pointer to the current value.
 *
 * Returns pointer to value, NULL on error
 */
void *cerebro_nodelist_iterator_metric_value(cerebro_nodelist_iterator_t nodelistItr,
                                             unsigned int *metric_value_size);

/* 
 * cerebro_nodelist_iterator_destroy
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_nodelist_iterator_destroy(cerebro_nodelist_iterator_t nodelistItr);

/*
 * cerebro_nodelist_iterator_errnum
 *
 * Return the most recent error number
 *
 * Returns error number on success
 */
int cerebro_nodelist_iterator_errnum(cerebro_nodelist_iterator_t handle);

#endif /* _CEREBRO_H */
