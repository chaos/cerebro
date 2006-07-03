/*****************************************************************************\
 *  $Id: cerebro_constants.h,v 1.6 2006-07-03 20:40:50 chu11 Exp $
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
 
#ifndef _CEREBRO_CONSTANTS_H
#define _CEREBRO_CONSTANTS_H

/* Do not use MAXHOSTNAMELEN defined in sys/param.h.  We do not want
 * the possibility that other machines may have a different length
 * definition for MAXHOSTNAMELEN.  We define our own maximum nodename
 * length.
 */
#define CEREBRO_MAX_NODENAME_LEN           64 
#define CEREBRO_MAX_HOSTNAME_LEN           CEREBRO_MAX_NODENAME_LEN

#define CEREBRO_MAX_PATH_LEN               512

#define CEREBRO_MAX_PACKET_LEN             4096

#define CEREBRO_MAX_IPADDR_LEN             64

#define CEREBRO_MAX_NETWORK_INTERFACE_LEN  128

#define CEREBRO_MAX_METRIC_NAME_LEN        32

/* Max of CEREBRO_MAX_NODENAME_LEN & CEREBRO_MAX_METRIC_NAME_LEN */
#define CEREBRO_MAX_NAME_LEN               CEREBRO_MAX_NODENAME_LEN

#define CEREBRO_MAX_METRIC_STRING_LEN      256

#define CEREBRO_MAX_LISTENERS              4

#endif /* _CEREBRO_CONSTANTS_H */
