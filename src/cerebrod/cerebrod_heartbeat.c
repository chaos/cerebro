/*****************************************************************************\
 *  $Id: cerebrod_heartbeat.c,v 1.21 2005-04-26 17:31:35 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <netinet/in.h>
#include <assert.h>
#include <errno.h>

#include "cerebrod_heartbeat_protocol.h"
#include "cerebrod_heartbeat.h"
#include "cerebrod_config.h"
#include "wrappers.h"

extern struct cerebrod_config conf;
#ifndef NDEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* NDEBUG */

void
cerebrod_heartbeat_dump(struct cerebrod_heartbeat *hb)
{
#ifndef NDEBUG
  assert(hb);

  if (conf.debug)
    {
      int ret;

      ret = Pthread_mutex_trylock(&debug_output_mutex);
      if (ret != EBUSY)
	{
	  fprintf(stderr, "cerebrod_heartbeat_dump: "
		  "debug_output_mutex not locked");
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
#endif /* NDEBUG */
}
