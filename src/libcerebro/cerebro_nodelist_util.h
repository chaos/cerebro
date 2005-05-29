/*****************************************************************************\
 *  $Id: cerebro_nodelist_util.h,v 1.4 2005-05-29 14:45:52 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_NODELIST_UTIL_H
#define _CEREBRO_NODELIST_UTIL_H

#include "cerebro.h"
#include "cerebro/cerebro_metric_protocol.h"

/*
 * _cerebro_nodelist_check
 *
 * Checks for a proper cerebro nodelist, setting the errnum
 * appropriately if an error is found.
 *
 * Returns 0 on success, -1 on error
 */
int _cerebro_nodelist_check(cerebro_nodelist_t nodelist);

/* 
 * _cerebro_nodelist_create
 *
 * Create and initialize a nodelist 
 *
 * Returns nodelist on success, NULL on error
 */
cerebro_nodelist_t _cerebro_nodelist_create(cerebro_t handle);

/*
 * _cerebro_nodelist_append
 * 
 * Append additional nodelist data
 *
 * Returns 0 on success, -1 on error
 */
int _cerebro_nodelist_append(cerebro_nodelist_t nodelist,
			     const char *nodename,
			     cerebro_metric_value_t *metric_value);

/*
 * _cerebro_nodelist_sort
 * 
 * Sort the nodelist by nodename
 *
 * Returns 0 on success, -1 on error
 */
int _cerebro_nodelist_sort(cerebro_nodelist_t nodelist);

#endif /* _CEREBRO_NODELIST_UTIL_H */
