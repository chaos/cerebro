/*****************************************************************************\
 *  $Id: cerebrod_clusterlist_genders_util.c,v 1.4 2005-03-30 05:41:45 achu Exp $
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

#include "cerebrod_clusterlist.h"
#include "cerebrod_error.h"
#include "wrappers.h"

int 
cerebrod_clusterlist_genders_init(genders_t *handle, char *file)
{
  char *clusterlist_module_name = cerebrod_clusterlist_module_name();

  assert(handle);

  if (!(*handle = genders_handle_create()))
    cerebrod_err_exit("%s(%s:%d): genders_handle_create",
		      __FILE__, __FUNCTION__, __LINE__);

  if (genders_load_data(*handle, file) < 0)
    {
      if (genders_errnum(*handle) == GENDERS_ERR_OPEN)
	{
	  if (file)
	    cerebrod_err_exit("%s clusterlist module: genders database '%s' "
			      "cannot be opened", 
			      clusterlist_module_name, file);
	  else
	    cerebrod_err_exit("%s clusterlist module: genders database '%s' "
			      "cannot be opened", 
			      clusterlist_module_name, GENDERS_DEFAULT_FILE);
	}
      else
	cerebrod_err_exit("%s(%s:%d): %s clusterlist module: "
			  "genders_load_data: %s",
			  __FILE__, __FUNCTION__, __LINE__, 
			  clusterlist_module_name, genders_errormsg(*handle));
    }

  return 0;
}

int
cerebrod_clusterlist_genders_finish(genders_t *handle, char **file)
{
  char *clusterlist_module_name = cerebrod_clusterlist_module_name();

  assert(handle && file);

  if (genders_handle_destroy(*handle) < 0)
    cerebrod_err_exit("%s(%s:%d): %s clusterlist module: "
		      "genders_handle_destroy: %s",
		      __FILE__, __FUNCTION__, __LINE__,
		      clusterlist_module_name, genders_errormsg(*handle));

  Free(*file);
  *handle = NULL;
  *file = NULL;

  return 0;
}

int
cerebrod_clusterlist_genders_get_all_nodes(genders_t handle, char **nodes, unsigned int nodeslen)
{
  char **nodelist;
  int i, nodelistlen, numnodes;
  char *clusterlist_module_name = cerebrod_clusterlist_module_name();
  
  assert(handle);
  assert(nodes);

  if ((nodelistlen = genders_nodelist_create(handle, &nodelist)) < 0)
    cerebrod_err_exit("%s(%s:%d): %s clusterlist module: "
		      "genders_nodelist_create: %s",
		      __FILE__, __FUNCTION__, __LINE__,
		      clusterlist_module_name, genders_errormsg(handle));
  
  if ((numnodes = genders_getnodes(handle, nodelist, nodelistlen, NULL, NULL)) < 0)
    cerebrod_err_exit("%s(%s:%d): %s clusterlist module: "
		      "genders_getnodes: %s",
		      __FILE__, __FUNCTION__, __LINE__,
		      clusterlist_module_name, genders_errormsg(handle));

  if (numnodes > nodeslen)
    cerebrod_err_exit("%s(%s:%d): %s clusterlist module: "
		      "nodeslen too small", 
		      __FILE__, __FUNCTION__, __LINE__,
		      clusterlist_module_name);

  for (i = 0; i < numnodes; i++)
    nodes[i] = Strdup(nodelist[i]);

  if (genders_nodelist_destroy(handle, nodelist) < 0)
    cerebrod_err_exit("%s(%s:%d): %s clusterlist module: "
		      "genders_nodelist_destroy: %s",
		      __FILE__, __FUNCTION__, __LINE__,
		      clusterlist_module_name, genders_errormsg(handle));
  
  return numnodes;
}

int 
cerebrod_clusterlist_genders_numnodes(genders_t handle)
{
  int ret;
  char *clusterlist_module_name = cerebrod_clusterlist_module_name();

  assert(handle);

  if ((ret = genders_getnumnodes(handle)) < 0)
    cerebrod_err_exit("%s(%s:%d): %s clusterlist module: "
		      "genders_getnumnodes: %s",
		      __FILE__, __FUNCTION__, __LINE__,
		      clusterlist_module_name, genders_errormsg(handle));

  return ret;
}
