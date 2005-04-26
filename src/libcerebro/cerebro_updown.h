/*****************************************************************************\
 *  $Id: cerebro_updown.h,v 1.3 2005-04-26 00:09:13 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_UPDOWN_H
#define _CEREBRO_UPDOWN_H

/* 
 * cerebro_updown_load_data
 *
 * Retrieve all updown data from a cerebro updown server and locally
 * cache the data in the cerebro_t handle for use by the remainder of
 * the cerebro updown library.  If updown data has been previously
 * retrieved, the latest data will be retrieved and updated in the
 * local cache.
 *
 * hostname - server to connect to
 * port - port to connect to
 * timeout_len - timeout length to use to evaluate up vs. down
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_updown_load_data(cerebro_t handle, 
                             const char *hostname, 
                             unsigned int port, 
                             unsigned int timeout_len);

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

#endif /* _CEREBRO_UPDOWN_H */
