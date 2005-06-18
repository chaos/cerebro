/*****************************************************************************\
 *  $Id: cerebro_module_metric.h,v 1.1 2005-06-18 06:47:06 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_MODULE_METRIC_H
#define _CEREBRO_MODULE_METRIC_H

typedef struct cerebro_metric_module *cerebro_metric_modules_t; 

/*
 * _cerebro_module_load_metric_modules
 *
 * Find and load the metric modules.
 * 
 * Returns metric module handle on success, NULL on error
 */
cerebro_metric_modules_t _cerebro_module_load_metric_modules(unsigned int metrics_max);

/*
 * _cerebro_module_destroy_metric_handle
 *
 * Destroy/Unload the metric module specified by the handle
 *
 * Returns 0 on success, -1 on error
 */
int _cerebro_module_destroy_metric_handle(cerebro_metric_modules_t metric_handle);

/*
 * _cerebro_metric_module_count
 *
 * Return number of metricing modules loaded, -1 on error
 */
int _cerebro_metric_module_count(cerebro_metric_modules_t metric_handle);

/*
 * _cerebro_metric_module_name
 *
 * Return metric module name
 */
char *_cerebro_metric_module_name(cerebro_metric_modules_t metric_handle, 
                                  unsigned int index);

/*
 * _cerebro_metric_module_setup
 *
 * call metric module setup function
 */
int _cerebro_metric_module_setup(cerebro_metric_modules_t metric_handle, 
                                 unsigned int index);

/*
 * _cerebro_metric_module_cleanup
 *
 * call metric module cleanup function
 */
int _cerebro_metric_module_cleanup(cerebro_metric_modules_t metric_handle, 
                                   unsigned int index);

/*
 * _cerebro_metric_module_get_metric_name
 *
 * call metric module get_metric_name function
 */
char *_cerebro_metric_module_get_metric_name(cerebro_metric_modules_t metric_handle,
                                             unsigned int index);

/*
 * _cerebro_metric_module_get_metric_value
 *
 * call metric module get_metric_value function
 */
int _cerebro_metric_module_get_metric_value(cerebro_metric_modules_t metric_handle,
                                            unsigned int index,
                                            unsigned int *metric_value_type,
                                            unsigned int *metric_value_len,
                                            void **metric_value);

/*
 * _cerebro_metric_module_destroy_metric_value
 *
 * call metric module destroy_metric_value function
 */
int _cerebro_metric_module_destroy_metric_value(cerebro_metric_modules_t metric_handle,
                                                unsigned int index,
                                                void *metric_value);

#endif /* _CEREBRO_MODULE_METRIC_H */
