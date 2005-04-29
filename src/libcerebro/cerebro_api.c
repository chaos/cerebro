/*****************************************************************************\
 *  $Id: cerebro_api.c,v 1.2 2005-04-29 19:09:57 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */

#include "cerebro.h"
#include "cerebro_api.h"
#include "cerebro_util.h"
#include "cerebro_updown.h"

#include "conffile.h"
#include "ltdl.h"

char *cerebro_error_messages[] =
  {
    "success",
    "null cerebro_t handle",
    "invalid magic number",
    "invalid parameters",
    "server data not loaded",
    "cerebro version incompatabile",
    "invalid hostname",
    "invalid address",
    "connection error",
    "connection timeout",
    "network communication error",
    "protocol error",
    "protocol timeout",
    "buffer overflow",
    "node not found",
    "out of memory",
    "config error",
    "module error",
    "internal error",
    "errnum out of range",
  };

cerebro_t
cerebro_handle_create(void)
{
  cerebro_t handle = NULL;

  if (!(handle = (cerebro_t)malloc(sizeof(struct cerebro))))
    goto cleanup;
                 
  memset(handle, '\0', sizeof(struct cerebro));
  handle->magic = CEREBRO_MAGIC_NUMBER;
  handle->errnum = CEREBRO_ERR_SUCCESS;
  handle->loaded_state = 0;
  memset(&(handle->config), '\0', sizeof(struct cerebro_config));
#if !WITH_STATIC_MODULES
  handle->clusterlist_dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */
  handle->clusterlist_module_info = NULL;
#if !WITH_STATIC_MODULES
  handle->config_dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */
  handle->config_module_info = NULL;
  handle->updown_data = NULL;

 cleanup:
  return handle;
}

