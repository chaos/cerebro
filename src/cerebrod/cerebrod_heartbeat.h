/*****************************************************************************\
 *  $Id: cerebrod_heartbeat.h,v 1.16 2006-02-22 06:08:27 chu11 Exp $
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

#ifndef _CEREBROD_HEARTBEAT_H
#define _CEREBROD_HEARTBEAT_H

#include "cerebro/cerebrod_heartbeat_protocol.h"

/*
 * cerebrod_heartbeat_destroy
 *
 * destroy a heartbeat packet
 */
void cerebrod_heartbeat_destroy(struct cerebrod_heartbeat *hb);

/* 
 * cerebrod_heartbeat_dump
 *
 * dump contents of a heartbeat packet.  Should be called with
 * debug_output_mutex held.
 */
void cerebrod_heartbeat_dump(struct cerebrod_heartbeat *hb);

#endif /* _CEREBROD_HEARTBEAT_H */
