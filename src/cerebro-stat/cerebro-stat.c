/*****************************************************************************\
 *  $Id: cerebro-stat.c,v 1.1 2005-07-05 21:26:17 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */

#define _GNU_SOURCE
#if HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */
#include <assert.h>
#include <errno.h>

#include "cerebro.h"
#include "cerebro/cerebro_error.h"

#include "hostlist.h"
#include "error.h"
#include "list.h"

/* 
 * External variables for getopt 
 */
extern char *optarg;
extern int optind, opterr, optopt;

/* 
 * Cerebro-Stat Data
 *
 * hostname - hostname to connect to
 * port - port to connect o
 * handle - cerebro handle
 */
static char *hostname = NULL;
static int port = 0;
static cerebro_t handle;
static int metric_list_flag = 0;
static char *metric_name = NULL;

/* 
 * _init_cerebro_stat
 *
 * initialize globals
 */
static void
_init_cerebro_stat(void)
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
	  "Usage: cerebro_stat [OPTIONS]...\n"
	  "  -h         --help              Print help and exit\n"
	  "  -v         --version           Print version and exit\n"
	  "  -o STRING  --hostname=STRING   Cerebro server hostname\n"
	  "  -p INT     --port=INT          Cerebro server port\n"
          "  -l         --metric-list       List all available metrics\n"
          "  -m STRING  --metric=STRING     Output metric data\n");        
#if CEREBRO_DEBUG
  fprintf(stderr,
          "  -d         --debug             Turn on debugging\n");
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
 * _metric_list
 *
 * Output list of all available metrics
 */
static void
_metric_list(void)
{
  const char *func = __FUNCTION__;
  cerebro_metriclist_t m = NULL;
  cerebro_metriclist_iterator_t mitr = NULL;
  List l = NULL;
  ListIterator litr = NULL;
  char *metric_name;

  if (!(m = cerebro_get_metric_names(handle)))
    {
      char *msg = cerebro_strerror(cerebro_errnum(handle));
      err_exit("%s: cerebro_get_metric_names: %s", func, msg);
    }
  
  if (!(mitr = cerebro_metriclist_iterator_create(m)))
    {
      char *msg = cerebro_strerror(cerebro_metriclist_errnum(m));
      err_exit("%s: cerebro_metriclist_iterator_create: %s", func, msg);
    }

  if (!(l = list_create(NULL)))
    err_exit("%s: list_create: %s", func, strerror(errno));

  while (!cerebro_metriclist_iterator_at_end(mitr))
    {
      if (cerebro_metriclist_iterator_metric_name(mitr, &metric_name) < 0)
        {
          char *msg = cerebro_strerror(cerebro_metriclist_iterator_errnum(mitr));
          err_exit("%s: cerebro_metriclist_iterator_next: %s", func, msg);
        }

      if (!list_append(l, metric_name))
        err_exit("%s: list_append: %s", func, strerror(errno));

      if (cerebro_metriclist_iterator_next(mitr) < 0)
        {
          char *msg = cerebro_strerror(cerebro_metriclist_iterator_errnum(mitr));
          err_exit("%s: cerebro_metriclist_iterator_next: %s", func, msg);
        }
    }
  
  if (!(litr = list_iterator_create(l)))
    err_exit("%s: list_iterator_create: %s", func, strerror(errno));

  while ((metric_name = list_next(litr)))
    fprintf(stdout, "%s\n", metric_name);

  /* cerebro_metriclist_destory() and list_destroy() Destroy iterators too */
  (void)cerebro_metriclist_destroy(m);
  (void)list_destroy(l);
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
      {"help",        0, NULL, 'h'},
      {"version",     0, NULL, 'v'},
      {"hostname",    1, NULL, 'o'},
      {"port",        1, NULL, 'p'},
      {"metric-list", 0, NULL, 'l'},
      {"metric",      1, NULL, 'm'},
#if CEREBRO_DEBUG
      {"debug",       0, NULL, 'd'},
#endif /* CEREBRO_DEBUG */
      {0, 0, 0, 0},
  };
#endif /* HAVE_GETOPT_LONG */

  assert(argv);

  strcpy(options, "hvo:p:lm:");
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
      case 'o':
        hostname = optarg;
        if (cerebro_set_hostname(handle, hostname) < 0)
          {
            char *msg = cerebro_strerror(cerebro_errnum(handle));
            err_exit("%s: cerebro_set_hostname: %s", func, msg);
          }
        break;
      case 'p':
        port = strtol(optarg, &ptr, 10);
        if (ptr != (optarg + strlen(optarg)))
          err_exit("invalid port specified");
        if (cerebro_set_port(handle, port) < 0)
          {
            char *msg = cerebro_strerror(cerebro_errnum(handle));
            err_exit("%s: cerebro_set_port: %s", func, msg);
          }
        break;
      case 'l':
        metric_list_flag++;
        break;
      case 'm':
        metric_name = optarg;
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

  if (metric_list_flag && metric_name)
    err_exit("Cannot specify both --metric-list and --metric options");
}

int 
main(int argc, char *argv[]) 
{
  err_init(argv[0]);
  err_set_flags(ERROR_STDERR);
#if CEREBRO_DEBUG
  cerebro_err_init(argv[0]);
#endif /* CEREBRO_DEBUG */

  _init_cerebro_stat();

  _cmdline_parse(argc, argv);
  
  if (metric_list_flag)
    _metric_list();

  _cleanup_cerebro_stat();
  exit(0);
}
