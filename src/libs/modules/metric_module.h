/*****************************************************************************\
 *  $Id: metric_module.h,v 1.5 2005-07-21 20:15:45 achu Exp $
\*****************************************************************************/

#ifndef _METRIC_MODULE_H
#define _METRIC_MODULE_H

typedef struct metric_module *metric_modules_t; 

/*
 * metric_modules_load
 *
 * Find and load the metric modules.
 * 
 * Returns metric module handle on success, NULL on error
 */
metric_modules_t metric_modules_load(unsigned int metrics_max);

/*
 * metric_modules_unload
 *
 * Destroy/Unload the metric module specified by the handle
 *
 * Returns 0 on success, -1 on error
 */
int metric_modules_unload(metric_modules_t handle);

/*
 * metric_modules_count
 *
 * Return number of metricing modules loaded, -1 on error
 */
int metric_modules_count(metric_modules_t handle);

/*
 * metric_module_name
 *
 * Return metric module name
 */
char *metric_module_name(metric_modules_t handle, unsigned int index);

/*
 * metric_module_setup
 *
 * call metric module setup function
 */
int metric_module_setup(metric_modules_t handle, unsigned int index);

/*
 * metric_module_cleanup
 *
 * call metric module cleanup function
 */
int metric_module_cleanup(metric_modules_t handle, unsigned int index);

/*
 * metric_module_get_metric_name
 *
 * call metric module get_metric_name function
 */
char *metric_module_get_metric_name(metric_modules_t handle, unsigned int index);

/*
 * metric_module_get_metric_period
 *
 * call metric module get_metric_period function
 */
int metric_module_get_metric_period(metric_modules_t handle, 
                                    unsigned int index,
                                    int *period);

/*
 * metric_module_get_metric_value
 *
 * call metric module get_metric_value function
 */
int metric_module_get_metric_value(metric_modules_t handle,
				   unsigned int index,
				   unsigned int *metric_value_type,
				   unsigned int *metric_value_len,
				   void **metric_value);

/*
 * metric_module_destroy_metric_value
 *
 * call metric module destroy_metric_value function
 */
int metric_module_destroy_metric_value(metric_modules_t handle,
				       unsigned int index,
				       void *metric_value);

/*
 * metric_module_get_metric_thread
 *
 * call metric module get_metric_thread function
 *
 * Returns 0 on success, -1 on error
 */
Cerebro_metric_thread_pointer metric_module_get_metric_thread(metric_modules_t handle,
                                                              unsigned int index);

#endif /* _METRIC_MODULE_H */
