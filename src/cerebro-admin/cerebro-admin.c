/*****************************************************************************\
 *  $Id: cerebro-admin.c,v 1.2 2005-07-21 15:47:42 achu Exp $
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
#define CEREBRO_ADMIN_RESTART     3

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
unsigned int metric_value_type = CEREBRO_METRIC_VALUE_TYPE_NONE;
unsigned int metric_value_len = 0;
char *metric_value = NULL;
int32_t metric_value_int32;
u_int32_t metric_value_u_int32;
float metric_value_float;
double metric_value_double;
char *metric_value_string;
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
	  "Usage: cerebro_admin [OPTIONS]...\n"
	  "  -h         Print help and exit\n"
	  "  -v         Print version and exit\n"
          "  -m STRING  Specify metric name\n"
          "  -r         Register a new metric\n"
          "  -u         Unregister a new metric\n"
          "  -p         Update a metric's value\n"
          "  -s         Restart a metric\n"
          "  -t INT     Specify metric type\n"
          "     %d - none (default)\n"
          "     %d - int32\n"
          "     %d - u_int32\n"
          "     %d - float\n"
          "     %d - double\n"
          "     %d - string\n"
          "  -l STRING  Specify metric value\n",
          CEREBRO_METRIC_VALUE_TYPE_NONE,
          CEREBRO_METRIC_VALUE_TYPE_INT32,
          CEREBRO_METRIC_VALUE_TYPE_U_INT32,
          CEREBRO_METRIC_VALUE_TYPE_FLOAT,
          CEREBRO_METRIC_VALUE_TYPE_DOUBLE,
          CEREBRO_METRIC_VALUE_TYPE_STRING
          );
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
      {"restart",           0, NULL, 's'},
      {"metric-value-type", 1, NULL, 't'},
      {"metric-value",      1, NULL, 'l'},
#if CEREBRO_DEBUG
      {"debug",       0, NULL, 'd'},
#endif /* CEREBRO_DEBUG */
      {0, 0, 0, 0},
  };
#endif /* HAVE_GETOPT_LONG */

  assert(argv);

  strcpy(options, "hvm:rupst:l:");
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
        operation = CEREBRO_ADMIN_RESTART;
        break;
      case 't':
        metric_value_type = strtol(optarg, &ptr, 10);
        if ((ptr != (optarg + strlen(optarg)))
            || !(metric_value_type >= CEREBRO_METRIC_VALUE_TYPE_NONE
                 && metric_value_type <= CEREBRO_METRIC_VALUE_TYPE_STRING))
          err_exit("invalid metric value type specified");
        break;
      case 'l':
        metric_value = optarg;
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

  if (metric_value_type != CEREBRO_METRIC_VALUE_TYPE_NONE && !metric_value)
    err_exit("invalid metric value specified");

  if (metric_value_type == CEREBRO_METRIC_VALUE_TYPE_INT32)
    {
      metric_value_int32 = strtol(metric_value, &ptr, 10);
      if (ptr != (metric_value + strlen(metric_value)))
        err_exit("invalid metric value specified");
      metric_value_len = sizeof(int32_t);
      metric_value_ptr = &metric_value_int32;
    }
  else if (metric_value_type == CEREBRO_METRIC_VALUE_TYPE_U_INT32)
    {
      metric_value_u_int32 = (u_int32_t)strtoul(metric_value, &ptr, 10);
      if (ptr != (metric_value + strlen(metric_value)))
        err_exit("invalid metric value specified");
      metric_value_len = sizeof(u_int32_t);
      metric_value_ptr = &metric_value_u_int32;
    }
  else if (metric_value_type == CEREBRO_METRIC_VALUE_TYPE_FLOAT)
    {
      metric_value_float = (float)strtod(metric_value, &ptr);
      if (ptr != (metric_value + strlen(metric_value)))
        err_exit("invalid metric value specified");
      metric_value_len = sizeof(float);
      metric_value_ptr = &metric_value_float;
    }
  else if (metric_value_type == CEREBRO_METRIC_VALUE_TYPE_DOUBLE)
    {
      metric_value_double = strtod(metric_value, &ptr);
      if (ptr != (metric_value + strlen(metric_value)))
        err_exit("invalid metric value specified");
      metric_value_len = sizeof(double);
      metric_value_ptr = &metric_value_double;
    }
  else if (metric_value_type == CEREBRO_METRIC_VALUE_TYPE_STRING)
    {
      metric_value_string = metric_value;
      metric_value_len = strlen(metric_value_string);
      if (metric_value_len > CEREBRO_MAX_METRIC_STRING_LEN)
        err_exit("string metric value too long");
      metric_value_ptr = metric_value_string;
    }
}

/* 
 * _try_nice_err_exit
 *
 * Check for and output "nice" error messages for certain errnums
 */
static void
_try_nice_err_exit(int errnum)
{
  if (errnum == CEREBRO_ERR_CONNECT)
    err_exit("Cannot connect to server");
  if (errnum == CEREBRO_ERR_CONNECT_TIMEOUT)
    err_exit("Timeout connecting to server");
  if (errnum == CEREBRO_ERR_VERSION_INCOMPATIBLE)
    err_exit("Server version not compatible");
  if (errnum == CEREBRO_ERR_METRIC_INVALID)
    err_exit("Unknown metric name specified");
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
  else if (operation == CEREBRO_ADMIN_RESTART)
    rv = cerebro_restart_metric(handle, metric_name);
  
  if (rv < 0)
    {
      char *msg = cerebro_strerror(cerebro_errnum(handle));
      
      _try_nice_err_exit(cerebro_errnum(handle));
      err_exit("%s: cerebro_register_metric: %s", func, msg);
    }
  
  _cleanup_cerebro_stat();
  exit(0);
}
