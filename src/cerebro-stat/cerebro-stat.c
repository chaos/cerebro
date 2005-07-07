/*****************************************************************************\
 *  $Id: cerebro-stat.c,v 1.8 2005-07-07 16:28:34 achu Exp $
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

#include "hostlist.h"
#include "error.h"
#include "hash.h"
#include "list.h"
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
 * Cerebro-Stat Data
 *
 * hostname - hostname to connect to
 * port - port to connect o
 * handle - cerebro handle
 */
static char *hostname = NULL;
static int port = 0;
static char output_type = CEREBRO_STAT_NEWLINE;
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
 * hostrange_data
 *
 * Contains hostlist and key data
 */
struct hostrange_data {
  hostlist_t hl;
  char *key;
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
	  "  -h         Print help and exit\n"
	  "  -v         Print version and exit\n"
	  "  -o STRING  Cerebro metric server hostname\n"
	  "  -p INT     Cerebro metric server port\n"
          "  -l         List all available metrics\n"
          "  -m STRING  Output metric data\n"
          "  -U         Only output metrics from up nodes\n"
          "  -D         Output '%s' for down nodes\n"
          "  -N         Output '%s' for nodes not monitoring a metric\n"
          "  -n         Output nodes one per line\n"
          "  -q         Output nodes in hostrange format\n", 
          CEREBRO_STAT_NONE_STRING, CEREBRO_STAT_NONE_STRING);
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
  const char *func = __FUNCTION__;
  char options[1024];
  char *ptr;
  int c;

#if HAVE_GETOPT_LONG
  struct option loptions[] = 
    {
      {"help",                  0, NULL, 'h'},
      {"version",               0, NULL, 'v'},
      {"hostname",              1, NULL, 'o'},
      {"port",                  1, NULL, 'p'},
      {"metric-list",           0, NULL, 'l'},
      {"metric",                1, NULL, 'm'},
      {"up-only",               0, NULL, 'U'},
      {"none-if-down",          0, NULL, 'D'},
      {"none-if-not-monitored", 0, NULL, 'N'},
      {"newline",               0, NULL, 'n'},
      {"hostrange",             0, NULL, 'q'},
#if CEREBRO_DEBUG
      {"debug",                 0, NULL, 'd'},
#endif /* CEREBRO_DEBUG */
      {0, 0, 0, 0},
  };
#endif /* HAVE_GETOPT_LONG */

  assert(argv);

  strcpy(options, "hvo:p:lm:UDNnq");
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
      case 'U':
        _cerebro_set_flags(CEREBRO_METRIC_FLAGS_UP_ONLY);
        break;
      case 'D':
        _cerebro_set_flags(CEREBRO_METRIC_FLAGS_NONE_IF_DOWN);
        break;
      case 'N':
        _cerebro_set_flags(CEREBRO_METRIC_FLAGS_NONE_IF_NOT_MONITORED);
        break;
      case 'n':
        output_type = CEREBRO_STAT_NEWLINE;
        break;
      case 'q':
        output_type = CEREBRO_STAT_HOSTRANGE;
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
  if (errnum == CEREBRO_ERR_HOSTNAME)
    err_exit("Invalid hostname");
  if (errnum == CEREBRO_ERR_VERSION_INCOMPATIBLE)
    err_exit("Server version not compatible");
  if (errnum == CEREBRO_ERR_METRIC_UNKNOWN)
    err_exit("Unknown metric name specified");
  if (errnum == CEREBRO_ERR_CONFIG_FILE)
    err_exit("Error parsing configuration file");
  if (errnum == CEREBRO_ERR_CONFIG_INPUT)
    err_exit("Invalid configuration input found");
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

      _try_nice_err_exit(cerebro_errnum(handle));
      err_exit("%s: cerebro_get_metric_names: %s", func, msg);
    }
  
  if (!(mitr = cerebro_metriclist_iterator_create(m)))
    {
      char *msg = cerebro_strerror(cerebro_metriclist_errnum(m));
      err_exit("%s: cerebro_metriclist_iterator_create: %s", func, msg);
    }

  l = List_create(NULL);

  while (!cerebro_metriclist_iterator_at_end(mitr))
    {
      if (cerebro_metriclist_iterator_metric_name(mitr, &str) < 0)
        {
          char *msg = cerebro_strerror(cerebro_metriclist_iterator_errnum(mitr));
          err_exit("%s: cerebro_metriclist_iterator_metric_name: %s", func, msg);
        }

      List_append(l, str);

      if (cerebro_metriclist_iterator_next(mitr) < 0)
        {
          char *msg = cerebro_strerror(cerebro_metriclist_iterator_errnum(mitr));
          err_exit("%s: cerebro_metriclist_iterator_next: %s", func, msg);
        }
    }
  
  litr = List_iterator_create(l);

  while ((str = list_next(litr)))
    fprintf(stdout, "%s\n", str);

  /* list_destroy() and cerebro_metriclist_destory() destroy iterators too */
  (void)list_destroy(l);
  (void)cerebro_metriclist_destroy(m);
}

