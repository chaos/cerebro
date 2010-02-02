/*****************************************************************************\
 *  $Id: cerebro-admin.c,v 1.19 2010-02-02 01:01:20 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2010 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2005-2007 The Regents of the University of California.
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
 *  with Cerebro.  If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <sys/types.h>

#define _GNU_SOURCE
#if HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */
#include <assert.h>
#include <errno.h>

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"
#if CEREBRO_DEBUG
#include "cerebro/cerebro_error.h"
#endif /* CEREBRO_DEBUG */

#include "error.h"

#define CEREBRO_ADMIN_REGISTER    0
#define CEREBRO_ADMIN_UNREGISTER  1
#define CEREBRO_ADMIN_UPDATE      2
#define CEREBRO_ADMIN_RESEND      3
#define CEREBRO_ADMIN_FLUSH       4

/* 
 * External variables for getopt 
 */
extern char *optarg;
extern int optind, opterr, optopt;

/* 
 * Cerebro-Admin Data
 *
 * handle - cerebro handle
 */
static cerebro_t handle;
int operation = -1;
char *metric_name = NULL;
unsigned int metric_value_type = CEREBRO_DATA_VALUE_TYPE_NONE;
unsigned int metric_value_len = 0;
char *metric_value = NULL;
int32_t metric_value_int32;
u_int32_t metric_value_u_int32;
float metric_value_float;
double metric_value_double;
char *metric_value_string;
int64_t metric_value_int64;
u_int64_t metric_value_u_int64;
void *metric_value_ptr;

/* 
 * _init_cerebro_admin
 *
 * initialize globals
 */
static void
_init_cerebro_admin(void)
{
  const char *func = __FUNCTION__;

  if (!(handle = cerebro_handle_create()))
    err_exit("%s: cerebro_handle_create()", func);
}

/* 
 * _cleanup_cerebro_stat
 *
 * cleanup globals
 */
static void
_cleanup_cerebro_stat(void)
{
  (void)cerebro_handle_destroy(handle);
}

/* 
 * _usage
 *
 * output usage and exit
 */
static void 
_usage(void) 
{
  fprintf(stderr,
	  "Usage: cerebro_admin [OPTIONS] -m <metric_name> [-r | -u | -p | -s]\n"
          "\n"
          "OPTIONS:\n"
	  "  -h         Print help and exit\n"
	  "  -v         Print version and exit\n"
          "  -m STRING  Specify the metric name\n"
          "  -r         Register the metric name\n"
          "  -u         Unregister the metric name\n"
          "  -p         Update the metric's value\n"
          "  -s         Resend the metric\n"
          "  -f         Flush the metric\n"
          "  -t INT     Specify metric type\n"
          "     %d - none (default)\n"
          "     %d - int32\n"
          "     %d - u_int32\n"
          "     %d - float\n"
          "     %d - double\n"
          "     %d - string\n"
          "     %d - int64\n"
          "     %d - u_int64\n"
          "  -l STRING  Specify metric value\n"
          "  -N         Propogate updated data immediately\n",
          CEREBRO_DATA_VALUE_TYPE_NONE,
          CEREBRO_DATA_VALUE_TYPE_INT32,
          CEREBRO_DATA_VALUE_TYPE_U_INT32,
          CEREBRO_DATA_VALUE_TYPE_FLOAT,
          CEREBRO_DATA_VALUE_TYPE_DOUBLE,
          CEREBRO_DATA_VALUE_TYPE_STRING,
          CEREBRO_DATA_VALUE_TYPE_INT64,
          CEREBRO_DATA_VALUE_TYPE_U_INT64);
#if CEREBRO_DEBUG
  fprintf(stderr,
          "  -d         Turn on debugging\n");
#endif /* CEREBRO_DEBUG */

  fprintf(stderr, "\n");
  exit(1);
}

/*  
 * _version
 *
 * output version and exit
 */
static void 
_version(void) 
{
  fprintf(stderr, "%s %s-%s\n", PROJECT, VERSION, RELEASE);
  exit(1);
}

/*
 * _cerebro_set_flags
 *
 * Set a cerebro flag
 */
static void
_cerebro_set_flags(unsigned int new_flag)
{
  const char *func = __FUNCTION__;
  int flags;

  if ((flags = cerebro_get_flags(handle)) < 0)
    {
      char *msg = cerebro_strerror(cerebro_errnum(handle));
      err_exit("%s: cerebro_get_flags: %s", func, msg);
    }

  flags |= new_flag;

  if (cerebro_set_flags(handle, flags) < 0)
    {
      char *msg = cerebro_strerror(cerebro_errnum(handle));
      err_exit("%s: cerebro_set_flags: %s", func, msg);
    }
}

