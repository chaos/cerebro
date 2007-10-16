/*****************************************************************************\
 *  $Id: cerebrod_message.c,v 1.3 2007-10-16 22:43:15 chu11 Exp $
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

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include "cerebro.h"

#include "cerebrod_config.h"
#include "cerebrod_message.h"

#include "wrappers.h"

#if CEREBRO_DEBUG
extern struct cerebrod_config conf;
extern pthread_mutex_t debug_output_mutex;
#endif /* CEREBRO_DEBUG */

void
cerebrod_message_destroy(struct cerebrod_message *msg)
{
  int i;

  if (!msg)
    return;

  for (i = 0; i < msg->metrics_len; i++)
    {
      Free(msg->metrics[i]->metric_value);
      Free(msg->metrics[i]);
    }

  Free(msg->metrics);
  Free(msg);
}

void
cerebrod_message_dump(struct cerebrod_message *msg)
{
#if CEREBRO_DEBUG
  int i;
#if !WITH_CEREBROD_NO_THREADS
  int rv;
#endif /* !WITH_CEREBROD_NO_THREADS */

  assert(msg);

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
  fprintf(stderr, "* Cerebrod Message:\n");     
  fprintf(stderr, "* -------------------\n");
  fprintf(stderr, "* version: %d\n", msg->version);
  fprintf(stderr, "* nodename: \"%s\"\n", msg->nodename);
  fprintf(stderr, "* metrics_len: %d\n", msg->metrics_len);

  for (i = 0; i < msg->metrics_len; i++)
    {
      char *buf;
      
      fprintf(stderr, "* %s: metric type = %d, len = %d ",
              msg->metrics[i]->metric_name, 
              msg->metrics[i]->metric_value_type,
              msg->metrics[i]->metric_value_len);
      
      switch(msg->metrics[i]->metric_value_type)
        {
        case CEREBRO_DATA_VALUE_TYPE_NONE:
          break;
        case CEREBRO_DATA_VALUE_TYPE_INT32:
          fprintf(stderr, "value = %d", 
                  *((int32_t *)msg->metrics[i]->metric_value));
          break;
        case CEREBRO_DATA_VALUE_TYPE_U_INT32:
          fprintf(stderr, "value = %u", 
                  *((u_int32_t *)msg->metrics[i]->metric_value));
          break;
        case CEREBRO_DATA_VALUE_TYPE_FLOAT:
          fprintf(stderr, "value = %f", 
                      *((float *)msg->metrics[i]->metric_value));
          break;
        case CEREBRO_DATA_VALUE_TYPE_DOUBLE:
          fprintf(stderr, "value = %f", 
                  *((double *)msg->metrics[i]->metric_value));
          break;
        case CEREBRO_DATA_VALUE_TYPE_STRING:
          /* Watch for NUL termination */
          buf = Malloc(msg->metrics[i]->metric_value_len + 1);
          memset(buf, '\0', msg->metrics[i]->metric_value_len + 1);
          memcpy(buf, 
                 msg->metrics[i]->metric_value, 
                 msg->metrics[i]->metric_value_len);
          fprintf(stderr, "value = %s", buf);
          Free(buf);
          break;
#if SIZEOF_LONG == 4
        case CEREBRO_DATA_VALUE_TYPE_INT64:
          fprintf(stderr, "value = %lld", 
                  *((int64_t *)msg->metrics[i]->metric_value));
          break;
        case CEREBRO_DATA_VALUE_TYPE_U_INT64:
          fprintf(stderr, "value = %llu", 
                  *((u_int64_t *)msg->metrics[i]->metric_value));
          break;
#else  /* SIZEOF_LONG == 8 */
        case CEREBRO_DATA_VALUE_TYPE_INT64:
          fprintf(stderr, "value = %ld", 
                  *((int64_t *)msg->metrics[i]->metric_value));
          break;
        case CEREBRO_DATA_VALUE_TYPE_U_INT64:
          fprintf(stderr, "value = %lu", 
                  *((u_int64_t *)msg->metrics[i]->metric_value));
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
