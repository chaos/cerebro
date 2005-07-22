/*****************************************************************************\
 *  $Id: cerebrod_daemon.c,v 1.10 2005-07-22 17:21:07 achu Exp $
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

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#include "cerebrod_daemon.h"

#include "wrappers.h"

void
cerebrod_daemon_init(void)
{
  /* Based on code in Unix network programming by R. Stevens */
  pid_t pid;
  int i;
 
  pid = Fork();
  if (pid != 0)			/* Terminate Parent */
    exit(0);
 
  setsid();
 
  Signal(SIGHUP, SIG_IGN);
   
  pid = Fork();
  if (pid != 0)			/* Terminate 1st Child */
    exit(0);

  Chdir("/");
 
  Umask(0);
   
  /* Don't use Close() wrapper, we don't want to exit on error */
  for (i = 0; i < 64; i++)
    close(i);			
}
