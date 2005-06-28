/*****************************************************************************\
 *  $Id: cerebro_clusterlist_genders_util.c,v 1.19 2005-06-28 17:08:38 achu Exp $
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

#include "debug.h"

int 
cerebro_clusterlist_genders_setup(genders_t *gh)
{
  if (!gh)
    { 
      CEREBRO_DBG(("gh null"));
      return -1;
    }

  *gh = NULL;

  if (!(*gh = genders_handle_create()))
    {
      CEREBRO_DBG(("genders_handle_create"));
      goto cleanup;
    }

  if (genders_load_data(*gh, NULL) < 0)
    {
      if (genders_errnum(*gh) == GENDERS_ERR_OPEN)
	{
	  cerebro_err_output("genders database '%s' cannot be opened",  
                             GENDERS_DEFAULT_FILE);
	  goto cleanup;
	}
      else
        {
          CEREBRO_DBG(("genders_load_data: %s", genders_errormsg(*gh)));
          goto cleanup;
        }
    }

  return 0;

 cleanup:
  if (*gh)
    genders_handle_destroy(*gh);
  *gh = NULL;
  return -1;
}

int
cerebro_clusterlist_genders_cleanup(genders_t *gh)
{
  if (!gh)
    {
      CEREBRO_DBG(("gh null"));
      return -1;
    }

  if (genders_handle_destroy(*gh) < 0)
    {
      CEREBRO_DBG(("genders_handle_destroy: %s", genders_errormsg(*gh)));
      return -1;
    }

  *gh = NULL;

  return 0;
}

int 
cerebro_clusterlist_genders_numnodes(genders_t gh)
{
  int num;

  if (!gh)
    {
      CEREBRO_DBG(("gh null"));
      return -1;
    }

  if ((num = genders_getnumnodes(gh)) < 0)
    {
      CEREBRO_DBG(("genders_getnumnodes: %s", genders_errormsg(gh)));
      return -1;
    }

  return num;
}

int
cerebro_clusterlist_genders_get_all_nodes(genders_t gh, char ***nodes)
{
  char **nodelist = NULL;
  int nodelistlen, numnodes;
  
  if (!gh)
    {
      CEREBRO_DBG(("gh null"));
      return -1;
    }

  if (!nodes)
    {
      CEREBRO_DBG(("nodes null"));
      return -1;
    }

  if ((nodelistlen = genders_nodelist_create(gh, &nodelist)) < 0)
    {
      CEREBRO_DBG(("genders_nodelist_create: %s", genders_errormsg(gh)));
      goto cleanup;
    }
  
  if ((numnodes = genders_getnodes(gh, 
				   nodelist, 
				   nodelistlen, 
				   NULL, 
				   NULL)) < 0)
    {
      CEREBRO_DBG(("genders_getnodes: %s", genders_errormsg(gh)));
      goto cleanup;
    }


  *nodes = nodelist;
  return numnodes;

 cleanup:
  if (nodelist)
    genders_nodelist_destroy(gh, nodelist);
  return -1;
}

