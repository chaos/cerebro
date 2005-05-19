/*****************************************************************************\
 *  $Id: cerebrod_heartbeat.c,v 1.26 2005-05-19 23:38:46 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include "cerebro/cerebrod_heartbeat_protocol.h"

#include "cerebrod_heartbeat.h"
#include "cerebrod_config.h"
#include "wrappers.h"

extern struct cerebrod_config conf;
#if CEREBRO_DEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* CEREBRO_DEBUG */

void
cerebrod_heartbeat_dump(struct cerebrod_heartbeat *hb)
{
#if CEREBRO_DEBUG
  assert(hb);

  if (conf.debug)
    {
      int rv;

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
      fprintf(stderr, "* starttime: %u\n", hb->starttime);
      fprintf(stderr, "* boottime: %u\n", hb->boottime);
      fprintf(stderr, "**************************************\n");
    }
#endif /* CEREBRO_DEBUG */
}
