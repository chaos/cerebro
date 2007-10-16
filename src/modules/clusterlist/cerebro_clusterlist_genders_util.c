/*****************************************************************************\
 *  $Id: cerebro_clusterlist_genders_util.c,v 1.25 2007-10-16 22:43:17 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2005 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <http://www.llnl.gov/linux/cerebro/>.
 *
 *  Cerebro is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  Cerebro is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with Cerebro; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
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
cerebro_clusterlist_genders_setup(genders_t *gh, char *filename)
{
  if (!gh)
    { 
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  *gh = NULL;

  if (!(*gh = genders_handle_create()))
    {
      CEREBRO_ERR(("genders_handle_create"));
      goto cleanup;
    }

  if (!filename)
    filename = GENDERS_DEFAULT_FILE;
  if (genders_load_data(*gh, filename) < 0)
    {
      if (genders_errnum(*gh) == GENDERS_ERR_OPEN)
	{
	  CEREBRO_ERR(("genders database '%s' cannot be opened", filename));
	  goto cleanup;
	}
      else
        {
          CEREBRO_ERR(("genders_load_data: %s", genders_errormsg(*gh)));
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
      CEREBRO_DBG(("invalid parameters"));
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
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  if ((num = genders_getnumnodes(gh)) < 0)
    {
      CEREBRO_ERR(("genders_getnumnodes: %s", genders_errormsg(gh)));
      return -1;
    }

  return num;
}

int
cerebro_clusterlist_genders_get_all_nodes(genders_t gh, char ***nodes)
{
  char **nodelist = NULL;
  int nodelistlen, num;
  
  if (!gh || !nodes)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  if ((nodelistlen = genders_nodelist_create(gh, &nodelist)) < 0)
    {
      CEREBRO_ERR(("genders_nodelist_create: %s", genders_errormsg(gh)));
      goto cleanup;
    }
  
  if ((num = genders_getnodes(gh, nodelist, nodelistlen, NULL, NULL)) < 0)
    {
      CEREBRO_ERR(("genders_getnodes: %s", genders_errormsg(gh)));
      goto cleanup;
    }

  *nodes = nodelist;
  return num;

 cleanup:
  if (nodelist)
    genders_nodelist_destroy(gh, nodelist);
  return -1;
}

