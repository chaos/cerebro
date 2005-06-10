/*****************************************************************************\
 *  $Id: cerebro_module.h,v 1.26 2005-06-10 00:28:09 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_MODULE_H
#define _CEREBRO_MODULE_H

#include "cerebro/cerebro_config.h"

typedef struct cerebro_clusterlist_module *cerebro_clusterlist_module_t;
typedef struct cerebro_config_module *cerebro_config_module_t; 
typedef struct cerebro_monitor_module *cerebro_monitor_modules_t; 

/*
 * _cerebro_module_load_clusterlist_module
 *
 * Find and load the clusterlist module.  If none is found, cerebro
 * library will assume a default clusterlist module.
 * 
 * Returns clusterlist module handle on success, NULL on error
 */
cerebro_clusterlist_module_t _cerebro_module_load_clusterlist_module(void);

/*
 * _cerebro_module_destroy_clusterlist_handle
 *
 * Destroy/Unload the clusterlist module specified by the handle.
 *
 * Returns 0 on success, -1 on error
 */
int _cerebro_module_destroy_clusterlist_handle(cerebro_clusterlist_module_t clusterlist_handle);

/*
 * _cerebro_module_load_config_module
 *
 * Find and load the config module.  If none is found, cerebro
 * library will assume a default config module.
 * 
 * Returns config module handle on success, NULL on error
 */
cerebro_config_module_t _cerebro_module_load_config_module(void);

/*
 * _cerebro_module_destroy_config_handle
 *
 * Destroy/Unload the config module specified by the handle
 *
 * Returns 0 on success, -1 on error
 */
int _cerebro_module_destroy_config_handle(cerebro_config_module_t config_handle);

/*
 * _cerebro_module_load_monitor_modules
 *
 * Find and load the monitor modules.
 * 
 * Returns monitor module handle on success, NULL on error
 */
cerebro_monitor_modules_t _cerebro_module_load_monitor_modules(unsigned int metrics_max);

/*
 * _cerebro_module_destroy_monitor_handle
 *
 * Destroy/Unload the monitor module specified by the handle
 *
 * Returns 0 on success, -1 on error
 */
int _cerebro_module_destroy_monitor_handle(cerebro_monitor_modules_t config_handle);

/*
 * _cerebro_clusterlist_module_name
 *
 * Return clusterlist module name
 */
char *_cerebro_clusterlist_module_name(cerebro_clusterlist_module_t clusterlist_handle);

/*
 * _cerebro_clusterlist_module_setup
 *
 * call clusterlist module setup function
 */
int _cerebro_clusterlist_module_setup(cerebro_clusterlist_module_t clusterlist_handle);

/*
 * _cerebro_clusterlist_module_cleanup
 *
 * call clusterlist module parse cleanup function
 */
int _cerebro_clusterlist_module_cleanup(cerebro_clusterlist_module_t clusterlist_handle);

/*
 * _cerebro_clusterlist_module_numnodes
 *
 * call clusterlist module numnodes function
 */
int _cerebro_clusterlist_module_numnodes(cerebro_clusterlist_module_t clusterlist_handle);

/*
 * _cerebro_clusterlist_module_get_all_nodes
 *
 * call clusterlist module get all nodes function
 */
int _cerebro_clusterlist_module_get_all_nodes(cerebro_clusterlist_module_t clusterlist_handle,
                                              char ***nodes);

/*
 * _cerebro_clusterlist_module_node_in_cluster
 *
 * call clusterlist module node in cluster function
 */
int _cerebro_clusterlist_module_node_in_cluster(cerebro_clusterlist_module_t clusterlist_handle,
                                                const char *node);

/*
 * _cerebro_clusterlist_module_get_nodename
 *
 * call clusterlist module get nodename function
 */
int _cerebro_clusterlist_module_get_nodename(cerebro_clusterlist_module_t clusterlist_handle,
                                             const char *node, 
                                             char *buf, 
                                             unsigned int buflen);

/*
 * _cerebro_config_module_name
 *
 * Return config module name
 */
char *_cerebro_config_module_name(cerebro_config_module_t config_handle);

/*
 * _cerebro_config_module_setup
 *
 * call config module setup function
 */
int _cerebro_config_module_setup(cerebro_config_module_t config_handle);

/*
 * _cerebro_config_module_cleanup
 *
 * call config module parse cleanup function
 */
int _cerebro_config_module_cleanup(cerebro_config_module_t config_handle);

/*
 * _cerebro_config_module_load_default
 *
 * call config module get all nodes function
 */
int _cerebro_config_module_load_default(cerebro_config_module_t config_handle,
                                        struct cerebro_config *conf);


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
 * call monitor module parse cleanup function
 */
int _cerebro_monitor_module_cleanup(cerebro_monitor_modules_t monitor_handle, 
                                    unsigned int index);

/*
 * _cerebro_monitor_module_get_metric_name
 *
 * call monitor module get all nodes function
 */
char *_cerebro_monitor_module_get_metric_name(cerebro_monitor_modules_t monitor_handle,
                                              unsigned int index);

/*
 * _cerebro_monitor_module_get_metric_value_type
 *
 * call monitor module get all nodes function
 */
int _cerebro_monitor_module_get_metric_value_type(cerebro_monitor_modules_t monitor_handle,
                                                  unsigned int index);

/*
 * _cerebro_monitor_module_get_metric_value_len
 *
 * call monitor module get all nodes function
 */
int _cerebro_monitor_module_get_metric_value_len(cerebro_monitor_modules_t monitor_handle,
                                                 unsigned int index);

/*
 * _cerebro_monitor_module_get_metric_value
 *
 * call monitor module get all nodes function
 */
int _cerebro_monitor_module_get_metric_value(cerebro_monitor_modules_t monitor_handle,
                                             unsigned int index,
                                             void *metric_value_buf,
                                             unsigned int metric_value_buflen);

#endif /* _CEREBRO_MODULE_H */
