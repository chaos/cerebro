/*****************************************************************************\
 *  $Id: cerebro_clusterlist_util.c,v 1.1 2005-04-21 00:37:31 achu Exp $
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

#include "cerebro_clusterlist_util.h"
#include "error.h"
#include "wrappers.h"

int 
cerebro_clusterlist_parse_filename(char **options, char **filename, char *clusterlist_module_name)
{
  int i = 0;

  assert(options);
  assert(filename);
  assert(clusterlist_module_name);

  *filename = NULL;

  while (options[i] != NULL)
    {
      if (strstr(options[i], "filename"))
	{
	  char *p = strchr(options[i], '=');

	  if (!p)
	    err_exit("%s clusterlist module: filename unspecified", 
                     clusterlist_module_name);

	  p++;
	  if (p == '\0')
	    err_exit("%s clusterlist module: filename unspecified", 
                     clusterlist_module_name);

	  *filename = Strdup(p);
	}
      else
	err_exit("%s clusterlist module: option '%s' unrecognized", 
                 clusterlist_module_name, options[i]);

      i++;
    }

  if (*filename != NULL)
    {
      struct stat buf;

      if (stat(*filename, &buf) < 0)
        err_exit("%s clusterlist module: filename '%s' not found",
                 clusterlist_module_name, *filename);
    }

  return 0;
}

int 
cerebro_clusterlist_copy_nodename(char *node, char *buf, unsigned int buflen, char *clusterlist_module_name)
{
  int len;

  assert(node);
  assert(buf);
  assert(buflen > 0);
  assert(clusterlist_module_name);

  len = strlen(node);

  if ((len + 1) > buflen)
    err_exit("%s(%s:%d): %s clusterlist module: "
             "buflen too small: %d %d", 
             __FILE__, __FUNCTION__, __LINE__,
             clusterlist_module_name, len, buflen);

  strcpy(buf, node);

  return 0;
}
