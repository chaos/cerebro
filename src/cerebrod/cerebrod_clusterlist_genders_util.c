/*****************************************************************************\
 *  $Id: cerebrod_clusterlist_genders_util.c,v 1.2 2005-03-22 01:34:54 achu Exp $
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

#include "cerebrod.h"
#include "cerebrod_clusterlist.h"
#include "error.h"
#include "wrappers.h"

int 
cerebrod_clusterlist_genders_init(genders_t *handle, char *file)
{
  char *clusterlist_module_name = cerebrod_clusterlist_module_name();

  assert(handle);

  if (!(*handle = genders_handle_create()))
    err_exit("genders_clusterlist_init: genders_handle_create");

  if (genders_load_data(*handle, file) < 0)
    {
      if (genders_errnum(*handle) == GENDERS_ERR_OPEN)
	{
	  if (file)
	    err_exit("%s clusterlist module: genders database '%s' cannot be opened", 
                     clusterlist_module_name, file);
	  else
	    err_exit("%s clusterlist module: genders database '%s' cannot be opened", 
                     clusterlist_module_name, GENDERS_DEFAULT_FILE);
	}
      else
	err_exit("%s clusterlist module: cerebrod_clusterlist_genders_init: genders_load_data: %s",
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
    err_exit("%s clusterlist module: cerebrod_clusterlist_genders_finish: genders_handle_destroy: %s",
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
    err_exit("%s clusterlist module: cerebrod_clusterlist_genders_get_all_nodes: genders_nodelist_create: %s",
	     clusterlist_module_name, genders_errormsg(handle));
  
  if ((numnodes = genders_getnodes(handle, nodelist, nodelistlen, NULL, NULL)) < 0)
    err_exit("%s clusterlist module: cerebrod_clusterlist_genders_get_all_nodes: genders_getnodes: %s",
	     clusterlist_module_name, genders_errormsg(handle));

  if (numnodes > nodeslen)
    err_exit("%s clusterlist module: cerebrod_clusterlist_genders_get_all_nodes: nodeslen too small", clusterlist_module_name);

  for (i = 0; i < numnodes; i++)
    nodes[i] = Strdup(nodelist[i]);

  if (genders_nodelist_destroy(handle, nodelist) < 0)
    err_exit("%s clusterlist module: cerebrod_clusterlist_genders_get_all_nodes: genders_nodelist_destroy: %s",
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
    err_exit("%s clusterlist module: cerebrod_clusterlist_genders_numnodes: genders_getnumnodes: %s",
	     clusterlist_module_name, genders_errormsg(handle));

  return ret;
}