/* 
 * _cmdline_parse
 *
 * parse all cmdline input
 */
static void
_cmdline_parse(int argc, char **argv) 
{
  char options[1024];
  char *ptr;
  int c;

#if HAVE_GETOPT_LONG
  struct option loptions[] = 
    {
      {"help",              0, NULL, 'h'},
      {"version",           0, NULL, 'v'},
      {"metric",            1, NULL, 'm'},
      {"register",          0, NULL, 'r'},
      {"unregister",        0, NULL, 'u'},
      {"update",            0, NULL, 'p'},
      {"resend",            0, NULL, 's'},
      {"flush",             0, NULL, 'f'},
      {"metric-value-type", 1, NULL, 't'},
      {"metric-value",      1, NULL, 'l'},
      {"send-now",          0, NULL, 'N'},
#if CEREBRO_DEBUG
      {"debug",       0, NULL, 'd'},
#endif /* CEREBRO_DEBUG */
      {0, 0, 0, 0},
  };
#endif /* HAVE_GETOPT_LONG */

  assert(argv);

  strcpy(options, "hvm:rupsft:l:N");
#if CEREBRO_DEBUG
  strcat(options, "d");
#endif /* CEREBRO_DEBUG */

  /* turn off output messages printed by getopt_long */
  opterr = 0;

#if HAVE_GETOPT_LONG
  while ((c = getopt_long(argc, argv, options, loptions, NULL)) != -1)
#else
  while ((c = getopt(argc, argv, options)) != -1)
#endif
    {
    switch(c) 
      {
      case 'h':
        _usage();
      case 'v':
        _version();
      case 'm':
        metric_name = optarg;
        break;
      case 'r':
        operation = CEREBRO_ADMIN_REGISTER;
        break;
      case 'u':
        operation = CEREBRO_ADMIN_UNREGISTER;
        break;
      case 'p':
        operation = CEREBRO_ADMIN_UPDATE;
        break;
      case 's':
        operation = CEREBRO_ADMIN_RESEND;
        break;
      case 'f':
        operation = CEREBRO_ADMIN_FLUSH;
        break;
      case 't':
        metric_value_type = strtol(optarg, &ptr, 10);
        if ((ptr != (optarg + strlen(optarg)))
            || !(metric_value_type >= CEREBRO_DATA_VALUE_TYPE_NONE
                 && metric_value_type <= CEREBRO_DATA_VALUE_TYPE_U_INT64))
          err_exit("invalid metric value type specified");
        break;
      case 'l':
        metric_value = optarg;
        break;
      case 'N':
        _cerebro_set_flags(CEREBRO_METRIC_CONTROL_FLAGS_SEND_NOW);
        break;
#if CEREBRO_DEBUG
      case 'd':
        cerebro_err_set_flags(CEREBRO_ERROR_STDERR);
        break;
#endif /* CEREBRO_DEBUG */
      default:
      case '?':
        fprintf(stderr, "command line option error\n");
        _usage();
      }
    }

  if (!metric_name || operation < 0)
    _usage();

  if (metric_value_type != CEREBRO_DATA_VALUE_TYPE_NONE && !metric_value)
    err_exit("invalid metric value specified");

  if (metric_value_type == CEREBRO_DATA_VALUE_TYPE_NONE)
    {
      metric_value_len = 0;
      metric_value_ptr = NULL;
    }
  else if (metric_value_type == CEREBRO_DATA_VALUE_TYPE_INT32)
    {
      metric_value_int32 = (int32_t)strtol(metric_value, &ptr, 10);
      if (ptr != (metric_value + strlen(metric_value)))
        err_exit("invalid metric value specified");
      metric_value_len = sizeof(int32_t);
      metric_value_ptr = &metric_value_int32;
    }
  else if (metric_value_type == CEREBRO_DATA_VALUE_TYPE_U_INT32)
    {
      metric_value_u_int32 = (u_int32_t)strtoul(metric_value, &ptr, 10);
      if (ptr != (metric_value + strlen(metric_value)))
        err_exit("invalid metric value specified");
      metric_value_len = sizeof(u_int32_t);
      metric_value_ptr = &metric_value_u_int32;
    }
  else if (metric_value_type == CEREBRO_DATA_VALUE_TYPE_FLOAT)
    {
      metric_value_float = (float)strtod(metric_value, &ptr);
      if (ptr != (metric_value + strlen(metric_value)))
        err_exit("invalid metric value specified");
      metric_value_len = sizeof(float);
      metric_value_ptr = &metric_value_float;
    }
  else if (metric_value_type == CEREBRO_DATA_VALUE_TYPE_DOUBLE)
    {
      metric_value_double = strtod(metric_value, &ptr);
      if (ptr != (metric_value + strlen(metric_value)))
        err_exit("invalid metric value specified");
      metric_value_len = sizeof(double);
      metric_value_ptr = &metric_value_double;
    }
  else if (metric_value_type == CEREBRO_DATA_VALUE_TYPE_STRING)
    {
      metric_value_string = metric_value;
      metric_value_len = strlen(metric_value_string);
      if (metric_value_len > CEREBRO_MAX_DATA_STRING_LEN)
        err_exit("string metric value too long");
      metric_value_ptr = metric_value_string;
    }
  else if (metric_value_type == CEREBRO_DATA_VALUE_TYPE_INT64)
    {
      metric_value_int64 = (int64_t)strtoll(metric_value, &ptr, 10);
      if (ptr != (metric_value + strlen(metric_value)))
        err_exit("invalid metric value specified");
      metric_value_len = sizeof(int64_t);
      metric_value_ptr = &metric_value_int64;
    }
  else if (metric_value_type == CEREBRO_DATA_VALUE_TYPE_U_INT64)
    {
      metric_value_u_int64 = (u_int64_t)strtoull(metric_value, &ptr, 10);
      if (ptr != (metric_value + strlen(metric_value)))
        err_exit("invalid metric value specified");
      metric_value_len = sizeof(u_int64_t);
      metric_value_ptr = &metric_value_u_int64;
    }
}

