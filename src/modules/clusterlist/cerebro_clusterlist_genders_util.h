/*****************************************************************************\
 *  $Id: cerebro_clusterlist_genders_util.h,v 1.15 2010-02-02 01:01:21 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2010 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2005-2007 The Regents of the University of California.
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
 *  with Cerebro.  If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#ifndef _CEREBRO_CLUSTERLIST_GENDERS_UTIL_H
#define _CEREBRO_CLUSTERLIST_GENDERS_UTIL_H

/* 
 * cerebro_clusterlist_genders_setup
 *
 * common setup function for genders and gendersllnl setup
 */
int cerebro_clusterlist_genders_setup(genders_t *gh, char *filename);

/* 
 * cerebro_clusterlist_genders_cleanup
 *
 * common cleanup function for genders and gendersllnl cleanup
 */
int cerebro_clusterlist_genders_cleanup(genders_t *gh);

/* 
 * cerebro_clusterlist_genders_numnodes
 *
 * common numnodes function for genders and gendersllnl numnodes
 */
int cerebro_clusterlist_genders_numnodes(genders_t gh);

/* 
 * cerebro_clusterlist_genders_get_all_nodes
 *
 * common get_all_nodes function for genders and gendersllnl get_all_nodes
 */
int cerebro_clusterlist_genders_get_all_nodes(genders_t gh, char ***nodes);

#endif /* _CEREBRO_CLUSTERLIST_GENDERS_UTIL_H */