/* 
 * _metric_value_str
 *
 * Get the metric value string
 */
static void
_metric_value_str(struct node_metric_data *data, char *buf, unsigned int buflen)
{
  const char *func = __FUNCTION__;
  unsigned int mtype, mlen;
  int rv;

  assert(data && buf && buflen > 0);

  mtype = data->metric_value_type;
  mlen = data->metric_value_len;

  if (mtype == CEREBRO_METRIC_VALUE_TYPE_NONE)
    {
#if CEREBRO_DEBUG
      if (mlen)
        err_exit("%s: invalid metric length: %d %d", func, mtype, mlen);
#endif /* CEREBRO_DEBUG */
      rv = snprintf(buf, buflen, "%s", CEREBRO_STAT_NONE_STRING);
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_INT32)
    {
#if CEREBRO_DEBUG
      if (mlen != sizeof(int32_t))
        err_exit("%s: invalid metric length: %d %d", func, mtype, mlen);
#endif /* CEREBRO_DEBUG */
      rv = snprintf(buf, buflen, "%d", *((int32_t *)data->metric_value));
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_U_INT32)
    {
#if CEREBRO_DEBUG
      if (mlen != sizeof(u_int32_t))
        err_exit("%s: invalid metric length: %d %d", func, mtype, mlen);
#endif /* CEREBRO_DEBUG */
      rv = snprintf(buf, buflen, "%u", *((u_int32_t *)data->metric_value));
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_FLOAT)
    {
#if CEREBRO_DEBUG
      if (mlen != sizeof(float))
        err_exit("%s: invalid metric length: %d %d", func, mtype, mlen);
#endif /* CEREBRO_DEBUG */
      rv = snprintf(buf, buflen, "%f", *((float *)data->metric_value));
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_DOUBLE)
    {
#if CEREBRO_DEBUG
      if (mlen != sizeof(double))
        err_exit("%s: invalid metric length: %d %d", func, mtype, mlen);
#endif /* CEREBRO_DEBUG */
      rv = snprintf(buf, buflen, "%f", *((double *)data->metric_value));
    }
  else if (mtype == CEREBRO_METRIC_VALUE_TYPE_STRING)
    {
#if CEREBRO_DEBUG
      if (mlen > CEREBRO_MAX_METRIC_STRING_LEN)
        err_exit("%s: invalid metric length: %d %d", func, mtype, mlen);
#endif /* CEREBRO_DEBUG */
      rv = snprintf(buf, buflen, "%s", (char *)data->metric_value);
    }
#if CEREBRO_DEBUG
  else
    err_exit("%s: invalid metric type: %d", func, mtype);
#endif /* CEREBRO_DEBUG */

  if (rv >= buflen)
    err_exit("%s: truncated output: %d", func, mlen);
}

/* 
 * _cluster_nodes_output
 *
 * Output cluster nodes
 */
static void
_cluster_nodes_output(List l)
{
  struct node_metric_data *data = NULL;
  hostlist_t hl = NULL;
  ListIterator litr = NULL;

  assert(l);

  litr = List_iterator_create(l);

  if (output_type == CEREBRO_STAT_HOSTRANGE)
    hl = Hostlist_create(NULL);

  while ((data = list_next(litr)))
    {
      if (output_type == CEREBRO_STAT_NEWLINE)
        fprintf(stdout, "%s\n", data->nodename);
      if (output_type == CEREBRO_STAT_HOSTRANGE)
        Hostlist_push(hl, data->nodename);
    }

  if (output_type == CEREBRO_STAT_HOSTRANGE)
    {
      char hstr[CEREBRO_STAT_BUFLEN];

      Hostlist_sort(hl);
      Hostlist_uniq(hl);

      memset(hstr, '\0', CEREBRO_STAT_BUFLEN);
      Hostlist_ranged_string(hl, CEREBRO_STAT_BUFLEN, hstr);

      fprintf(stdout, "%s\n", hstr);
    }

  /* No need to destroy list iterator, caller will destroy List */
  if (hl)
    Hostlist_destroy(hl);
}