/* 
 * _clean_err_exit
 *
 * Check for and output "nice" error messages for certain errnums.  If
 * a "nice" error message isn't available, just return.
 */
static void
_clean_err_exit(int errnum)
{
  if (errnum == CEREBRO_ERR_CONNECT)
    err_exit("Cannot connect to server");
  if (errnum == CEREBRO_ERR_CONNECT_TIMEOUT)
    err_exit("Timeout connecting to server");
  if (errnum == CEREBRO_ERR_VERSION_INCOMPATIBLE)
    err_exit("Server version not compatible");
  if (errnum == CEREBRO_ERR_METRIC_INVALID)
    err_exit("Invalid metric name specified");
  if (errnum == CEREBRO_ERR_METRIC_MAX)
    err_exit("Maximum metrics reached");
  if (errnum == CEREBRO_ERR_CONFIG_FILE)
    err_exit("Error parsing configuration file");
  if (errnum == CEREBRO_ERR_CONFIG_INPUT)
    err_exit("Invalid configuration input found");
}

int 
main(int argc, char *argv[]) 
{
  const char *func = __FUNCTION__;
  int rv = 0;

  err_init(argv[0]);
  err_set_flags(ERROR_STDERR);
#if CEREBRO_DEBUG
  cerebro_err_init(argv[0]);
#endif /* CEREBRO_DEBUG */

  _init_cerebro_admin();

  _cmdline_parse(argc, argv);
  
  if (operation == CEREBRO_ADMIN_REGISTER)
    rv = cerebro_register_metric(handle, metric_name);
  else if (operation == CEREBRO_ADMIN_UNREGISTER)
    rv = cerebro_unregister_metric(handle, metric_name);
  else if (operation == CEREBRO_ADMIN_UPDATE)
    rv = cerebro_update_metric_value(handle, 
                                     metric_name,
                                     metric_value_type,
                                     metric_value_len,
                                     metric_value_ptr);
  else if (operation == CEREBRO_ADMIN_RESEND)
    rv = cerebro_resend_metric(handle, metric_name);
  else if (operation == CEREBRO_ADMIN_FLUSH)
    rv = cerebro_flush_metric(handle, metric_name);
  
  if (rv < 0)
    {
      char *msg = cerebro_strerror(cerebro_errnum(handle));
      
      _clean_err_exit(cerebro_errnum(handle));
      err_exit("%s: %s", func, msg);
    }
  
  _cleanup_cerebro_stat();
  exit(0);
}
