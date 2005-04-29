/*****************************************************************************\
 *  $Id: module.c,v 1.1 2005-04-29 06:33:38 achu Exp $
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

#include "module.h"
#include "cerebro_error.h"
#include "wrappers.h"

int 
module_parse_filename(char **options, 
		      char **filename, 
		      char *module_name)
{
  int i = 0;

  if (!options)
    {
      cerebro_err_debug("%s(%s:%d): %s module: null options",
                        __FILE__, __FUNCTION__, __LINE__,
                        module_name);
      return -1;
    }

  if (!filename)
    {
      cerebro_err_debug("%s(%s:%d): %s module: null filename",
                        __FILE__, __FUNCTION__, __LINE__,
                        module_name);
      return -1;
    }

  if (!module_name)
    {
      cerebro_err_debug("%s(%s:%d): %s module: null "
                        "module_name",
                        __FILE__, __FUNCTION__, __LINE__,
                        module_name);
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
              cerebro_err_debug("%s module: filename unspecified", 
                                module_name);
              return -1;
            }

	  p++;
	  if (p == '\0')
            {
              cerebro_err_debug("%s module: filename unspecified", 
                                module_name);
              return -1;
            }

	  if (!(*filename = strdup(p)))
            {
              cerebro_err_debug("%s(%s:%d): %s module: strdup: %s",
                                __FILE__, __FUNCTION__, __LINE__,
                                module_name, strerror(errno));
              return -1;
            }
	}
      else
        {
          cerebro_err_debug("%s module: option '%s' unrecognized", 
                            module_name, options[i]);
          return -1;
        }

      i++;
    }

  if (*filename != NULL)
    {
      struct stat buf;

      if (stat(*filename, &buf) < 0)
        {
          cerebro_err_debug("%s module: filename '%s' not found",
                            module_name, *filename);
          return -1;
        }
    }

  return 0;
}
