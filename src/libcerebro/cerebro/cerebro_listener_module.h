/*****************************************************************************\
 *  $Id: cerebro_listener_module.h,v 1.1 2005-06-10 23:17:29 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_LISTENER_MODULE_H
#define _CEREBRO_LISTENER_MODULE_H

/*
 * Cerebro_listener_setup
 *
 * function prototype for listener module function to setup the
 * module.  Required to be defined by each listener module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_listener_setup)(void);

/*
 * Cerebro_listener_cleanup
 *
 * function prototype for listener module function to cleanup.  Required
 * to be defined by each listener module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_listener_cleanup)(void);

/* 
 * Cerebro_listener_metric_update
 *
 * Returns the name of the metric listenered by this module Required to
 * be defined by each listener module.
 *
 * Returns string on success, -1 on error
 */
typedef char *(*Cerebro_listener_get_metric_name)(void);

/* 
 * Cerebro_listener_get_metric_value_type
 *
 * Returns the value_type of the metric listenered by this module. Required
 * to be defined by each listener module.
 *
 * Returns value_type on success, -1 on error
 */
typedef int (*Cerebro_listener_get_metric_value_type)(void);

/* 
 * Cerebro_listener_get_metric_value_len
 *
 * Returns the value_len of the metric listenered by this
 * module. Required to be defined by each listener module.
 *
 * Returns value_len on success, -1 on error
 */
typedef int (*Cerebro_listener_get_metric_value_len)(void);

/*
 * Cerebro_listener_get_metric_value
 *
 * function prototype for listener module function to get a metric
 * value.  Required to be defined by each listener module.
 *
 * Returns length of data copied on success, -1 on error
 */
typedef int (*Cerebro_listener_get_metric_value)(void *metric_value_buf,
                                                unsigned int metric_value_buflen);

/*
 * struct cerebro_listener_module_info 
 * 
 * contains listener module information and operations.  Required to be
 * defined in each listener module.
 */
struct cerebro_listener_module_info
{
  char *listener_module_name;
  Cerebro_listener_setup setup;
  Cerebro_listener_cleanup cleanup;
  Cerebro_listener_get_metric_name get_metric_name;
  Cerebro_listener_get_metric_value_type get_metric_value_type;
  Cerebro_listener_get_metric_value_len get_metric_value_len;
  Cerebro_listener_get_metric_value get_metric_value;
};

#endif /* _CEREBRO_LISTENER_MODULE_H */
