/*****************************************************************************\
 *  $Id: cerebrod_heartbeat.c,v 1.49 2006-11-08 00:34:04 chu11 Exp $
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
#include <assert.h>
#include <errno.h>

#include "cerebro.h"

#include "cerebrod_config.h"
#include "cerebrod_heartbeat.h"

#include "wrappers.h"

#if CEREBRO_DEBUG
extern struct cerebrod_config conf;
extern pthread_mutex_t debug_output_mutex;
#endif /* CEREBRO_DEBUG */

void
cerebrod_heartbeat_destroy(struct cerebrod_heartbeat *hb)
{
  int i;

  if (!hb)
    return;

  for (i = 0; i < hb->metrics_len; i++)
    {
      Free(hb->metrics[i]->metric_value);
      Free(hb->metrics[i]);
    }

  Free(hb->metrics);
  Free(hb);
}

void
cerebrod_heartbeat_dump(struct cerebrod_heartbeat *hb)
{
#if CEREBRO_DEBUG
  int i;
#if !WITH_CEREBROD_NO_THREADS
  int rv;
#endif /* !WITH_CEREBROD_NO_THREADS */

  assert(hb);

  if (!conf.debug)
    return;

#if !WITH_CEREBROD_NO_THREADS
  rv = Pthread_mutex_trylock(&debug_output_mutex);
  if (rv != EBUSY)
    {
      fprintf(stderr, "(%s, %s, %d): mutex not locked: rv=%d",
              __FILE__, __FUNCTION__, __LINE__, rv);
      exit(1);
    }
#endif /* !WITH_CEREBROD_NO_THREADS */
      
  fprintf(stderr, "**************************************\n");
  fprintf(stderr, "* Cerebrod Heartbeat:\n");     
  fprintf(stderr, "* -------------------\n");
  fprintf(stderr, "* version: %d\n", hb->version);
  fprintf(stderr, "* nodename: \"%s\"\n", hb->nodename);
  fprintf(stderr, "* metrics_len: %d\n", hb->metrics_len);

  for (i = 0; i < hb->metrics_len; i++)
    {
      char *buf;
      
      fprintf(stderr, "* %s: metric type = %d, len = %d ",
              hb->metrics[i]->metric_name, 
              hb->metrics[i]->metric_value_type,
              hb->metrics[i]->metric_value_len);
      
      switch(hb->metrics[i]->metric_value_type)
        {
        case CEREBRO_DATA_VALUE_TYPE_NONE:
          break;
        case CEREBRO_DATA_VALUE_TYPE_INT32:
          fprintf(stderr, "value = %d", 
                  *((int32_t *)hb->metrics[i]->metric_value));
          break;
        case CEREBRO_DATA_VALUE_TYPE_U_INT32:
          fprintf(stderr, "value = %u", 
                  *((u_int32_t *)hb->metrics[i]->metric_value));
          break;
        case CEREBRO_DATA_VALUE_TYPE_FLOAT:
          fprintf(stderr, "value = %f", 
                      *((float *)hb->metrics[i]->metric_value));
          break;
        case CEREBRO_DATA_VALUE_TYPE_DOUBLE:
          fprintf(stderr, "value = %f", 
                  *((double *)hb->metrics[i]->metric_value));
          break;
        case CEREBRO_DATA_VALUE_TYPE_STRING:
          /* Watch for NUL termination */
          buf = Malloc(hb->metrics[i]->metric_value_len + 1);
          memset(buf, '\0', hb->metrics[i]->metric_value_len + 1);
          memcpy(buf, 
                 hb->metrics[i]->metric_value, 
                 hb->metrics[i]->metric_value_len);
          fprintf(stderr, "value = %s", buf);
          Free(buf);
          break;
#if SIZEOF_LONG == 4
        case CEREBRO_DATA_VALUE_TYPE_INT64:
          fprintf(stderr, "value = %lld", 
                  *((int64_t *)hb->metrics[i]->metric_value));
          break;
        case CEREBRO_DATA_VALUE_TYPE_U_INT64:
          fprintf(stderr, "value = %llu", 
                  *((u_int64_t *)hb->metrics[i]->metric_value));
          break;
#else  /* SIZEOF_LONG == 8 */
        case CEREBRO_DATA_VALUE_TYPE_INT64:
          fprintf(stderr, "value = %ld", 
                  *((int64_t *)hb->metrics[i]->metric_value));
          break;
        case CEREBRO_DATA_VALUE_TYPE_U_INT64:
          fprintf(stderr, "value = %lu", 
                  *((u_int64_t *)hb->metrics[i]->metric_value));
          break;
#endif /* SIZEOF_LONG == 8 */
        default:
          break;
        }
      fprintf(stderr, "\n");
    }
  fprintf(stderr, "**************************************\n");
#endif /* CEREBRO_DEBUG */
}
