/*****************************************************************************\
 *  $Id: cerebrod_heartbeat.c,v 1.32 2005-06-08 15:32:01 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include "cerebro.h"

#include "cerebrod_heartbeat.h"
#include "cerebrod_config.h"
#include "wrappers.h"

extern struct cerebrod_config conf;
#if CEREBRO_DEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* CEREBRO_DEBUG */

void
cerebrod_heartbeat_destroy(struct cerebrod_heartbeat *hb)
{
  int i;

  assert(hb);

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
  assert(hb);

  if (conf.debug)
    {
      int i, rv;

      rv = Pthread_mutex_trylock(&debug_output_mutex);
      if (rv != EBUSY)
	{
	  fprintf(stderr, "%s(%s:%d): mutex not locked: rv=%d",
                  __FILE__, __FUNCTION__, __LINE__, rv);
	  exit(1);
	}
      
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Cerebrod Heartbeat:\n");     
      fprintf(stderr, "* -------------------\n");
      fprintf(stderr, "* version: %d\n", hb->version);
      fprintf(stderr, "* nodename: \"%s\"\n", hb->nodename);
      fprintf(stderr, "* metrics_len: %d\n", hb->metrics_len);
      for (i = 0; i < hb->metrics_len; i++)
        {
          fprintf(stderr, "* %s: metric_value_type = %d, metric_value_len = %d ", 
                  hb->metrics[i]->metric_name, 
                  hb->metrics[i]->metric_value_type,
                  hb->metrics[i]->metric_value_len);

          switch(hb->metrics[i]->metric_value_type)
            {
            case CEREBRO_METRIC_VALUE_TYPE_NONE:
              break;
            case CEREBRO_METRIC_VALUE_TYPE_INT32:
              fprintf(stderr, "metric_value = %d\n", 
                      *((int32_t *)hb->metrics[i]->metric_value));
              break;
            case CEREBRO_METRIC_VALUE_TYPE_UNSIGNED_INT32:
              fprintf(stderr, "metric_value = %u\n", 
                      *((u_int32_t *)hb->metrics[i]->metric_value));
              break;
            case CEREBRO_METRIC_VALUE_TYPE_FLOAT:
              fprintf(stderr, "metric_value = %f\n", 
                      *((float *)hb->metrics[i]->metric_value));
              break;
            case CEREBRO_METRIC_VALUE_TYPE_DOUBLE:
              fprintf(stderr, "metric_value = %f\n", 
                      *((double *)hb->metrics[i]->metric_value));
              break;
            case CEREBRO_METRIC_VALUE_TYPE_STRING:
              fprintf(stderr, "metric_value = %s\n", 
                      (char *)hb->metrics[i]->metric_value);
              break;
            case CEREBRO_METRIC_VALUE_TYPE_RAW:
            default:
              fprintf(stderr, "\n");
              break;
            }
        }
      fprintf(stderr, "**************************************\n");
    }
#endif /* CEREBRO_DEBUG */
}
