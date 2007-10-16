/*****************************************************************************\
 *  $Id: cerebrod_message.h,v 1.3 2007-10-16 22:43:15 chu11 Exp $
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
 *  with Cerebro; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
\*****************************************************************************/

#ifndef _CEREBROD_MESSAGE_H
#define _CEREBROD_MESSAGE_H

#include "cerebro/cerebrod_message_protocol.h"

/*
 * cerebrod_message_destroy
 *
 * destroy a message packet
 */
void cerebrod_message_destroy(struct cerebrod_message *msg);

/* 
 * cerebrod_message_dump
 *
 * dump contents of a message packet.  Should be called with
 * debug_output_mutex held.
 */
void cerebrod_message_dump(struct cerebrod_message *msg);

#endif /* _CEREBROD_MESSAGE_H */
