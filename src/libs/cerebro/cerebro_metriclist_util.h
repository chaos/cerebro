/*****************************************************************************\
 *  $Id: cerebro_metriclist_util.h,v 1.1 2005-06-23 22:54:05 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_METRICLIST_UTIL_H
#define _CEREBRO_METRICLIST_UTIL_H

#include "cerebro.h"

/*
 * _cerebro_metriclist_check
 *
 * Checks for a proper cerebro metriclist, setting the errnum
 * appropriately if an error is found.
 *
 * Returns 0 on success, -1 on error
 */
int _cerebro_metriclist_check(cerebro_metriclist_t metriclist);

/* 
 * _cerebro_metriclist_create
 *
 * Create and initialize a metriclist 
 *
 * Returns metriclist on success, NULL on error
 */
cerebro_metriclist_t _cerebro_metriclist_create(cerebro_t handle);

/*
 * _cerebro_metriclist_append
 * 
 * Append additional metriclist data
 *
 * Returns 0 on success, -1 on error
 */
int _cerebro_metriclist_append(cerebro_metriclist_t metriclist,
                               const char *metric_name);

#endif /* _CEREBRO_METRICLIST_UTIL_H */
