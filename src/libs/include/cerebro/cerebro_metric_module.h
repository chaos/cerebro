/*****************************************************************************\
 *  $Id: cerebro_metric_module.h,v 1.6 2005-07-21 23:11:26 achu Exp $
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
 * Cerebro_metric_get_metric_period
 *
 * Retrieve the period in seconds that the metric value should be read
 * and propogated.  If the period is 0, the metric will be read and
 * propogated with every cerebro heartbeat. If the period is < 0, the
 * metric will be propogated only when instructed to. Note that the
 * period is not precise, and is only an approximation.  Data is only
 * propogated in cerebro heartbeats, therefore the period time
 * granularity will be related to the cerebro heartbeat period.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_metric_get_metric_period)(int *period);

/*
 * Cerebro_metric_get_metric_value
 *
 * function prototype for metric module function to get a metric
 * value.  Required to be defined by each metric module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_metric_get_metric_value)(unsigned int *metric_value_type,
                                               unsigned int *metric_value_len,
                                               void **metric_value);

/*
 * Cerebro_metric_destroy_metric_value
 *
 * function prototype for metric module function to destroy a metric
 * value if necessary.  Required to be defined by each metric module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_metric_destroy_metric_value)(void *metric_value);

/* 
 * Cerebro_metric_updated
 *
 * function prototype to inform the cerebrod daemon a metric
 * has been updated.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Cerebro_metric_updated)(char *metric_name);

/* 
 * Cerebro_metric_thread_pointer
 *
 * function prototype for a thread which will be passed to
 * pthread_create
 *
 * The function will be passed a pointer to a 'Cerebro_metric_updated'
 * function that can be called when value is updated.  This thread can
 * perform any metric monitoring duties it pleases and optionally call
 * the 'Cerebro_metric_updated' function when a metric value is
 * updated.
 *
 * Typically the thread is used to watch or monitor for some event and
 * locally update data so that cerebrod will propogate the newly
 * received data from a 'get_metric_value' call.
 *
 * If the user wishes to use mutexes within the metric thread to
 * protect against concurrent access, the user is responsible for not
 * putting the locks in situations that can lead to a deadlock.  The
 * 'Cerebro_metric_updated' function call may require locks to
 * function appropriately within the cerebrod daemon.
 */
typedef void *(*Cerebro_metric_thread_pointer)(void *arg);

/*
 * Cerebro_metric_get_metric_thread
 *
 * function prototype for metric module function that will return a
 * pointer to function that will be executed as a detached thread.
 *
 * If a metric_thread is not needed, this function returns NULL.
 * Required to be defined by each metric module.
 *
 * Returns 0 on success, -1 on error
 */
typedef Cerebro_metric_thread_pointer (*Cerebro_metric_get_metric_thread)(void);

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
  Cerebro_metric_get_metric_period get_metric_period;
  Cerebro_metric_get_metric_value get_metric_value;
  Cerebro_metric_destroy_metric_value destroy_metric_value;
  Cerebro_metric_get_metric_thread get_metric_thread;
};

#endif /* _CEREBRO_METRIC_MODULE_H */
