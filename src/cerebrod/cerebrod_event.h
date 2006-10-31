/*****************************************************************************\
 *  $Id: cerebrod_event.h,v 1.1.2.1 2006-10-31 06:26:36 chu11 Exp $
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

#ifndef _CEREBROD_EVENT_H
#define _CEREBROD_EVENT_H

/*
 * struct cerebrod_event_module
 *
 * contains cerebrod event module metric information
 */
struct cerebrod_event_module
{
  char *metric_names;
  int index;
  pthread_mutex_t event_lock;
};
 
/* 
 * cerebrod_event_manager
 *
 * Runs a cerebrod event management thread
 *
 * Passed no argument
 * 
 * Executed in detached state, no return value.
 */
void *cerebrod_event_manager(void *);

/* 
 * cerebrod_event_server
 *
 * Runs a cerebrod event management thread
 *
 * Passed no argument
 * 
 * Executed in detached state, no return value.
 */
void *cerebrod_event_server(void *);

#endif /* _CEREBROD_EVENT_H */
