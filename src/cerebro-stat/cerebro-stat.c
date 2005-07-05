/*****************************************************************************\
 *  $Id: cerebro-stat.c,v 1.2 2005-07-05 22:25:06 achu Exp $
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
#include "cerebro/cerebro_error.h"

#include "hostlist.h"
#include "error.h"
#include "list.h"

#define NONE_STRING   "(none)"
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
 * node_metric_data
 *
 * Contains node and metric data
 */
struct node_metric_data {
  char *nodename;
  unsigned int metric_value_type;
  unsigned int metric_value_len;
  void *metric_value;
};

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
  char *str;

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
      if (cerebro_metriclist_iterator_metric_name(mitr, &str) < 0)
        {
          char *msg = cerebro_strerror(cerebro_metriclist_iterator_errnum(mitr));
          err_exit("%s: cerebro_metriclist_iterator_metric_name: %s", func, msg);
        }

      if (!list_append(l, str))
        err_exit("%s: list_append: %s", func, strerror(errno));

      if (cerebro_metriclist_iterator_next(mitr) < 0)
        {
          char *msg = cerebro_strerror(cerebro_metriclist_iterator_errnum(mitr));
          err_exit("%s: cerebro_metriclist_iterator_next: %s", func, msg);
        }
    }
  
  if (!(litr = list_iterator_create(l)))
    err_exit("%s: list_iterator_create: %s", func, strerror(errno));

  while ((str = list_next(litr)))
    fprintf(stdout, "%s\n", str);

  /* list_destroy() and cerebro_metriclist_destory() destroy iterators too */
  (void)list_destroy(l);
  (void)cerebro_metriclist_destroy(m);
}

/*
 * _metric_data
 *
 * Output list of all available metrics
 */
static void
_metric_data(void)
{
  const char *func = __FUNCTION__;
  cerebro_nodelist_t n = NULL;
  cerebro_nodelist_iterator_t nitr = NULL;
  List l = NULL;
  ListIterator litr = NULL;
  struct node_metric_data *data = NULL;

  if (!(n = cerebro_get_metric_data(handle, metric_name)))
    {
      char *msg = cerebro_strerror(cerebro_errnum(handle));
      err_exit("%s: cerebro_get_metric_names: %s", func, msg);
    }
  
  if (!(nitr = cerebro_nodelist_iterator_create(n)))
    {
      char *msg = cerebro_strerror(cerebro_nodelist_errnum(n));
      err_exit("%s: cerebro_nodelist_iterator_create: %s", func, msg);
    }

  if (!(l = list_create((ListDelF)free)))
    err_exit("%s: list_create: %s", func, strerror(errno));

  while (!cerebro_nodelist_iterator_at_end(nitr))
    {
      data = NULL;

      if (!(data = malloc(sizeof(struct node_metric_data))))
        err_exit("%s: malloc: %s", func, strerror(errno));
      memset(data, '\0', sizeof(struct node_metric_data));

      if (cerebro_nodelist_iterator_nodename(nitr, &(data->nodename)) < 0)
        {
          char *msg = cerebro_strerror(cerebro_nodelist_iterator_errnum(nitr));
          err_exit("%s: cerebro_nodelist_iterator_nodename: %s", func, msg);
        }

      if (cerebro_nodelist_iterator_metric_value(nitr, 
                                                 &(data->metric_value_type),
                                                 &(data->metric_value_len),
                                                 &(data->metric_value)) < 0)
        {
          char *msg = cerebro_strerror(cerebro_nodelist_iterator_errnum(nitr));
          err_exit("%s: cerebro_nodelist_iterator_metric_value: %s", func, msg);
        }

      if (!list_append(l, data))
        err_exit("%s: list_append: %s", func, strerror(errno));

      if (cerebro_nodelist_iterator_next(nitr) < 0)
        {
          char *msg = cerebro_strerror(cerebro_nodelist_iterator_errnum(nitr));
          err_exit("%s: cerebro_nodelist_iterator_next: %s", func, msg);
        }
    }
  
  if (!(litr = list_iterator_create(l)))
    err_exit("%s: list_iterator_create: %s", func, strerror(errno));

  while ((data = list_next(litr)))
    {
      unsigned int mtype, mlen;
      
      fprintf(stdout, "%s: ", data->nodename);

      mtype = data->metric_value_type;
      mlen = data->metric_value_len;

      if (mtype == CEREBRO_METRIC_VALUE_TYPE_NONE)
        {
#if CEREBRO_DEBUG
          if (!mlen)
            err_exit("%s: invalid metric length: %d %d", func, mtype, mlen);
#endif /* CEREBRO_DEBUG */
          fprintf(stdout, "%s", NONE_STRING);
        }
      else if (mtype == CEREBRO_METRIC_VALUE_TYPE_INT32)
        {
#if CEREBRO_DEBUG
          if (mlen != sizeof(int32_t))
            err_exit("%s: invalid metric length: %d %d", func, mtype, mlen);
#endif /* CEREBRO_DEBUG */
          fprintf(stdout, "%d", *((int32_t *)data->metric_value));
        }
      else if (mtype == CEREBRO_METRIC_VALUE_TYPE_U_INT32)
        {
#if CEREBRO_DEBUG
          if (mlen != sizeof(u_int32_t))
            err_exit("%s: invalid metric length: %d %d", func, mtype, mlen);
#endif /* CEREBRO_DEBUG */
          fprintf(stdout, "%u", *((u_int32_t *)data->metric_value));
        }
      else if (mtype == CEREBRO_METRIC_VALUE_TYPE_FLOAT)
        {
#if CEREBRO_DEBUG
          if (mlen != sizeof(float))
            err_exit("%s: invalid metric length: %d %d", func, mtype, mlen);
#endif /* CEREBRO_DEBUG */
          fprintf(stdout, "%f", *((float *)data->metric_value));
        }
      else if (mtype == CEREBRO_METRIC_VALUE_TYPE_DOUBLE)
        {
#if CEREBRO_DEBUG
          if (mlen != sizeof(double))
            err_exit("%s: invalid metric length: %d %d", func, mtype, mlen);
#endif /* CEREBRO_DEBUG */
          fprintf(stdout, "%f", *((double *)data->metric_value));
        }
      else if (mtype == CEREBRO_METRIC_VALUE_TYPE_STRING)
        {
#if CEREBRO_DEBUG
          if (mlen > CEREBRO_MAX_METRIC_STRING_LEN)
            err_exit("%s: invalid metric length: %d %d", func, mtype, mlen);
#endif /* CEREBRO_DEBUG */
          fprintf(stdout, "%s", (char *)data->metric_value);
        }
#if CEREBRO_DEBUG
      else
        err_exit("%s: invalid metric type: %d", func, mtype);
#endif /* CEREBRO_DEBUG */

      fprintf(stdout, "\n");
    }

  /* cerebro_nodelist_destory() and list_destroy() Destroy iterators too */
  (void)list_destroy(l);
  (void)cerebro_nodelist_destroy(n);
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

  if (!metric_list_flag && !metric_name)
    _usage();
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

  if (metric_name)
    _metric_data();

  _cleanup_cerebro_stat();
  exit(0);
}
