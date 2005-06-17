/*****************************************************************************\
 *  $Id: cerebro_module_monitor.h,v 1.1 2005-06-17 20:54:08 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_MODULE_MONITOR_H
#define _CEREBRO_MODULE_MONITOR_H

typedef struct cerebro_monitor_module *cerebro_monitor_modules_t; 

/*
 * _cerebro_module_load_monitor_modules
 *
 * Find and load the monitor modules.
 * 
 * Returns monitor module handle on success, NULL on error
 */
cerebro_monitor_modules_t _cerebro_module_load_monitor_modules(unsigned int monitors_max);

/*
 * _cerebro_module_destroy_monitor_handle
 *
 * Destroy/Unload the monitor module specified by the handle
 *
 * Returns 0 on success, -1 on error
 */
int _cerebro_module_destroy_monitor_handle(cerebro_monitor_modules_t monitor_handle);

/*
 * _cerebro_monitor_module_count
 *
 * Return number of monitoring modules loaded, -1 on error
 */
int _cerebro_monitor_module_count(cerebro_monitor_modules_t monitor_handle);

/*
 * _cerebro_monitor_module_name
 *
 * Return monitor module name
 */
char *_cerebro_monitor_module_name(cerebro_monitor_modules_t monitor_handle, 
                                   unsigned int index);

/*
 * _cerebro_monitor_module_setup
 *
 * call monitor module setup function
 */
int _cerebro_monitor_module_setup(cerebro_monitor_modules_t monitor_handle, 
                                  unsigned int index);

/*
 * _cerebro_monitor_module_cleanup
 *
 * call monitor module cleanup function
 */
int _cerebro_monitor_module_cleanup(cerebro_monitor_modules_t monitor_handle, 
                                    unsigned int index);

/*
 * _cerebro_monitor_module_metric_name
 *
 * call monitor module metric_names function
 */
char * _cerebro_monitor_module_metric_name(cerebro_monitor_modules_t monitor_handle, 
                                           unsigned int index);

/*
 * _cerebro_monitor_module_metric_update
 *
 * call monitor module metric_update function
 */
int _cerebro_monitor_module_metric_update(cerebro_monitor_modules_t monitor_handle, 
                                          unsigned int index,
                                          const char *nodename,
                                          const char *metric_name,
                                          unsigned int metric_value_type,
                                          unsigned int metric_value_len,
                                          void *metric_value);

#endif /* _CEREBRO_MODULE_MONITOR_H */
