/*****************************************************************************\
 *  $Id: cerebro_monitor_bootlog.c,v 1.7 2005-06-16 21:35:34 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#include <stdarg.h>
#endif /* STDC_HEADERS */
#include <qsql/qsql.h>
#include <errno.h>

#include "cerebro.h"
#include "cerebro/cerebro_error.h"
#include "cerebro/cerebro_monitor_module.h"

#define BOOTLOG_MONITOR_MODULE_NAME  "bootlog"
#define BOOTLOG_BOOTTIME_METRIC_NAME "boottime"
#define BOOTLOG_QUERY_BUFLEN         4096
#define BOOTLOG_MYSQL_HOST           "localhost"
#define BOOTLOG_USER                 "qsnet"
#define BOOTLOG_PASSWORD             ""
#define BOOTLOG_DATABASE             "qsnet"
#define BOOTLOG_PORT                 0

/* 
 * qsql_handle
 * 
 * The qsql library handle 
 */
static void *qsql_handle = NULL;

/* 
 * _qsql_query
 *
 * Perform a database query
 */
static int
_qsql_query(char *fmt, ...)
{
  char buf[BOOTLOG_QUERY_BUFLEN];
  va_list ap;

  if (!qsql_handle)
    {
      cerebro_err_debug("%s(%s:%d): qsql_handle null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!fmt)
    {
      cerebro_err_debug("%s(%s:%d): fmt null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  va_start(ap, fmt);
  vsnprintf(buf, BOOTLOG_QUERY_BUFLEN, fmt, ap);
  va_end(ap);

  if (qsql_query(qsql_handle, buf) < 0)
    {
      cerebro_err_debug("%s(%s:%d): %s query failed: %s",
			__FILE__, __FUNCTION__, __LINE__,
			buf, qsql_error(qsql_handle));
      return -1;
    }

  return 0;
}

/*
 * bootlog_monitor_setup
 *
 * bootlog monitor module setup function
 */
static int
bootlog_monitor_setup(void)
{
  QSQL_RESULT *res = NULL;
  QSQL_ROW row;
  int rows, fields;
  int bootlog_created = 0;
  int last_btime_created = 0;

  qsql_init(QSQL_USE_MYSQL);
  if (!(qsql_handle = qsql_connect_and_select(BOOTLOG_MYSQL_HOST, 
                                              BOOTLOG_USER, 
                                              BOOTLOG_PASSWORD,
                                              BOOTLOG_DATABASE, 
                                              BOOTLOG_PORT, 
                                              NULL, 
                                              0)))
    {
      cerebro_err_debug("%s(%s:%d): failed to connect to database",
			__FILE__, __FUNCTION__, __LINE__);
      goto cleanup;
    }

  if (!(res = qsql_list_tables(qsql_handle)))
    {
      cerebro_err_debug("%s(%s:%d): qsql_list_tables: %s",
			__FILE__, __FUNCTION__, __LINE__,
			qsql_error(qsql_handle));
      goto cleanup;
    }

  if ((rows = qsql_num_rows(res)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): qsql_num_rows: %s",
			__FILE__, __FUNCTION__, __LINE__,
			qsql_error(qsql_handle));
      goto cleanup;
    }

  if (rows > 0)
    {
      if ((fields = qsql_num_fields(res)) < 0)
        {
          cerebro_err_debug("%s(%s:%d): qsql_num_fields: %s",
			    __FILE__, __FUNCTION__, __LINE__,
			    qsql_error(qsql_handle));
          goto cleanup;
        }

      if (fields != 1)
        {
          cerebro_err_debug("%s(%s:%d): fields = %d != 1",
			    __FILE__, __FUNCTION__, __LINE__,
			    fields);
          goto cleanup;
        }

      while ((row = qsql_fetch_row(res)) != NULL)
        {
          if (!row[0])
            continue;
          if (!strcmp(row[0], "bootlog"))
            bootlog_created++;
          if (!strcmp(row[0], "last_btime"))
            last_btime_created++;
        }
    }

  qsql_free_result(res);

  if (!bootlog_created)
    {       
      if (_qsql_query("create table bootlog(name varchar(16), btime int, ctime int)") < 0)
        goto cleanup;
    }
  
  if (!last_btime_created)
    {
      if (_qsql_query("create table last_btime(name varchar(16), btime int)") < 0)
        goto cleanup;
    }
  return 0;

 cleanup:
  if (qsql_handle)
    {
      qsql_close(qsql_handle);
      qsql_handle = NULL;
    }
  if (res)
    qsql_free_result(res);
  qsql_fini();
  return -1;
}

/*
 * bootlog_monitor_cleanup
 *
 * bootlog monitor module cleanup function
 */
static int
bootlog_monitor_cleanup(void)
{
  if (qsql_handle)
    qsql_close(qsql_handle);
  qsql_handle = NULL;
  return 0;
}

/* 
 * bootlog_monitor_metric_names
 *
 * bootlog monitor module metric_names function
 *
 * Returns 0 on success, -1 on error
 */
static int 
bootlog_monitor_metric_names(char ***metric_names)
{
  char **names = NULL;
   
  if (!metric_names)
    {
      cerebro_err_debug("%s(%s:%d): metric_names null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
 
  if (!(names = (char **)malloc(sizeof(char *) * 2)))
    {
      cerebro_err_debug("%s(%s:%d): malloc: %s",
			__FILE__, __FUNCTION__, __LINE__,
			strerror(errno));
      goto cleanup;
    }
  memset(names, '\0', sizeof(char *) * 2);

  if (!(names[0] = strdup(BOOTLOG_BOOTTIME_METRIC_NAME)))
    {
      cerebro_err_debug("%s(%s:%d): strdup: %s",
			__FILE__, __FUNCTION__, __LINE__,
			strerror(errno));
      goto cleanup;
    }
  names[1] = NULL;

  *metric_names = names;
  return 1;
 
 cleanup:
  if (names)
    {
      if (names[0])
        free(names[0]);
      free(names);
    }
  return -1;
}

/* 
 * _check_if_new_btime
 *
 * Check of the boottime recently received is new
 *
 * Returns 1 if it is new, 0 if not, -1 on error
 */
static int
_check_if_new_btime(const char *nodename, u_int32_t btime)
{
  unsigned long stored_btime;
  QSQL_RESULT *res = NULL;
  QSQL_ROW row;
  char *ptr;
  int rows, fields;
  int rv = 0;

  if (!nodename)
    {
      cerebro_err_debug("%s(%s:%d): nodename null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (_qsql_query("select btime from last_btime where name='%s'", nodename) < 0)
    goto cleanup;

  if (!(res = qsql_store_result(qsql_handle)))
    {
      cerebro_err_debug("%s(%s:%d): qsql_store_result: %s",
			__FILE__, __FUNCTION__, __LINE__,
			qsql_error(qsql_handle));
      goto cleanup;
    }

  if ((rows = qsql_num_rows(res)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): qsql_num_rows: %s",
			__FILE__, __FUNCTION__, __LINE__,
			qsql_error(qsql_handle));
      goto cleanup;
    }

  if (!rows)
    {
      if (_qsql_query("insert into last_btime (name,btime) values ('%s', %d)", nodename, btime) < 0)
        goto cleanup;
      rv++;
    }
  else if (rows)
    {
      if ((fields = qsql_num_fields(res)) < 0)
        {
          cerebro_err_debug("%s(%s:%d): qsql_num_fields: %s",
			    __FILE__, __FUNCTION__, __LINE__,
			    qsql_error(qsql_handle));
          goto cleanup;
        }

      if (fields != 1)
        {
          cerebro_err_debug("%s(%s:%d): fields = %d != 1",
			    __FILE__, __FUNCTION__, __LINE__,
			    fields);
          goto cleanup;
        }

      if ((row = qsql_fetch_row(res)))
        {
          if (row[0])
            {
              stored_btime = strtol(row[0], &ptr, 10);
              if (ptr != (row[0] + strlen(row[0])))
                {
                  cerebro_err_debug("%s(%s:%d): strtol: %s",
				    __FILE__, __FUNCTION__, __LINE__,
				    strerror(errno));
                  goto cleanup;
                }
            }
          else
            {
              cerebro_err_debug("%s(%s:%d): %s btime null",
				__FILE__, __FUNCTION__, __LINE__,
				nodename);
              goto cleanup;
            }
        }

      /* Rounding issue on some kernels that can change boottime +/-
       * one.  This is the fix.
       */
      if (btime > (stored_btime + 1))
        {
          if (_qsql_query("update last_btime set btime=%d where name='%s'",
                          btime, nodename) < 0)
            goto cleanup;
          rv++;
        }
    }
  else
    {
      cerebro_err_debug("%s(%s:%d): %s has multiple name entries",
			__FILE__, __FUNCTION__, __LINE__,
			nodename);
      goto cleanup;
    }

  qsql_free_result(res);
  return rv;

 cleanup:
  if (res)
    qsql_free_result(res);
  return -1;
}

static int 
bootlog_monitor_metric_update(const char *nodename,
                              const char *metric_name,
                              unsigned int metric_value_type,
                              unsigned int metric_value_len,
                              void *metric_value)
{
  u_int32_t btime;
  int new_btime;

  if (!qsql_handle)
    {
      cerebro_err_debug("%s(%s:%d): qsql_handle null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!nodename)
    {
      cerebro_err_debug("%s(%s:%d): nodename null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!metric_name)
    {
      cerebro_err_debug("%s(%s:%d): metric_name null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (strcmp(metric_name, BOOTLOG_BOOTTIME_METRIC_NAME))
    {
      cerebro_err_debug("%s(%s:%d): metric_name invalid: %s",
			__FILE__, __FUNCTION__, __LINE__,
			metric_name);
      return -1;
    }

  if (metric_value_type != CEREBRO_METRIC_VALUE_TYPE_U_INT32)
    {
      cerebro_err_debug("%s(%s:%d): invalid metric_value_type: %d",
			__FILE__, __FUNCTION__, __LINE__,
			metric_value_type);
      return -1;
    }

  if (metric_value_len != sizeof(u_int32_t))
    {
      cerebro_err_debug("%s(%s:%d): invalid metric_value_len: %d",
			__FILE__, __FUNCTION__, __LINE__,
			metric_value_len);
      return -1;
    }

  if (!metric_value)
    {
      cerebro_err_debug("%s(%s:%d): metric_value null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  btime = *((u_int32_t *)metric_value);
  if ((new_btime = _check_if_new_btime(nodename, btime)) < 0)
    goto cleanup;

  if (new_btime)
    {
      if (_qsql_query("insert into bootlog (name,btime,ctime) values ('%s',%d,%d)", nodename, btime, btime) < 0)
        goto cleanup;
    }
  else
    {
      time_t now = time(NULL);
      
      if (_qsql_query("update bootlog set ctime=%d where name='%s' and btime=%d", now, nodename, btime) < 0)
        goto cleanup;
    }

  return 0;

 cleanup:
  return -1;
}

struct cerebro_monitor_module_info monitor_module_info =
  {
    BOOTLOG_MONITOR_MODULE_NAME,
    &bootlog_monitor_setup,
    &bootlog_monitor_cleanup,
    &bootlog_monitor_metric_names,
    &bootlog_monitor_metric_update,
  };
