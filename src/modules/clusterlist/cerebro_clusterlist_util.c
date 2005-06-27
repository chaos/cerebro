/*****************************************************************************\
 *  $Id: cerebro_clusterlist_util.c,v 1.13 2005-06-27 04:44:49 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */

#include "cerebro_clusterlist_util.h"

#include "debug.h"

int 
cerebro_clusterlist_copy_nodename(const char *node, 
                                  char *buf, 
                                  unsigned int buflen)
{
  int len;

  if (!node)
    {
      CEREBRO_DBG(("node null"));
      return -1;
    }

  if (!buf)
    {
      CEREBRO_DBG(("buf null"));
      return -1;
    }

  if (!buflen)
    {
      CEREBRO_DBG(("buflen invalid"));
      return -1;
    }

  len = strlen(node);
  if ((len + 1) > buflen)
    {
      CEREBRO_DBG(("buflen too small: len=%d buflen=%d", len, buflen));
      return -1;
    }

  strcpy(buf, node);

  return 0;
}
