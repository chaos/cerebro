/*****************************************************************************\
 *  $Id: cerebro_clusterlist_util.c,v 1.10 2005-05-04 20:58:09 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */

#include "cerebro_error.h"
#include "cerebro_clusterlist_util.h"

int 
cerebro_clusterlist_copy_nodename(const char *node, 
                                  char *buf, 
                                  unsigned int buflen)
{
  int len;

  if (!node)
    {
      cerebro_err_debug_module("%s(%s:%d): node null",
			       __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!buf)
    {
      cerebro_err_debug_module("%s(%s:%d): buf null",
			       __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!buflen)
    {
      cerebro_err_debug_module("%s(%s:%d): buflen invalid",
			       __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  len = strlen(node);
  if ((len + 1) > buflen)
    {
      cerebro_err_debug_module("%s(%s:%d): buflen too small: len=%d buflen=%d", 
			       __FILE__, __FUNCTION__, __LINE__,
			       len, buflen);
      return -1;
    }

  strcpy(buf, node);

  return 0;
}
