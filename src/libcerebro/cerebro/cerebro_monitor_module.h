/*****************************************************************************\
 *  $Id: cerebro_monitor_module.h,v 1.2 2005-06-10 00:28:09 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_MONITOR_MODULE_H
#define _CEREBRO_MONITOR_MODULE_H

/*
 * Cerebro_monitor_setup
 *
 * function prototype for monitor module function to setup the
 * module.  Required to be defined by each monitor module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_monitor_setup)(void);

/*
 * Cerebro_monitor_cleanup
 *
 * function prototype for monitor module function to cleanup.  Required
 * to be defined by each monitor module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_monitor_cleanup)(void);

/* 
 * Cerebro_monitor_get_metric_name
 *
 * Returns the name of the metric monitored by this module Required to
 * be defined by each monitor module.
 *
 * Returns string on success, -1 on error
 */
typedef char *(*Cerebro_monitor_get_metric_name)(void);

/* 
 * Cerebro_monitor_get_metric_value_type
 *
 * Returns the value_type of the metric monitored by this module. Required
 * to be defined by each monitor module.
 *
 * Returns value_type on success, -1 on error
 */
typedef int (*Cerebro_monitor_get_metric_value_type)(void);

/* 
 * Cerebro_monitor_get_metric_value_len
 *
 * Returns the value_len of the metric monitored by this
 * module. Required to be defined by each monitor module.
 *
 * Returns value_len on success, -1 on error
 */
typedef int (*Cerebro_monitor_get_metric_value_len)(void);

/*
 * Cerebro_monitor_get_metric_value
 *
 * function prototype for monitor module function to get a metric
 * value.  Required to be defined by each monitor module.
 *
 * Returns length of data copied on success, -1 on error
 */
typedef int (*Cerebro_monitor_get_metric_value)(void *metric_value_buf,
                                                unsigned int metric_value_buflen);

/*
 * struct cerebro_monitor_module_info 
 * 
 * contains monitor module information and operations.  Required to be
 * defined in each monitor module.
 */
struct cerebro_monitor_module_info
{
  char *monitor_module_name;
  Cerebro_monitor_setup setup;
  Cerebro_monitor_cleanup cleanup;
  Cerebro_monitor_get_metric_name get_metric_name;
  Cerebro_monitor_get_metric_value_type get_metric_value_type;
  Cerebro_monitor_get_metric_value_len get_metric_value_len;
  Cerebro_monitor_get_metric_value get_metric_value;
};

#endif /* _CEREBRO_MONITOR_MODULE_H */
