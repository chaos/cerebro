/*****************************************************************************\
 *  $Id: cerebro_nodelist_util.h,v 1.2 2005-07-22 17:21:07 achu Exp $
 *****************************************************************************
 *  Copyright (C) 2005 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <http://www.llnl.gov/linux/cerebro/>.
 *
 *  Cerebro is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  Cerebro is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with Genders; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
\*****************************************************************************/

#ifndef _CEREBRO_NODELIST_UTIL_H
#define _CEREBRO_NODELIST_UTIL_H

#include "cerebro.h"

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
cerebro_nodelist_t _cerebro_nodelist_create(cerebro_t handle, 
                                            const char *metric_name);

/*
 * _cerebro_nodelist_append
 * 
 * Append additional nodelist data
 *
 * Returns 0 on success, -1 on error
 */
int _cerebro_nodelist_append(cerebro_nodelist_t nodelist,
			     const char *nodename,
                             u_int32_t metric_type,
                             u_int32_t metric_len,
                             void *metric_data);

/*
 * _cerebro_nodelist_sort
 * 
 * Sort the nodelist by nodename
 *
 * Returns 0 on success, -1 on error
 */
int _cerebro_nodelist_sort(cerebro_nodelist_t nodelist);

#endif /* _CEREBRO_NODELIST_UTIL_H */
