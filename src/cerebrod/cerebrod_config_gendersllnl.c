/*****************************************************************************\
 *  $Id: cerebrod_config_gendersllnl.c,v 1.5 2005-03-24 01:29:21 achu Exp $
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

#include "cerebrod_config_module.h"

#include "cerebrod.h"
#include "cerebrod_config.h"
#include "error.h"
#include "wrappers.h"

/* 
 * gendersllnl_config_load_default
 *
 * alter default module specifically for use on LLNL clusters 'mgmt'
 * nodes listen and speak, while compute nodes only speak.
 *
 * Returns 0 on success, -1 on error
 */
int
gendersllnl_config_load_default(struct cerebrod_module_config *conf)
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

  return 0;
}

#if WITH_STATIC_MODULES
struct cerebrod_config_module_info gendersllnl_config_module_info =
#else
struct cerebrod_config_module_info config_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    "gendersllnl",
    &gendersllnl_config_load_default,
  };