int
cerebro_handle_destroy(cerebro_t handle)
{
  if (cerebro_handle_check(handle) < 0)
    return -1;

  if (handle->loaded_state & CEREBRO_CONFIG_LOADED)
    {
      if (cerebro_unload_config(handle) < 0)
        return -1;
      
      if (handle->loaded_state & CEREBRO_CONFIG_LOADED)
        {
          handle->errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }

  if (handle->loaded_state & CEREBRO_MODULES_LOADED)
    {
      if (cerebro_unload_modules(handle) < 0)
        return -1;
      
      if (handle->loaded_state & CEREBRO_MODULES_LOADED)
        {
          handle->errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }

  if (handle->loaded_state & CEREBRO_UPDOWN_DATA_LOADED)
    {
      if (cerebro_updown_unload_data(handle) < 0)
        return -1;

      if (handle->loaded_state & CEREBRO_UPDOWN_DATA_LOADED)
        {
          handle->errnum = CEREBRO_ERR_INTERNAL;
          return -1;
        }
    }

  handle->errnum = CEREBRO_ERR_SUCCESS;
  handle->magic = ~CEREBRO_MAGIC_NUMBER;
  free(handle);
  return 0;
}

int
cerebro_errnum(cerebro_t handle)
{
  if (!handle)
    return CEREBRO_ERR_NULLHANDLE;
  else if (handle->magic != CEREBRO_MAGIC_NUMBER)
    return CEREBRO_ERR_MAGIC_NUMBER;
  else
    return handle->errnum;
}

char *
cerebro_strerror(int errnum)
{
  if (errnum >= CEREBRO_ERR_SUCCESS && errnum <= CEREBRO_ERR_ERRNUMRANGE)
    return cerebro_error_messages[errnum];
  else
    return cerebro_error_messages[CEREBRO_ERR_ERRNUMRANGE];
}

char *
cerebro_errormsg(cerebro_t handle)
{
  return cerebro_strerror(cerebro_errnum(handle));
}

void
cerebro_perror(cerebro_t handle, const char *msg)
{
  char *errormsg = cerebro_strerror(cerebro_errnum(handle));
 
  if (!msg)
    fprintf(stderr, "%s\n", errormsg);
  else
    fprintf(stderr, "%s: %s\n", msg, errormsg);
}

/*
 * _cb_string
 *
 * conffile callback function that parses and stores a string
 *
 * Returns 0 on success, -1 on error
 */
static int
_cb_string(conffile_t cf, struct conffile_data *data,
           char *optionname, int option_type, void *option_ptr,
           int option_data, void *app_ptr, int app_data)
{
  if (option_ptr == NULL)
    {
      conffile_seterrnum(cf, CONFFILE_ERR_PARAMETERS);
      return -1;
    }
                                                                                      
  if (!(*((char **)option_ptr) = strdup(data->string)))
    {
      conffile_seterrnum(cf, CONFFILE_ERR_OUTMEM);
      return -1;
    }

  return 0;
}

/*
 * _cb_string_array
 *
 * conffile callback function that parses and stores an array of
 * strings
 *
 * Returns 0 on success, -1 on error
 */
static int
_cb_string_array(conffile_t cf, struct conffile_data *data,
                 char *optionname, int option_type, void *option_ptr,
                 int option_data, void *app_ptr, int app_data)
{
  if (option_ptr == NULL)
    {
      conffile_seterrnum(cf, CONFFILE_ERR_PARAMETERS);
      return -1;
    }
                                                                                      
  if (data->stringlist_len > 0)
    {
      char ***p = (char ***)option_ptr;
      int i;
                                                                                      
      if (!(*p = (char **)malloc(sizeof(char *) * (data->stringlist_len + 1))))
        {
          conffile_seterrnum(cf, CONFFILE_ERR_OUTMEM);
          return -1;
        }
                                                                                      
      for (i = 0; i < data->stringlist_len; i++)
        {
          if (!((*p)[i] = strdup(data->stringlist[i])))
            {
              int j;
              
              for (j = 0; j < i; j++)
                free((*p)[j]);
              free(*p);

              conffile_seterrnum(cf, CONFFILE_ERR_OUTMEM);
              return -1;
            }
        }
      (*p)[i] = NULL;
    }
                                                                                      
  return 0;
}

/* 
 * _cerebro_load_config
 *
 * Read and load configuration data
 *
 * Returns 0 on success, -1 on error
 */
static int
_cerebro_load_config(cerebro_t handle, struct cerebro_config *config)
{
  struct conffile_option options[] =
    {
      {"clusterlist_module", CONFFILE_OPTION_STRING, -1,
       _cb_string, 1, 0, &(config->clusterlist_module_flag),
       &(config->clusterlist_module), 0},
      {"clusterlist_module_options", CONFFILE_OPTION_LIST_STRING, -1,
       _cb_string_array, 1, 0, &(config->clusterlist_module_options_flag),
       &(config->clusterlist_module_options), 0},
      {"config_module", CONFFILE_OPTION_STRING, -1,
       _cb_string, 1, 0, &(config->config_module_flag),
       &(config->config_module), 0},
      {"updown_hostnames", CONFFILE_OPTION_LIST_STRING, -1,
       _cb_string_array, 1, 0, &(config->updown_hostnames_flag),
       &(config->updown_hostnames), 0},
      {"updown_port", CONFFILE_OPTION_INT, -1,
       conffile_int, 1, 0, &(config->updown_port_flag),
       &(config->updown_port), 0},
      {"updown_timeout_len", CONFFILE_OPTION_INT, -1,
       conffile_int, 1, 0, &(config->updown_timeout_len_flag),
       &(config->updown_timeout_len), 0},
      {"updown_flags", CONFFILE_OPTION_INT, -1,
       conffile_int, 1, 0, &(config->updown_flags_flag),
       &(config->updown_flags), 0},
    };
  conffile_t cf = NULL;
  int num;

  if (!(cf = conffile_handle_create()))
    {
      handle->errnum = CEREBRO_ERR_CONFIG;
      goto cleanup;
    }
                                                                                      
  memset(config, '\0', sizeof(struct cerebro_config));

  num = sizeof(options)/sizeof(struct conffile_option);
  if (conffile_parse(cf, CEREBRO_CONFIG_FILE_DEFAULT, options, num, NULL, 0, 0) < 0)
    {
      /* Its not an error if the default configuration file doesn't exist */
      if (conffile_errnum(cf) != CONFFILE_ERR_EXIST)
        {
          handle->errnum = CEREBRO_ERR_CONFIG;
          goto cleanup;
        }
    }
  
  conffile_handle_destroy(cf);
  return 0;

 cleanup:
  conffile_handle_destroy(cf);
  if (config->clusterlist_module_flag && config->clusterlist_module)
    free(config->clusterlist_module);
         
  if (config->config_module_flag && config->config_module)
    free(config->config_module);

  if (config->clusterlist_module_options_flag && 
      config->clusterlist_module_options)
    {
      int i = 0;
      
      while (config->clusterlist_module_options[i])
        free(config->clusterlist_module_options[i]);
      free(config->clusterlist_module_options);
    }

  if (config->updown_hostnames_flag && config->updown_hostnames)
    {
      int i = 0;
      
      while (config->updown_hostnames[i])
        free(config->updown_hostnames[i]);
      free(config->updown_hostnames);
    }
  memset(config, '\0', sizeof(struct cerebro_config));
  return -1;
}

int 
cerebro_load_config(cerebro_t handle)
{
  if (cerebro_handle_check(handle) < 0)
    return -1;

  if (handle->loaded_state & CEREBRO_CONFIG_LOADED)
    {
      handle->errnum = CEREBRO_ERR_SUCCESS;
      return 0;
    }
  
  if (_cerebro_load_config(handle, &(handle->config)) < 0)
    return -1;

  handle->loaded_state |= CEREBRO_CONFIG_LOADED;
  handle->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}

int 
cerebro_unload_config(cerebro_t handle)
{
  struct cerebro_config *config;

  if (cerebro_handle_check(handle) < 0)
    return -1;

  config = &(handle->config);

  if (config->clusterlist_module_flag && config->clusterlist_module)
    free(config->clusterlist_module);
         
  if (config->clusterlist_module_options_flag && config->clusterlist_module_options)
    {
      int i = 0;
      
      while (config->clusterlist_module_options[i])
        free(config->clusterlist_module_options[i]);
      free(config->clusterlist_module_options);
    }

  if (config->config_module_flag && config->config_module)
    free(config->config_module);

  if (config->updown_hostnames_flag && config->updown_hostnames)
    {
      int i = 0;
      
      while (config->updown_hostnames[i])
        free(config->updown_hostnames[i]);
      free(config->updown_hostnames);
    }

  config->clusterlist_module = NULL;
  config->clusterlist_module_flag = 0;
  config->clusterlist_module_options = NULL;
  config->clusterlist_module_options_flag = 0;
  config->config_module = NULL;
  config->config_module_flag = 0;
  config->updown_hostnames = NULL;
  config->updown_hostnames_flag = 0;
  config->updown_port = 0;
  config->updown_port_flag = 0;
  config->updown_timeout_len = 0;
  config->updown_timeout_len_flag = 0;
  config->updown_flags = 0;
  config->updown_flags_flag = 0;

  handle->loaded_state &= ~CEREBRO_MODULES_LOADED;
  handle->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}

/* 
 * 
 */

int 
cerebro_load_modules(cerebro_t handle)
{
  if (cerebro_handle_check(handle) < 0)
    return -1;

#if WITH_STATIC_MODULES
#else  /* !WITH_STATIC_MODULES */
  
#endif /* !WITH_STATIC_MODULES */
  return 0;
}

int 
cerebro_unload_modules(cerebro_t handle)
{
  if (cerebro_handle_check(handle) < 0)
    return -1;

  if (handle->clusterlist_module_info)
    (*handle->clusterlist_module_info->cleanup)();
  if (handle->config_module_info)
    (*handle->config_module_info->cleanup)();

#if !WITH_STATIC_MODULES
  if (handle->clusterlist_dl_handle)
    lt_dlclose(handle->clusterlist_dl_handle);
  if (handle->config_dl_handle)
    lt_dlclose(handle->config_dl_handle);
  lt_dlexit();
#endif /* !WITH_STATIC_MODULES */

#if !WITH_STATIC_MODULES
  handle->clusterlist_dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */
  handle->clusterlist_module_info = NULL;
#if !WITH_STATIC_MODULES
  handle->config_dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */
  handle->config_module_info = NULL;

  handle->loaded_state &= ~CEREBRO_MODULES_LOADED;
  handle->errnum = CEREBRO_ERR_SUCCESS;
  return 0;
}
