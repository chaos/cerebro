/*****************************************************************************\
 *  $Id: cerebro_metric_util.h,v 1.2 2005-06-23 23:58:32 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_METRIC_UTIL_H
#define _CEREBRO_METRIC_UTIL_H

/*
 * _cerebro_metric_protocol_err_conversion
 *
 * Convert metric protocol err codes to API err codes
 *
 * Returns proper err code
 */
int _cerebro_metric_protocol_err_conversion(u_int32_t protocol_error);

/* 
 * _cerebro_metric_config
 *
 * Determine the port, timeout_len, and flags to use
 */
int _cerebro_metric_config(cerebro_t handle,
                           unsigned int *port,
                           unsigned int *timeout_len,
                           unsigned int *flags);

/*
 * _cerebro_metric_request_send
 *
 * Send the metric request
 *
 * Returns 0 on success, -1 on error
 */
int _cerebro_metric_request_send(cerebro_t handle,
                                 int fd,
                                 const char *metric_name,
                                 unsigned int timeout_len,
                                 int flags);

     
#endif /* _CEREBRO_METRIC_UTIL_H */
