/*****************************************************************************\
 *  $Id: cerebro_util.h,v 1.9 2005-05-11 16:38:12 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_UTIL_H
#define _CEREBRO_UTIL_H

#include "cerebro_api.h"

/* 
 * _cerebro_handle_check
 *
 * Checks for a proper cerebro handle, setting the errnum
 * appropriately if an error is found.
 *
 * Returns 0 on succss, -1 on error
 */
int _cerebro_handle_check(cerebro_t handle);

/*
 * _cerebro_low_timeout_connect
 *
 * Setup a tcp connection to 'hostname' and 'port' using a connection
 * timeout of 'connect_timeout'.
 *
 * Return file descriptor on success, -1 on error.
 */
int _cerebro_low_timeout_connect(cerebro_t handle,
				 const char *hostname,
				 unsigned int port,
				 unsigned int connect_timeout);

/* 
 * _cerebro_load_config
 *
 * Read and load config file data
 *
 * Returns 0 on success, -1 on error
 */
int _cerebro_load_config(cerebro_t handle);

/* 
 * _cerebro_unload_config
 *
 * Unload config file info
 *
 * Returns 0 on success, -1 on error
 */
int _cerebro_unload_config(cerebro_t handle);

/* 
 * _cerebro_load_clusterlist_module
 *
 * Find and load clusterlist module
 *
 * Returns 1 if clusterlist module was loaded, 0 if not, -1 on fatal error
 */
int _cerebro_load_clusterlist_module(cerebro_t handle);

/* 
 * _cerebro_unload_clusterlist_module
 *
 * Unload clusterlist module
 *
 * Returns 0 on success, -1 on error
 */
int _cerebro_unload_clusterlist_module(cerebro_t handle);

#endif /* _CEREBRO_UTIL_H */
