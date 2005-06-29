/*****************************************************************************\
 *  $Id: cerebro_metric_util.h,v 1.7 2005-06-29 17:52:26 achu Exp $
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
 * _cerebro_metric_response_check
 *
 * Check that the version and error code are good prior to unmarshalling
 *
 * Returns 0 on success, -1 on error
 */
int _cerebro_metric_response_check(cerebro_t handle,
                                   const char *buf,
                                   unsigned int buflen);


#endif /* _CEREBRO_METRIC_UTIL_H */
