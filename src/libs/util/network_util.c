/*****************************************************************************\
 *  $Id: network_util.c,v 1.3 2005-07-22 17:21:07 achu Exp $
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
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <sys/select.h>
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else  /* !TIME_WITH_SYS_TIME */
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else /* !HAVE_SYS_TIME_H */
#  include <time.h>
# endif /* !HAVE_SYS_TIME_H */
#endif /* !TIME_WITH_SYS_TIME */
#include <sys/types.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <errno.h>

#include "cerebro.h"

#include "network_util.h"

#include "debug.h"

int
receive_data(int fd,
             unsigned int bytes_to_read,
             char *buf,
             unsigned int buflen,
             unsigned int timeout_len,
             unsigned int *errnum)
{
  int bytes_read = 0;
  
  if (!buf 
      || !buflen 
      || !bytes_to_read 
      || buflen < bytes_to_read
      || !timeout_len)
    {
      CEREBRO_DBG(("invalid parameters"));
      if (errnum)
        *errnum = CEREBRO_ERR_INTERNAL;
      goto cleanup;
    }
  
  memset(buf, '\0', buflen);
  
  while (bytes_read < bytes_to_read)
    {
      fd_set rfds;
      struct timeval tv;
      int num;
      
      tv.tv_sec = timeout_len;
      tv.tv_usec = 0;
      
      FD_ZERO(&rfds);
      FD_SET(fd, &rfds);
      
      if ((num = select(fd + 1, &rfds, NULL, NULL, &tv)) < 0)
        {
          CEREBRO_DBG(("select: %s", strerror(errno)));
          if (errnum)
            *errnum = CEREBRO_ERR_INTERNAL;
          goto cleanup;
        }

      if (!num)
        {
          /* Timed out.  If atleast some bytes were read, return data
           * back to the caller to let them possibly unmarshall the
           * data.  Its possible we are expecting more bytes than the
           * client is sending, perhaps because we are using a
           * different protocol version.  This will allow the server
           * to return a invalid version number back to the user.
           */
          if (!bytes_read)
            {
              if (errnum)
                *errnum = CEREBRO_ERR_PROTOCOL_TIMEOUT;
              goto cleanup;
            }
          else
            goto out;
        }

      if (FD_ISSET(fd, &rfds))
        {
          int n;

          /* Don't use fd_read_n b/c it loops until exactly
           * bytes_to_read is read.  Due to version incompatability or
           * error packets, we may want to read a smaller packet.
           */
          if ((n = read(fd, buf + bytes_read, bytes_to_read - bytes_read)) < 0)
            {
              CEREBRO_DBG(("read: %s", strerror(errno)));
              if (errnum)
                *errnum = CEREBRO_ERR_INTERNAL;
              goto cleanup;
            }
          
          /* Pipe closed */
          if (!n)
            {
              if (bytes_read)
                goto out;
              
              if (errnum)
                *errnum = CEREBRO_ERR_PROTOCOL;
              goto cleanup;
            }
          
          bytes_read += n;
        }
      else
        {
          CEREBRO_DBG(("num != 0 but fd not set"));
          if (errnum)
            *errnum = CEREBRO_ERR_INTERNAL;
          goto cleanup;
        }
    }
  
 out:
  return bytes_read;
  
 cleanup:
  return -1;
}
