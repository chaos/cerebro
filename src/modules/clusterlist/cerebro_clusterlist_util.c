/*****************************************************************************\
 *  $Id: cerebro_clusterlist_util.c,v 1.5 2005-04-29 06:33:38 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <assert.h>
#include <errno.h>

#include "cerebro_error.h"
#include "cerebro_clusterlist_util.h"
#include "wrappers.h"

int 
cerebro_clusterlist_copy_nodename(char *node, 
                                  char *buf, 
                                  unsigned int buflen, 
                                  char *clusterlist_module_name)
{
  int len;

  if (!node)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: null node",
                        __FILE__, __FUNCTION__, __LINE__,
                        clusterlist_module_name);
      return -1;
    }

  if (!buf)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: null buf",
                        __FILE__, __FUNCTION__, __LINE__,
                        clusterlist_module_name);
      return -1;
    }

  if (buflen <= 0)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: bad buflen",
                        __FILE__, __FUNCTION__, __LINE__,
                        clusterlist_module_name);
      return -1;
    }

  if (!clusterlist_module_name)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: null "
                        "clusterlist_module_name",
                        __FILE__, __FUNCTION__, __LINE__,
                        clusterlist_module_name);
      return -1;
    }

  len = strlen(node);

  if ((len + 1) > buflen)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: buflen too small: %d %d", 
                        __FILE__, __FUNCTION__, __LINE__,
                        clusterlist_module_name, len, buflen);
      return -1;
    }

  strcpy(buf, node);

  return 0;
}
