/*****************************************************************************\
 *  $Id: cerebro_monitor_bootlog.c,v 1.15 2005-06-28 17:08:38 achu Exp $
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
#include "cerebro/cerebro_monitor_module.h"

#include "debug.h"

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
      CEREBRO_DBG(("qsql_handle null"));
      return -1;
    }

  if (!fmt)
    {
      CEREBRO_DBG(("fmt null"));
      return -1;
    }

  va_start(ap, fmt);
  vsnprintf(buf, BOOTLOG_QUERY_BUFLEN, fmt, ap);
  va_end(ap);

  if (qsql_query(qsql_handle, buf) < 0)
    {
      CEREBRO_DBG(("%s query failed: %s", buf, qsql_error(qsql_handle)));
      return -1;
    }

  return 0;
}

/*
 * bootlog_monitor_setup
 *
 * bootlog monitor module setup function.  Setup and connect to the
 * database through qsql.
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

  /* 
   * Host/user/password/database are defined by Quadrics
   * documentation.
   */

  if (!(qsql_handle = qsql_connect_and_select(BOOTLOG_MYSQL_HOST, 
                                              BOOTLOG_USER, 
                                              BOOTLOG_PASSWORD,
                                              BOOTLOG_DATABASE, 
                                              BOOTLOG_PORT, 
                                              NULL, 
                                              0)))
    {
      CEREBRO_DBG(("failed to connect to database"));
      goto cleanup;
    }

  if (!(res = qsql_list_tables(qsql_handle)))
    {
      CEREBRO_DBG(("qsql_list_tables: %s",	qsql_error(qsql_handle)));
      goto cleanup;
    }

  if ((rows = qsql_num_rows(res)) < 0)
    {
      CEREBRO_DBG(("qsql_num_rows: %s", qsql_error(qsql_handle)));
      goto cleanup;
    }

  if (rows > 0)
    {
      if ((fields = qsql_num_fields(res)) < 0)
        {
          CEREBRO_DBG(("qsql_num_fields: %s", qsql_error(qsql_handle)));
          goto cleanup;
        }

      if (fields != 1)
        {
          CEREBRO_DBG(("fields = %d != 1", fields));
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
  if (!qsql_handle)
    return 0;

  qsql_close(qsql_handle);
  qsql_handle = NULL;
  return 0;
}

/*
 * bootlog_monitor_metric_name
 *
 * bootlog monitor module metric_name function
 */
static char *
bootlog_monitor_metric_name(void)
{
  return BOOTLOG_BOOTTIME_METRIC_NAME;
}

/* 
 * _check_if_new_btime
 *
 * Check of the boottime recently received is new or old
 *
 * Returns 1 if it is new, 0 if it is old, -1 on error
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
      CEREBRO_DBG(("nodename null"));
      return -1;
    }

  if (_qsql_query("select btime from last_btime where name='%s'", nodename) < 0)
    goto cleanup;

  if (!(res = qsql_store_result(qsql_handle)))
    {
      CEREBRO_DBG(("qsql_store_result: %s", qsql_error(qsql_handle)));
      goto cleanup;
    }

  if ((rows = qsql_num_rows(res)) < 0)
    {
      CEREBRO_DBG(("qsql_num_rows: %s", qsql_error(qsql_handle)));
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
          CEREBRO_DBG(("qsql_num_fields: %s", qsql_error(qsql_handle)));
          goto cleanup;
        }

      if (fields != 1)
        {
          CEREBRO_DBG(("fields = %d != 1", fields));
          goto cleanup;
        }

      if ((row = qsql_fetch_row(res)))
        {
          if (row[0])
            {
              stored_btime = strtol(row[0], &ptr, 10);
              if (ptr != (row[0] + strlen(row[0])))
                {
                  CEREBRO_DBG(("strtol: %s", strerror(errno)));
                  goto cleanup;
                }
            }
          else
            {
              CEREBRO_DBG(("%s btime null", nodename));
              goto cleanup;
            }
        }

      /* Rounding issue on some kernels that can change boottime +/-
       * one or two.  This is the fix.  We have to assume that a
       * machine can't fully reboot within 2 seconds.
       */
      if (btime > (stored_btime + 2))
        {
          if (_qsql_query("update last_btime set btime=%d where name='%s'",
                          btime, nodename) < 0)
            goto cleanup;
          rv++;
        }
    }
  else
    {
      CEREBRO_DBG(("%s has multiple name entries", nodename));
      goto cleanup;
    }

  qsql_free_result(res);
  return rv;

 cleanup:
  if (res)
    qsql_free_result(res);
  return -1;
}


/*
 * bootlog_monitor_metric_name
 *
 * bootlog monitor module metric_name function.  Store results the
 * bootlog database appropriately.
 */
static int 
bootlog_monitor_metric_update(const char *nodename,
                              unsigned int metric_value_type,
                              unsigned int metric_value_len,
                              void *metric_value)
{
  u_int32_t btime;
  int new_btime;

  if (!qsql_handle)
    {
      CEREBRO_DBG(("qsql_handle null"));
      return -1;
    }

  if (!nodename || !metric_value)
    {
      CEREBRO_DBG(("invalid parameters"));
      return -1;
    }

  if (metric_value_type != CEREBRO_METRIC_VALUE_TYPE_U_INT32)
    {
      CEREBRO_DBG(("invalid metric_value_type: %d", metric_value_type));
      return -1;
    }

  if (metric_value_len != sizeof(u_int32_t))
    {
      CEREBRO_DBG(("invalid metric_value_len: %d", metric_value_len));
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
    &bootlog_monitor_metric_name,
    &bootlog_monitor_metric_update,
  };
