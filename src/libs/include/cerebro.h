/*****************************************************************************\
 *  $Id: cerebro.h,v 1.32 2010-02-02 01:01:20 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2018 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2005-2007 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <https://github.com/chaos/cerebro>.
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
 *  with Cerebro. If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#ifndef _CEREBRO_H
#define _CEREBRO_H

/*
 * Cerebro Error Codes
 */
#define CEREBRO_ERR_SUCCESS                   0
#define CEREBRO_ERR_NULLHANDLE                1
#define CEREBRO_ERR_NULLNAMELIST              2
#define CEREBRO_ERR_NULLNAMELIST_ITERATOR     3
#define CEREBRO_ERR_NULLNODELIST              4
#define CEREBRO_ERR_NULLNODELIST_ITERATOR     5
#define CEREBRO_ERR_MAGIC_NUMBER              6
#define CEREBRO_ERR_PARAMETERS                7
#define CEREBRO_ERR_HOSTNAME                  8
#define CEREBRO_ERR_CONNECT                   9
#define CEREBRO_ERR_CONNECT_TIMEOUT          10
#define CEREBRO_ERR_PROTOCOL                 11
#define CEREBRO_ERR_PROTOCOL_TIMEOUT         12
#define CEREBRO_ERR_VERSION_INCOMPATIBLE     13
#define CEREBRO_ERR_OVERFLOW                 14
#define CEREBRO_ERR_NODE_NOTFOUND            15
#define CEREBRO_ERR_METRIC_INVALID           16
#define CEREBRO_ERR_EVENT_INVALID            17
#define CEREBRO_ERR_METRIC_MAX               18
#define CEREBRO_ERR_END_OF_LIST              19
#define CEREBRO_ERR_EVENT_NOT_RECEIVED       20
#define CEREBRO_ERR_CONFIG_FILE              21
#define CEREBRO_ERR_CONFIG_MODULE            22
#define CEREBRO_ERR_CONFIG_INPUT             23
#define CEREBRO_ERR_OUTMEM                   24
#define CEREBRO_ERR_INTERNAL                 25
#define CEREBRO_ERR_ERRNUMRANGE              26

/*
 * Cerebro Flags
 */

#define CEREBRO_METRIC_DATA_FLAGS_UP_ONLY               0x00000001
#define CEREBRO_METRIC_DATA_FLAGS_NONE_IF_DOWN          0x00000002
#define CEREBRO_METRIC_DATA_FLAGS_NONE_IF_NOT_MONITORED 0x00000004
#define CEREBRO_METRIC_CONTROL_FLAGS_SEND_NOW           0x00000008
#define CEREBRO_METRIC_FLAGS_MASK                       0x0000000F

/*
 * Cerebro Data Value Types
 */
#define CEREBRO_DATA_VALUE_TYPE_NONE             0
#define CEREBRO_DATA_VALUE_TYPE_INT32            1
#define CEREBRO_DATA_VALUE_TYPE_U_INT32          2
#define CEREBRO_DATA_VALUE_TYPE_FLOAT            3
#define CEREBRO_DATA_VALUE_TYPE_DOUBLE           4
#define CEREBRO_DATA_VALUE_TYPE_STRING           5
#define CEREBRO_DATA_VALUE_TYPE_INT64            6
#define CEREBRO_DATA_VALUE_TYPE_U_INT64          7

/*
 * Default metrics
 */
#define CEREBRO_METRIC_METRIC_NAMES                "metric_names"
#define CEREBRO_METRIC_CLUSTER_NODES               "cluster_nodes"
#define CEREBRO_METRIC_UPDOWN_STATE                "updown_state"

#define CEREBRO_METRIC_UPDOWN_STATE_NODE_UP        1
#define CEREBRO_METRIC_UPDOWN_STATE_NODE_DOWN      0

/*
 * Default events
 */
#define CEREBRO_EVENT_NAMES                        "event_names"

/*
 * Server defaults
 */
#define CEREBRO_METRIC_SERVER_PORT                 8852
#define CEREBRO_METRIC_SERVER_TIMEOUT_LEN_DEFAULT  60
#define CEREBRO_METRIC_SERVER_FLAGS_DEFAULT        0

#define CEREBRO_EVENT_SERVER_PORT                  8853

typedef struct cerebro *cerebro_t;

typedef struct cerebro_namelist *cerebro_namelist_t;

typedef struct cerebro_namelist_iterator *cerebro_namelist_iterator_t;

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
 * cerebro_namelist_errnum
 *
 * Return the most recent error number from a cerebro_namelist_t
 * namelist
 *
 * Returns error number on success
 */
