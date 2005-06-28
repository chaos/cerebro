/*****************************************************************************\
 *  $Id: cerebro_metric_util.h,v 1.5 2005-06-28 20:58:32 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_METRIC_UTIL_H
#define _CEREBRO_METRIC_UTIL_H

#include "cerebro.h"

/*
 * _cerebro_metric_protocol_err_conversion
 *
 * Convert metric protocol err codes to API err codes
 *
 * Returns proper err code
 */
int _cerebro_metric_protocol_err_conversion(u_int32_t protocol_error);

/* 
 * Cerebro_metric_response_receive
 *
 * Function to call after the metric request has been sent
 */
typedef int (*Cerebro_metric_response_receive)(cerebro_t handle,
                                               void *list,
                                               int fd);

/* 
 * _cerebro_metric_connect_and_receive
 *
 * Connect to the cerebrod metric and receive responses
 *
 * Returns 0 on success, -1 on error
 */
int _cerebro_metric_connect_and_receive(cerebro_t handle,
                                        void *list,
                                        const char *metric_name,
                                        Cerebro_metric_response_receive response_receive);

/*
 * _cerebro_metric_receive_data
 *
 * Receive a certain amount of data
 *
 * Returns bytes read on success, -1 on error
 *
 */
int _cerebro_metric_receive_data(cerebro_t handle,
                                 int fd,
                                 unsigned int bytes_to_read,
                                 char *buf,
                                 unsigned int buflen);

#endif /* _CEREBRO_METRIC_UTIL_H */
