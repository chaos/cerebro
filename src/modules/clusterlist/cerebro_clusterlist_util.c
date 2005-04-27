/*****************************************************************************\
 *  $Id: cerebro_clusterlist_util.c,v 1.4 2005-04-27 18:11:35 achu Exp $
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
cerebro_clusterlist_parse_filename(char **options, 
                                   char **filename, 
                                   char *clusterlist_module_name)
{
  int i = 0;

  if (!options)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: null options",
                        __FILE__, __FUNCTION__, __LINE__,
                        clusterlist_module_name);
      return -1;
    }

  if (!filename)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: null filename",
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

  *filename = NULL;

  while (options[i] != NULL)
    {
      if (strstr(options[i], "filename"))
	{
	  char *p = strchr(options[i], '=');

	  if (!p)
            {
              cerebro_err_debug("%s clusterlist module: filename unspecified", 
                                clusterlist_module_name);
              return -1;
            }

	  p++;
	  if (p == '\0')
            {
              cerebro_err_debug("%s clusterlist module: filename unspecified", 
                                clusterlist_module_name);
              return -1;
            }

	  if (!(*filename = strdup(p)))
            {
              cerebro_err_debug("%s(%s:%d): %s clusterlist module: strdup: %s",
                                __FILE__, __FUNCTION__, __LINE__,
                                clusterlist_module_name, strerror(errno));
              return -1;
            }
	}
      else
        {
          cerebro_err_debug("%s clusterlist module: option '%s' unrecognized", 
                            clusterlist_module_name, options[i]);
          return -1;
        }

      i++;
    }

  if (*filename != NULL)
    {
      struct stat buf;

      if (stat(*filename, &buf) < 0)
        {
          cerebro_err_debug("%s clusterlist module: filename '%s' not found",
                            clusterlist_module_name, *filename);
          return -1;
        }
    }

  return 0;
}

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
