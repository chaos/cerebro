/*****************************************************************************\
 *  $Id: cerebro_monitor_module.h,v 1.1 2005-06-09 20:09:51 achu Exp $
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
 * Cerebro_monitor_get_metric_value
 *
 * function prototype for monitor module function to get a metric
 * value.  Required to be defined by each monitor module.
 *
 * Returns length copied into buffer on success, 0 if buffer was too
 * small, -1 on error
 */
typedef int (*Cerebro_monitor_get_metric_value)(void *metric_value_buf,
                                                unsigned int metric_value_buflen,
                                                unsigned int *metric_value_type,
                                                unsigned int *metric_value_len);

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
  Cerebro_monitor_get_metric_value get_metric_value;
};

#endif /* _CEREBRO_MONITOR_MODULE_H */