int cerebro_namelist_errnum(cerebro_namelist_t namelist);

/*
 * cerebro_namelist_iterator_errnum
 *
 * Return the most recent error number from a
 * cerebro_namelist_iterator_t iterator
 *
 * Returns error number on success
 */
int cerebro_namelist_iterator_errnum(cerebro_namelist_iterator_t namelistItr);

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
 * Event Retrieval API
 */

/*
 * cerebro_get_event_names
 *
 * Get a namelist of currently available events
 *
 * Returns namelist on success, -1 on error
 */
cerebro_namelist_t cerebro_get_event_names(cerebro_t handle);

/*
 * cerebro_event_register
 *
 * Setup a file descriptor for event polling.
 *
 * Returns a file descriptor for polling on success, -1 on error
 */
int cerebro_event_register(cerebro_t handle, const char *event_name);

/*
 * cerebro_event_unregister
 *
 * Tear down event file descriptor
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_event_unregister(cerebro_t handle, int fd);

/*
 * cerebro_event_parse
 *
 * Parse event data when there is data to read on the file descriptor.
 * User is responsible for freeing nodename and event_value memory.
 *
 * Returns 0 and event data on success, -1 on error
 */
int cerebro_event_parse(cerebro_t handle,
                        int fd,
                        char **nodename,
                        unsigned int *event_value_type,
                        unsigned int *event_value_len,
                        void **event_value);

/*
 * Metric Retrieval API
 */

/*
 * cerebro_get_metric_names
 *
 * Get a namelist of currently available metrics
 *
 * Returns namelist on success, -1 on error
 */
cerebro_namelist_t cerebro_get_metric_names(cerebro_t handle);

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
 * Metric Monitoring API
 */

/*
 * cerebro_register_metric
 *
 * Register a new metric with the local cerebro daemon
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_register_metric(cerebro_t handle, const char *metric_name);

/*
 * cerebro_unregister_metric
 *
 * Unregister a metric with the local cerebro daemon
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_unregister_metric(cerebro_t handle, const char *metric_name);

/*
 * cerebro_update_metric_value
 *
 * Update the value of a metric on the local cerebro daemon
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_update_metric_value(cerebro_t handle,
				const char *metric_name,
                                unsigned int metric_value_type,
                                unsigned int metric_value_len,
                                void *metric_value);

/*
 * cerebro_resend_metric
 *
 * Resend a metric
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_resend_metric(cerebro_t handle, const char *metric_name);

/*
 * cerebro_flush_metric
 *
 * Flush a metric
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_flush_metric(cerebro_t handle, const char *metric_name);

/*
 * Namelist API
 *
 */

/*
 * cerebro_namelist_length
 *
 * Determine the length of the namelist.
 *
 * Returns the length of the namelist on success, -1 on error
 */
int cerebro_namelist_length(cerebro_namelist_t namelist);

/*
 * cerebro_namelist_destroy
 *
 * Destroy a namelist
 *
 * Return 0 on success, -1 on error
 */
int cerebro_namelist_destroy(cerebro_namelist_t namelist);

/*
 * cerebro_namelist_iterator_create
 *
 * Create a namelist iterator
 *
 * Return iterator on success, NULL on error
 */
cerebro_namelist_iterator_t cerebro_namelist_iterator_create(cerebro_namelist_t namelist);

/*
 * cerebro_namelist_iterator_name
 *
 * Retrieve a pointer to the current name
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_namelist_iterator_name(cerebro_namelist_iterator_t namelistItr,
                                   char **name);

/*
 * cerebro_namelist_iterator_next
 *
 * Move the iterator pointer forward
 *
 * Return 1 if more data exists, 0 if the end of list has been
 * reached, -1 on error
 */
int cerebro_namelist_iterator_next(cerebro_namelist_iterator_t namelistItr);

/*
 * cerebro_namelist_iterator_reset
 *
 * Reset the namelist iterator to the beginning.
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_namelist_iterator_reset(cerebro_namelist_iterator_t namelistItr);

/*
 * cerebro_namelist_iterator_at_end
 *
 * Returns 1 if the end of the list has been reached, 0 if not, -1 on
 * error
 */
int cerebro_namelist_iterator_at_end(cerebro_namelist_iterator_t namelistItr);

/*
 * cerebro_namelist_iterator_destroy
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_namelist_iterator_destroy(cerebro_namelist_iterator_t namelistItr);

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
                                           unsigned int *metric_value_received_time,
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
