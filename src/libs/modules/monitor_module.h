/*****************************************************************************\
 *  $Id: monitor_module.h,v 1.2 2005-06-20 18:46:02 achu Exp $
\*****************************************************************************/

#ifndef _MONITOR_MODULE_H
#define _MONITOR_MODULE_H

typedef struct monitor_module *monitor_modules_t; 

/*
 * monitor_modules_load
 *
 * Find and load the monitor modules.
 * 
 * Returns monitor module handle on success, NULL on error
 */
monitor_modules_t monitor_modules_load(unsigned int monitors_max);

/*
 * monitor_modules_unload
 *
 * Destroy/Unload the monitor module specified by the handle
 *
 * Returns 0 on success, -1 on error
 */
int monitor_modules_unload(monitor_modules_t monitor_handle);

/*
 * monitor_modules_count
 *
 * Return number of monitoring modules loaded, -1 on error
 */
int monitor_modules_count(monitor_modules_t monitor_handle);

/*
 * monitor_module_name
 *
 * Return monitor module name
 */
char *monitor_module_name(monitor_modules_t monitor_handle, 
			  unsigned int index);

/*
 * monitor_module_setup
 *
 * call monitor module setup function
 */
int monitor_module_setup(monitor_modules_t monitor_handle, 
			 unsigned int index);

/*
 * monitor_module_cleanup
 *
 * call monitor module cleanup function
 */
int monitor_module_cleanup(monitor_modules_t monitor_handle, 
			   unsigned int index);

/*
 * monitor_module_metric_name
 *
 * call monitor module metric_names function
 */
char * monitor_module_metric_name(monitor_modules_t monitor_handle, 
				  unsigned int index);

/*
 * monitor_module_metric_update
 *
 * call monitor module metric_update function
 */
int monitor_module_metric_update(monitor_modules_t monitor_handle, 
				 unsigned int index,
				 const char *nodename,
				 unsigned int metric_value_type,
				 unsigned int metric_value_len,
				 void *metric_value);

#endif /* _MONITOR_MODULE_H */
