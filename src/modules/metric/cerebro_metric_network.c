/*****************************************************************************\
 *  $Id: cerebro_metric_network.c,v 1.2 2007-09-05 18:16:02 chu11 Exp $
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
 *  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */
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

#include "cerebro.h"
#include "cerebro/cerebro_metric_module.h"

#include "debug.h"

#define NETWORK_FILE                "/proc/net/dev"
#define NETWORK_BUFLEN              4096
#define NETWORK_CACHETIMEOUT        5

static u_int64_t cache_bytesin;
static u_int64_t cache_bytesout;
static u_int32_t cache_packetsin;
static u_int32_t cache_packetsout;
static u_int32_t cache_rxerrs;
static u_int32_t cache_txerrs;

static unsigned long int last_read = 0;

/*
 * cerebro_metric_get_network
 *
 * Read network statistics
 */
int
cerebro_metric_get_network(u_int64_t *bytesin,
		       u_int64_t *bytesout,
		       u_int32_t *packetsin,
		       u_int32_t *packetsout,
		       u_int32_t *rxerrs,
		       u_int32_t *txerrs)
{
  int len, fd = -1;
  char buf[NETWORK_BUFLEN];
  struct timeval now;
  u_int64_t total_bytesin = 0;
  u_int64_t total_bytesout = 0;
  u_int32_t total_packetsin = 0;
  u_int32_t total_packetsout = 0;
  u_int32_t total_rxerrs = 0;
  u_int32_t total_txerrs = 0;
  char *parseptr;
  int rv = -1;

  if (gettimeofday(&now, NULL) < 0)
    {
      CEREBRO_DBG(("gettimeofday: %s", strerror(errno)));
      goto cleanup;
    }

  if ((now.tv_sec - last_read) > NETWORK_CACHETIMEOUT)
    {
      if ((fd = open(NETWORK_FILE, O_RDONLY, 0)) < 0)
	{
	  CEREBRO_DBG(("open: %s", strerror(errno)));
	  goto cleanup;
	}
      
      memset(buf, '\0', NETWORK_BUFLEN);
      if ((len = read(fd, buf, NETWORK_BUFLEN)) < 0)
	{
	  CEREBRO_DBG(("read: %s", strerror(errno)));
	  goto cleanup;
	}

      /* skip the first two lines of the file, which are headers, and
       * skip the local loopback interface 
       */
      parseptr = buf;
      if (!(parseptr = strstr(parseptr, "\n")))
	{
	  CEREBRO_DBG(("%s parse error", NETWORK_FILE));
	  goto cleanup;
	}
      parseptr++;
      if (!(parseptr = strstr(parseptr, "\n")))
	{
	  CEREBRO_DBG(("%s parse error", NETWORK_FILE));
	  goto cleanup;
	}
      parseptr++;
      if (!(parseptr = strstr(parseptr, "\n")))
	{
	  CEREBRO_DBG(("%s parse error", NETWORK_FILE));
	  goto cleanup;
	}
      parseptr++;

      while (strstr(parseptr, "\n"))
	{
	  u_int64_t rx_bytes;
	  u_int64_t tx_bytes;
	  u_int32_t rx_packets, rx_errs;
	  u_int32_t tx_packets, tx_errs;
	  u_int32_t temp;
	  char *strptr;
  
	  /* skip the device name */
	  if (!(strptr = strstr(parseptr, ":")))
	    {
	      CEREBRO_DBG(("%s parse error", NETWORK_FILE));
	      goto cleanup;
	    }
	  strptr++;
	  
	  rx_bytes = strtoull(strptr, &strptr, 10);
	  if (!strptr)
	    {
	      CEREBRO_DBG(("%s parse error", NETWORK_FILE));
	      goto cleanup;
	    }
	  rx_packets = strtoul(strptr, &strptr, 10);
	  if (!strptr)
	    {
	      CEREBRO_DBG(("%s parse error", NETWORK_FILE));
	      goto cleanup;
	    }
	  rx_errs = strtoul(strptr, &strptr, 10);
	  if (!strptr)
	    {
	      CEREBRO_DBG(("%s parse error", NETWORK_FILE));
	      goto cleanup;
	    }
 	  temp = strtoul(strptr, &strptr, 10); /* drop */
	  if (!strptr)
	    {
	      CEREBRO_DBG(("%s parse error", NETWORK_FILE));
	      goto cleanup;
	    }
 	  temp = strtoul(strptr, &strptr, 10); /* fifo */
	  if (!strptr)
	    {
	      CEREBRO_DBG(("%s parse error", NETWORK_FILE));
	      goto cleanup;
	    }
 	  temp = strtoul(strptr, &strptr, 10); /* frame */
	  if (!strptr)
	    {
	      CEREBRO_DBG(("%s parse error", NETWORK_FILE));
	      goto cleanup;
	    }
 	  temp = strtoul(strptr, &strptr, 10); /* compressed */
	  if (!strptr)
	    {
	      CEREBRO_DBG(("%s parse error", NETWORK_FILE));
	      goto cleanup;
	    }
 	  temp = strtoul(strptr, &strptr, 10); /* multicast */
	  if (!strptr)
	    {
	      CEREBRO_DBG(("%s parse error", NETWORK_FILE));
	      goto cleanup;
	    }
	  tx_bytes = strtoull(strptr, &strptr, 10);
	  if (!strptr)
	    {
	      CEREBRO_DBG(("%s parse error", NETWORK_FILE));
	      goto cleanup;
	    }
	  tx_packets = strtoul(strptr, &strptr, 10);
	  if (!strptr)
	    {
	      CEREBRO_DBG(("%s parse error", NETWORK_FILE));
	      goto cleanup;
	    }
	  tx_errs = strtoul(strptr, &strptr, 10);
	  if (!strptr)
	    {
	      CEREBRO_DBG(("%s parse error", NETWORK_FILE));
	      goto cleanup;
	    }
		
	  total_bytesin += rx_bytes;
	  total_packetsin += rx_packets;
	  total_rxerrs += rx_errs;
	  total_bytesout += tx_bytes;
	  total_packetsout += tx_packets;
	  total_txerrs += tx_errs;

	  if (!(parseptr = strstr(parseptr, "\n")))
	    {
	      CEREBRO_DBG(("%s parse error", NETWORK_FILE));
	      goto cleanup;
	    }
	  parseptr++;
	}

      cache_bytesin = total_bytesin;
      cache_bytesout = total_bytesout;
      cache_packetsin = total_packetsin;
      cache_packetsout = total_packetsout;
      cache_rxerrs = total_rxerrs;
      cache_txerrs = total_txerrs;

      last_read = now.tv_sec;
    }

  if (bytesin)
    *bytesin = cache_bytesin;
  if (bytesout)
    *bytesout = cache_bytesout;
  if (packetsin)
    *packetsin = cache_packetsin;
  if (packetsout)
    *packetsout = cache_packetsout;
  if (rxerrs)
    *rxerrs = cache_rxerrs;
  if (txerrs)
    *txerrs = cache_txerrs;

  rv = 0;
 cleanup:
  if (fd >= 0)
    close(fd);
  return rv;
}
