/*****************************************************************************\
 *  $Id: cerebro_monitor_module.h,v 1.1 2005-06-17 22:58:30 achu Exp $
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
 * Cerebro_monitor_metric_name
 *
 * function prototype for monitor module to return the metric name
 * this module wishes to monitor.
 *
 * Returns metric name on success, -1 on error
 */
typedef char *(*Cerebro_monitor_metric_name)(void);

/*
 * Cerebro_monitor_metric_update
 *
 * function prototype for monitor module function to be updatd with a
 * new metric value.  Required to be defined by each monitor module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_monitor_metric_update)(const char *nodename,
                                             const char *metric_name,
                                             unsigned int metric_value_type,
                                             unsigned int metric_value_len,
                                             void *metric_value);

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
  Cerebro_monitor_metric_name metric_name;
  Cerebro_monitor_metric_update metric_update;
};

#endif /* _CEREBRO_MONITOR_MODULE_H */
