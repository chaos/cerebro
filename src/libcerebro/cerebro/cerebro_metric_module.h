/*****************************************************************************\
 *  $Id: cerebro_metric_module.h,v 1.1 2005-06-13 16:22:16 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_METRIC_MODULE_H
#define _CEREBRO_METRIC_MODULE_H

/*
 * Cerebro_metric_setup
 *
 * function prototype for metric module function to setup the
 * module.  Required to be defined by each metric module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_metric_setup)(void);

/*
 * Cerebro_metric_cleanup
 *
 * function prototype for metric module function to cleanup.  Required
 * to be defined by each metric module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_metric_cleanup)(void);

/* 
 * Cerebro_metric_get_metric_name
 *
 * Returns the name of the metric metriced by this module Required to
 * be defined by each metric module.
 *
 * Returns string on success, -1 on error
 */
typedef char *(*Cerebro_metric_get_metric_name)(void);

/* 
 * Cerebro_metric_get_metric_value_type
 *
 * Returns the value_type of the metric metriced by this module. Required
 * to be defined by each metric module.
 *
 * Returns value_type on success, -1 on error
 */
typedef int (*Cerebro_metric_get_metric_value_type)(void);

/* 
 * Cerebro_metric_get_metric_value_len
 *
 * Returns the value_len of the metric metriced by this
 * module. Required to be defined by each metric module.
 *
 * Returns value_len on success, -1 on error
 */
typedef int (*Cerebro_metric_get_metric_value_len)(void);

/*
 * Cerebro_metric_get_metric_value
 *
 * function prototype for metric module function to get a metric
 * value.  Required to be defined by each metric module.
 *
 * Returns length of data copied on success, -1 on error
 */
typedef int (*Cerebro_metric_get_metric_value)(void *metric_value_buf,
                                                unsigned int metric_value_buflen);

/*
 * struct cerebro_metric_module_info 
 * 
 * contains metric module information and operations.  Required to be
 * defined in each metric module.
 */
struct cerebro_metric_module_info
{
  char *metric_module_name;
  Cerebro_metric_setup setup;
  Cerebro_metric_cleanup cleanup;
  Cerebro_metric_get_metric_name get_metric_name;
  Cerebro_metric_get_metric_value_type get_metric_value_type;
  Cerebro_metric_get_metric_value_len get_metric_value_len;
  Cerebro_metric_get_metric_value get_metric_value;
};

#endif /* _CEREBRO_METRIC_MODULE_H */
