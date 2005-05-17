/*****************************************************************************\
 *  $Id: cerebrod_status.h,v 1.3 2005-05-17 22:33:44 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_STATUS_H
#define _CEREBROD_STATUS_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_PTHREAD_H
#include <pthread.h>
#endif /* HAVE_PTHREAD_H */

#include <sys/types.h>

#include "hash.h"

#define CEREBROD_STATUS_REINITIALIZE_WAIT 2

/* 
 * cerebrod_status_type_t
 *
 * type for status type
 */
typedef enum {
  CEREBROD_STATUS_TYPE_INT32_T,
  CEREBROD_STATUS_TYPE_U_INT32_T,
} cerebrod_status_type_t;

/* 
 * cerebrod_status_type_t
 *
 * type for status value
 */
typedef union {
  int32_t   val_int32;
  u_int32_t val_u_int32;
} cerebrod_status_val_t;

/*
 * struct cerebrod_status_node_data
 *
 * contains cerebrod status node data
 */
struct cerebrod_status_node_data
{
  char *nodename;
  hash_t status_node_data;
  int status_node_data_count;
  pthread_mutex_t status_node_data_lock;
};

/*
 * cerebrod_status
 *
 * Runs the cerebrod status server thread
 *
 * Passed no argument
 *
 * Executed in detached state, no return value.
 */
void *cerebrod_status(void *);

/* 
 * cerebrod_status_update_data
 *
 * Update status server with last_received time for a specific cluster
 * node
 */
void cerebrod_status_update_data(char *nodename, 
                                 char *status_name,
                                 cerebrod_status_type_t status_type,
                                 cerebrod_status_val_t status_val);

#endif /* _CEREBROD_STATUS_H */
