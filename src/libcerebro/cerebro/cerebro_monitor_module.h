/*****************************************************************************\
 *  $Id: cerebro_monitor_module.h,v 1.4 2005-06-13 23:05:54 achu Exp $
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
 * Cerebro_monitor_metric_names
 *
 * function prototype for monitor module to return all metric names
 * this module wishes to monitor.  Caller is responsible for freeing
 * the created char ** array of metric name strings.  The returned
 * array will be NULL terminated.  Required to be defined by each
 * monitor module.
 *
 * - metric_names - pointer to return char ** array of nodes
 *
 * Returns number of metric names on success, -1 on error
 */
typedef int (*Cerebro_monitor_metric_names)(char ***metric_names);

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
  Cerebro_monitor_metric_names metric_names;
  Cerebro_monitor_metric_update metric_update;
};

#endif /* _CEREBRO_MONITOR_MODULE_H */