/* 
 * _newline_output
 *
 * Output metric data, one node per line
 */
static void
_newline_output(List l)
{
  struct node_metric_data *data = NULL;
  ListIterator litr = NULL;

  assert(l);

  litr = List_iterator_create(l);

  while ((data = list_next(litr)))
    {
      char buf[CEREBRO_STAT_BUFLEN];

      memset(buf, '\0', CEREBRO_STAT_BUFLEN);
      _metric_value_str(data, buf, CEREBRO_STAT_BUFLEN);
      fprintf(stdout, "%s: %s\n", data->nodename, buf);
    }

  /* No need to destroy list iterator, caller will destroy List */
}

/* 
 * _hostrange_data_destroy
 *
 * Free hostrange_data structures
 */
static void
_hostrange_data_destroy(void *data)
{
  struct hostrange_data *hd;

  assert(data);

  hd = (struct hostrange_data *)data;
  Hostlist_destroy(hd->hl);
  Free(hd->key);
  Free(hd);
}

/* 
 * _hostrange_output_data
 * 
 * Callback for hash_for_each to output a hostrange for a metric data
 */
static int
_hostrange_output_data(void *data, const void *key, void *arg)
{
  struct hostrange_data *hd;
  char hstr[CEREBRO_STAT_BUFLEN];

  assert(data && key);

  hd = (struct hostrange_data *)data;
  
  memset(hstr, '\0', CEREBRO_STAT_BUFLEN);
  Hostlist_ranged_string(hd->hl, CEREBRO_STAT_BUFLEN, hstr);

  fprintf(stdout, "%s: %s\n", hstr, (char *)key);
  return 1;
}

/* 
 * _hostrange_output
 *
 * Output metric data in hostrange format
 */
static void
_hostrange_output(List l)
{
  const char *func = __FUNCTION__;
  struct node_metric_data *data = NULL;
  ListIterator litr = NULL;
  unsigned int count;
  hash_t h;

  assert(l);

  count = List_count(l);

#if CEREBRO_DEBUG
  if (!count)
    err_exit("%s: invalid count", func);
#endif /* CEREBRO_DEBUG */

  h = Hash_create(count, 
                  (hash_key_f)hash_key_string,
                  (hash_cmp_f)strcmp,
                  (hash_del_f)_hostrange_data_destroy);

  litr = List_iterator_create(l);

  while ((data = list_next(litr)))
    {
      char buf[CEREBRO_STAT_BUFLEN];
      struct hostrange_data *hd;

      _metric_value_str(data, buf, CEREBRO_STAT_BUFLEN);

      if (!(hd = Hash_find(h, buf)))
        {
          
          hd = Malloc(sizeof(struct hostrange_data));
          hd->hl = Hostlist_create(NULL);
          hd->key = Strdup(buf);

          Hash_insert(h, hd->key, hd);
        }

      Hostlist_push(hd->hl, data->nodename);
    }

  Hash_for_each(h, _hostrange_output_data, NULL);

  /* No need to destroy list iterator, caller will destroy List */
  Hash_destroy(h);
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
  struct node_metric_data *data = NULL;
  List l = NULL;

  if (!(n = cerebro_get_metric_data(handle, metric_name)))
    {
      char *msg = cerebro_strerror(cerebro_errnum(handle));

      _try_nice_err_exit(cerebro_errnum(handle));
      err_exit("%s: cerebro_get_metric_data: %s", func, msg);
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

  /* 
   * XXX need cluster nodes special case
   */
  if (!strcmp(metric_name, CEREBRO_METRIC_CLUSTER_NODES))
    _cluster_nodes_output(l);
  else if (output_type == CEREBRO_STAT_NEWLINE)
    _newline_output(l);
  else if (output_type == CEREBRO_STAT_HOSTRANGE)
    _hostrange_output(l);

  /* list_destroy() and cerebro_nodelist_destory() destroy iterators too */
  (void)list_destroy(l);
  (void)cerebro_nodelist_destroy(n);
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
  
  if (metric_list_flag || !strcmp(metric_name, CEREBRO_METRIC_METRIC_NAMES))
    _metric_list();

  if (metric_name)
    _metric_data();

  _cleanup_cerebro_stat();
  exit(0);
}
