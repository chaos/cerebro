/*****************************************************************************\
 *  $Id: cerebrod_config_gendersllnl.c,v 1.1 2005-03-18 19:04:25 achu Exp $
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

#include <gendersllnl.h>

#include "cerebrod.h"
#include "cerebrod_config.h"
#include "error.h"
#include "wrappers.h"

int
gendersllnl_config_load_default(struct cerebrod_config *conf)
{
  genders_t handle = NULL;
  int ret;

  assert(conf);

  if (!(handle = genders_handle_create()))
    err_exit("genders_config_load_default: genders_handle_create");
 
  if (genders_load_data(handle, NULL) < 0)
    err_exit("genders_config_load_default: genders_load_data: %s", 
             genders_errormsg(handle));

  if ((ret = genders_testattr(handle, NULL, "mgmt", NULL, 0)) < 0)
    err_exit("genders_config_load_default: genders_testattr: %s", 
             genders_errormsg(handle));
    
  if (ret)
    {
      conf->speak = 1;
      conf->listen = 1;
      conf->updown_server = 1;
    }
  else
    {
      conf->speak = 1;
      conf->listen = 0;
      conf->updown_server = 0;
    }

  if (genders_handle_destroy(handle) < 0)
    err_exit("genders_config_load_default: genders_handle_destroy: %s",
             genders_errormsg(handle));
}

struct cerebrod_config_module_info config_module_info =
  {
    "gendersllnl",
  };

struct cerebrod_config_module_ops config_module_ops =
  {
    &gendersllnl_config_load_default,
  };


