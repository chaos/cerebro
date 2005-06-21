/*****************************************************************************\
 *  $Id: cerebrod_heartbeat.c,v 1.39 2005-06-21 22:29:07 achu Exp $
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

extern struct cerebrod_config conf;
#if CEREBRO_DEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* CEREBRO_DEBUG */

/*
 * cerebrod_heartbeat_destroy
 *
 * destroy a heartbeat packet
 */
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
  char *buf;

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
          fprintf(stderr, 
                  "* %s: metric_value_type = %d, metric_value_len = %d ",
                  hb->metrics[i]->metric_name, 
                  hb->metrics[i]->metric_value_type,
                  hb->metrics[i]->metric_value_len);

          switch(hb->metrics[i]->metric_value_type)
            {
            case CEREBRO_METRIC_VALUE_TYPE_NONE:
              break;
            case CEREBRO_METRIC_VALUE_TYPE_INT32:
              fprintf(stderr, "metric_value = %d", 
                      *((int32_t *)hb->metrics[i]->metric_value));
              break;
            case CEREBRO_METRIC_VALUE_TYPE_U_INT32:
              fprintf(stderr, "metric_value = %u", 
                      *((u_int32_t *)hb->metrics[i]->metric_value));
              break;
            case CEREBRO_METRIC_VALUE_TYPE_FLOAT:
              fprintf(stderr, "metric_value = %f", 
                      *((float *)hb->metrics[i]->metric_value));
              break;
            case CEREBRO_METRIC_VALUE_TYPE_DOUBLE:
              fprintf(stderr, "metric_value = %f", 
                      *((double *)hb->metrics[i]->metric_value));
              break;
            case CEREBRO_METRIC_VALUE_TYPE_STRING:
              /* Watch for NUL termination */
              buf = Malloc(hb->metrics[i]->metric_value_len + 1);
              memset(buf, '\0', hb->metrics[i]->metric_value_len + 1);
              memcpy(buf, 
                     hb->metrics[i]->metric_value, 
                     hb->metrics[i]->metric_value_len);
              fprintf(stderr, "metric_value = %s", buf);
              Free(buf);
              break;
            case CEREBRO_METRIC_VALUE_TYPE_RAW:
              /* Don't output raw data */
            default:
              break;
            }
          fprintf(stderr, "\n");
        }
      fprintf(stderr, "**************************************\n");
    }
#endif /* CEREBRO_DEBUG */
}
