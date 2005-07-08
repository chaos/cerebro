/*****************************************************************************\
 *  $Id: cerebro_metric_util.h,v 1.9 2005-07-08 23:10:03 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_METRIC_UTIL_H
#define _CEREBRO_METRIC_UTIL_H

#include "cerebro.h"
#include "cerebro/cerebro_metric_server_protocol.h"

/* 
 * Cerebro_metric_receive_response
 *
 * Function to call after the metric request has been sent
 */
typedef int (*Cerebro_metric_receive_response)(cerebro_t handle,
                                               void *list,
                                               struct cerebro_metric_server_response *res,
                                               unsigned int bytes_read,
                                               int fd);

/* 
 * _cerebro_metric_get_data
 *
 * Connect to the cerebrod metric and receive responses
 *
 * Returns 0 on success, -1 on error
 */
int _cerebro_metric_get_data(cerebro_t handle,
                             void *list,
                             const char *metric_name,
                             Cerebro_metric_receive_response receive_response);

#endif /* _CEREBRO_METRIC_UTIL_H */
