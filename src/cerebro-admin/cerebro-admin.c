/*****************************************************************************\
 *  $Id: cerebro-admin.c,v 1.1 2005-07-20 21:51:47 achu Exp $
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
#include "wrappers.h"

#define CEREBRO_STAT_BUFLEN        16384
#define CEREBRO_STAT_NONE_STRING   "(none)"
#define CEREBRO_STAT_NEWLINE       0
#define CEREBRO_STAT_HOSTRANGE     1

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
	  "  -v         Print version and exit\n");
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
  const char *func = __FUNCTION__;
  char options[1024];
  char *ptr;
  int c;

#if HAVE_GETOPT_LONG
  struct option loptions[] = 
    {
      {"help",                  0, NULL, 'h'},
      {"version",               0, NULL, 'v'},
#if CEREBRO_DEBUG
      {"debug",                 0, NULL, 'd'},
#endif /* CEREBRO_DEBUG */
      {0, 0, 0, 0},
  };
#endif /* HAVE_GETOPT_LONG */

  assert(argv);

  strcpy(options, "hv");
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
  err_init(argv[0]);
  err_set_flags(ERROR_STDERR);
#if CEREBRO_DEBUG
  cerebro_err_init(argv[0]);
#endif /* CEREBRO_DEBUG */

  _init_cerebro_admin();

  _cmdline_parse(argc, argv);
  
  _cleanup_cerebro_stat();
  exit(0);
}
