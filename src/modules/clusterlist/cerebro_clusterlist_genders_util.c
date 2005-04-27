/*****************************************************************************\
 *  $Id: cerebro_clusterlist_genders_util.c,v 1.5 2005-04-27 18:11:35 achu Exp $
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

#include <genders.h>

#include "cerebro_error.h"
#include "wrappers.h"

int 
cerebro_clusterlist_genders_setup(genders_t *handle, 
                                  char *file, 
                                  char *clusterlist_module_name)
{
  if (!handle)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: null handle",
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

  *handle = NULL;

  if (!(*handle = genders_handle_create()))
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "genders_handle_create",
                        clusterlist_module_name,
                        __FILE__, __FUNCTION__, __LINE__);
      goto cleanup;
    }

  if (genders_load_data(*handle, file) < 0)
    {
      if (genders_errnum(*handle) == GENDERS_ERR_OPEN)
	{
	  if (file)
            {
              cerebro_err_debug("%s clusterlist module: genders database '%s' "
                               "cannot be opened", 
                               clusterlist_module_name, file);
              goto cleanup;
            }
	  else
            {
              cerebro_err_debug("%s clusterlist module: genders database '%s' "
                               "cannot be opened", 
                               clusterlist_module_name, GENDERS_DEFAULT_FILE);
              goto cleanup;
            }
	}
      else
        {
          cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                           "genders_load_data: %s",
                           __FILE__, __FUNCTION__, __LINE__, 
                           clusterlist_module_name, genders_errormsg(*handle));
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
cerebro_clusterlist_genders_cleanup(genders_t *handle, 
                                    char **file, 
                                    char *clusterlist_module_name)
{
  if (!handle)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: null handle",
                        __FILE__, __FUNCTION__, __LINE__,
                        clusterlist_module_name);
      return -1;
    }

  if (!file)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: null file",
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

  if (genders_handle_destroy(*handle) < 0)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "genders_handle_destroy: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        clusterlist_module_name, genders_errormsg(*handle));
      return -1;
    }

  free(*file);
  *handle = NULL;
  *file = NULL;

  return 0;
}

int
cerebro_clusterlist_genders_get_all_nodes(genders_t handle, 
                                          char **nodes, 
                                          unsigned int nodeslen, 
                                          char *clusterlist_module_name)
{
  char **nodelist = NULL;
  int i, nodelistlen, numnodes;
  
  if (!handle)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: null handle",
                        __FILE__, __FUNCTION__, __LINE__,
                        clusterlist_module_name);
      return -1;
    }

  if (!nodes)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: null nodes",
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

  if ((nodelistlen = genders_nodelist_create(handle, &nodelist)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "genders_nodelist_create: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        clusterlist_module_name, genders_errormsg(handle));
      goto cleanup;
    }
  
  if ((numnodes = genders_getnodes(handle, nodelist, nodelistlen, NULL, NULL)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "genders_getnodes: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        clusterlist_module_name, genders_errormsg(handle));
      goto cleanup;
    }

  if (numnodes > nodeslen)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: nodeslen too small", 
                        __FILE__, __FUNCTION__, __LINE__,
                        clusterlist_module_name);
      goto cleanup;
    }

  for (i = 0; i < numnodes; i++)
    nodes[i] = Strdup(nodelist[i]);

  if (genders_nodelist_destroy(handle, nodelist) < 0)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "genders_nodelist_destroy: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        clusterlist_module_name, genders_errormsg(handle));
      goto cleanup;
    }
  
  return numnodes;

 cleanup:
  if (nodelist)
    genders_nodelist_destroy(handle, nodelist);
  return -1;
}

int 
cerebro_clusterlist_genders_numnodes(genders_t handle, 
                                     char *clusterlist_module_name)
{
  int ret;

  if (!handle)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: null handle",
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

  if ((ret = genders_getnumnodes(handle)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): %s clusterlist module: "
                        "genders_getnumnodes: %s",
                        __FILE__, __FUNCTION__, __LINE__,
                        clusterlist_module_name, genders_errormsg(handle));
      return -1;
    }

  return ret;
}
