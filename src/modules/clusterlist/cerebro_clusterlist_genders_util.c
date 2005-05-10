/*****************************************************************************\
 *  $Id: cerebro_clusterlist_genders_util.c,v 1.14 2005-05-10 17:55:27 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <errno.h>

#include <genders.h>

#include "cerebro/cerebro_error.h"

int 
cerebro_clusterlist_genders_setup(genders_t *handle)
{
  if (!handle)
    {
      cerebro_err_debug_module("%s(%s:%d): handle null",
			       __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }


  *handle = NULL;

  if (!(*handle = genders_handle_create()))
    {
      cerebro_err_debug_module("%s(%s:%d): genders_handle_create",
			       __FILE__, __FUNCTION__, __LINE__);
      goto cleanup;
    }

  if (genders_load_data(*handle, NULL) < 0)
    {
      if (genders_errnum(*handle) == GENDERS_ERR_OPEN)
	{
	  cerebro_err_debug_module("genders database '%s' cannot be opened", 
				   GENDERS_DEFAULT_FILE);
	  goto cleanup;
	}
      else
        {
          cerebro_err_debug_module("%s(%s:%d): genders_load_data: %s",
				   __FILE__, __FUNCTION__, __LINE__, 
				   genders_errormsg(*handle));
          goto cleanup;
        }
    }

  return 0;

 cleanup:
  if (*handle)
    genders_handle_destroy(*handle);
  *handle = NULL;
  return -1;
}

int
cerebro_clusterlist_genders_cleanup(genders_t *handle)
{
  if (!handle)
    {
      cerebro_err_debug_module("%s(%s:%d): handle null",
			       __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (genders_handle_destroy(*handle) < 0)
    {
      cerebro_err_debug_module("%s(%s:%d): genders_handle_destroy: %s",
			       __FILE__, __FUNCTION__, __LINE__,
			       genders_errormsg(*handle));
      return -1;
    }

  *handle = NULL;

  return 0;
}

int 
cerebro_clusterlist_genders_numnodes(genders_t handle)
{
  int num;

  if (!handle)
    {
      cerebro_err_debug_module("%s(%s:%d): handle null",
			       __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if ((num = genders_getnumnodes(handle)) < 0)
    {
      cerebro_err_debug_module("%s(%s:%d): genders_getnumnodes: %s",
			       __FILE__, __FUNCTION__, __LINE__,
			       genders_errormsg(handle));
      return -1;
    }

  return num;
}

int
cerebro_clusterlist_genders_get_all_nodes(genders_t handle, 
                                          char **nodes, 
                                          unsigned int nodeslen)
{
  char **nodelist = NULL;
  int i, nodelistlen, numnodes;
  
  if (!handle)
    {
      cerebro_err_debug_module("%s(%s:%d): handle null",
			       __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!nodes)
    {
      cerebro_err_debug_module("%s(%s:%d): nodes null",
			       __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if ((nodelistlen = genders_nodelist_create(handle, &nodelist)) < 0)
    {
      cerebro_err_debug_module("%s(%s:%d): genders_nodelist_create: %s",
			       __FILE__, __FUNCTION__, __LINE__,
			       genders_errormsg(handle));
      goto cleanup;
    }
  
  if ((numnodes = genders_getnodes(handle, 
				   nodelist, 
				   nodelistlen, 
				   NULL, 
				   NULL)) < 0)
    {
      cerebro_err_debug_module("%s(%s:%d): genders_getnodes: %s",
			       __FILE__, __FUNCTION__, __LINE__,
			       genders_errormsg(handle));
      goto cleanup;
    }

  if (numnodes > nodeslen)
    {
      cerebro_err_debug_module("%s(%s:%d): nodeslen too small", 
			       __FILE__, __FUNCTION__, __LINE__);
      goto cleanup;
    }

  for (i = 0; i < numnodes; i++)
    {
      if (!(nodes[i] = strdup(nodelist[i])))
	{
	  cerebro_err_debug_module("%s(%s:%d): strdup: %s",
                                   __FILE__, __FUNCTION__, __LINE__,
                                   strerror(errno));
	}
    }

  if (genders_nodelist_destroy(handle, nodelist) < 0)
    {
      cerebro_err_debug_module("%s(%s:%d): genders_nodelist_destroy: %s",
			       __FILE__, __FUNCTION__, __LINE__,
			       genders_errormsg(handle));
      goto cleanup;
    }
  
  return numnodes;

 cleanup:
  if (nodelist)
    genders_nodelist_destroy(handle, nodelist);
  return -1;
}

