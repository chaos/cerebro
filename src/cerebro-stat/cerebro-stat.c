/*****************************************************************************\
 *  $id: cerebro-stat.c,v 1.18 2005/07/26 22:30:56 achu Exp $
 *****************************************************************************
 *  Copyright (C) 2005 The Regents of the University of California.
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
 *  with Genders; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
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

#define CEREBRO_STAT_BUFLEN          65536
#define CEREBRO_STAT_NONE_STRING     "(none)"
#define CEREBRO_STAT_UNKNOWN_STRING  "unknown"
#define CEREBRO_STAT_NEWLINE         0
#define CEREBRO_STAT_HOSTRANGE       1

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
static char *event_name = NULL;
static int event_list_flag = 0;
static int metric_list_flag = 0;
static char *metric_name = NULL;
static int metric_received_time_flag = 0;

/*  
 * node_metric_data
 *
 * Contains node and metric data
 */
struct node_metric_data {
  char *nodename;
  unsigned int metric_value_received_time;
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
	  "  -o STRING  Cerebro server hostname\n"
	  "  -p INT     Cerebro server port\n"
          "  -z         List all available events that can be monitored\n"
          "  -e STRING  Monitor the specified event\n"
          "  -l         List all available metrics that can be queried\n"
          "  -m STRING  Output metric data for the specified metric\n"
          "  -U         Only output metrics from up nodes\n"
          "  -D         Output '%s' for down nodes\n"
          "  -N         Output '%s' for nodes not monitoring a metric\n"
          "  -n         Output nodes one per line\n"
          "  -q         Output nodes in hostrange format\n"
          "  -t         Output metric receive times\n",
          CEREBRO_STAT_NONE_STRING, 
          CEREBRO_STAT_NONE_STRING);
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
      {"metric-received-time",  0, NULL, 't'},
      {"event-list",            0, NULL, 'z'},
      {"event",                 1, NULL, 'e'},
#if CEREBRO_DEBUG
      {"debug",                 0, NULL, 'd'},
#endif /* CEREBRO_DEBUG */
      {0, 0, 0, 0},
    };
#endif /* HAVE_GETOPT_LONG */

  assert(argv);

  strcpy(options, "hvo:p:ze:lm:UDNnqt");
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
        _cerebro_set_flags(CEREBRO_METRIC_DATA_FLAGS_UP_ONLY);
        break;
      case 'D':
        _cerebro_set_flags(CEREBRO_METRIC_DATA_FLAGS_NONE_IF_DOWN);
        break;
      case 'N':
        _cerebro_set_flags(CEREBRO_METRIC_DATA_FLAGS_NONE_IF_NOT_MONITORED);
        break;
      case 'n':
        output_type = CEREBRO_STAT_NEWLINE;
        break;
      case 'q':
        output_type = CEREBRO_STAT_HOSTRANGE;
        break;
      case 't':
        metric_received_time_flag++;
        break;
      case 'e':
        event_name = optarg;
        break;
      case 'z':
        event_list_flag++;
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

  if (((metric_list_flag ? 1 : 0)
       + (metric_name ? 1 : 0)
       + (event_name ? 1 : 0)
       + (event_list_flag ? 1 : 0)) > 1)
    err_exit("Specify one of --event-list, --event, --metric-list, and --metric options");
  
  if (!event_name && !event_list_flag && !metric_list_flag && !metric_name)
    _usage();
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
  if (errnum == CEREBRO_ERR_HOSTNAME)
    err_exit("Invalid hostname");
  if (errnum == CEREBRO_ERR_VERSION_INCOMPATIBLE)
    err_exit("Server version not compatible");
  if (errnum == CEREBRO_ERR_METRIC_INVALID)
    err_exit("Invalid metric name specified");
  if (errnum == CEREBRO_ERR_EVENT_INVALID)
    err_exit("Invalid event name specified");
  if (errnum == CEREBRO_ERR_CONFIG_FILE)
    err_exit("Error parsing configuration file");
  if (errnum == CEREBRO_ERR_CONFIG_INPUT)
    err_exit("Invalid configuration input found");
}

/*
 * _event_list
 *
 * Output list of all available events
 */
static void
_event_list(void)
{
  const char *func = __FUNCTION__;
  cerebro_namelist_t m = NULL;
  cerebro_namelist_iterator_t mitr = NULL;
  List l = NULL;
  ListIterator litr = NULL;
  char *str;

  if (!(m = cerebro_get_event_names(handle)))
    {
      char *msg = cerebro_strerror(cerebro_errnum(handle));

      _clean_err_exit(cerebro_errnum(handle));
      err_exit("%s: cerebro_get_event_names: %s", func, msg);
    }
  
  if (!(mitr = cerebro_namelist_iterator_create(m)))
    {
      char *msg = cerebro_strerror(cerebro_namelist_errnum(m));
      err_exit("%s: cerebro_namelist_iterator_create: %s", func, msg);
    }

  l = List_create(NULL);

  while (!cerebro_namelist_iterator_at_end(mitr))
    {
      if (cerebro_namelist_iterator_name(mitr, &str) < 0)
        {
          char *msg = cerebro_strerror(cerebro_namelist_iterator_errnum(mitr));
          err_exit("%s: cerebro_namelist_iterator_event_name: %s", func, msg);
        }

      List_append(l, str);

      if (cerebro_namelist_iterator_next(mitr) < 0)
        {
          char *msg = cerebro_strerror(cerebro_namelist_iterator_errnum(mitr));
          err_exit("%s: cerebro_namelist_iterator_next: %s", func, msg);
        }
    }
  
  litr = List_iterator_create(l);

  while ((str = list_next(litr)))
    fprintf(stdout, "%s\n", str);

  /* List_destroy() and cerebro_namelist_destory() destroy iterators too */
  List_destroy(l);
  (void)cerebro_namelist_destroy(m);
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
  cerebro_namelist_t m = NULL;
  cerebro_namelist_iterator_t mitr = NULL;
  List l = NULL;
  ListIterator litr = NULL;
  char *str;

  if (!(m = cerebro_get_metric_names(handle)))
    {
      char *msg = cerebro_strerror(cerebro_errnum(handle));

      _clean_err_exit(cerebro_errnum(handle));
      err_exit("%s: cerebro_get_metric_names: %s", func, msg);
    }
  
  if (!(mitr = cerebro_namelist_iterator_create(m)))
    {
      char *msg = cerebro_strerror(cerebro_namelist_errnum(m));
      err_exit("%s: cerebro_namelist_iterator_create: %s", func, msg);
    }

  l = List_create(NULL);

  while (!cerebro_namelist_iterator_at_end(mitr))
    {
      if (cerebro_namelist_iterator_name(mitr, &str) < 0)
        {
          char *msg = cerebro_strerror(cerebro_namelist_iterator_errnum(mitr));
          err_exit("%s: cerebro_namelist_iterator_name: %s", func, msg);
        }

      List_append(l, str);

      if (cerebro_namelist_iterator_next(mitr) < 0)
        {
          char *msg = cerebro_strerror(cerebro_namelist_iterator_errnum(mitr));
          err_exit("%s: cerebro_namelist_iterator_next: %s", func, msg);
        }
    }
  
  litr = List_iterator_create(l);

  while ((str = list_next(litr)))
    fprintf(stdout, "%s\n", str);

  /* List_destroy() and cerebro_namelist_destory() destroy iterators too */
  List_destroy(l);
  (void)cerebro_namelist_destroy(m);
}

/* 
 * _metric_value_str
 *
 * Get the metric value string
 */
static void
_metric_value_str(unsigned int mtype,
                  unsigned int mlen,
                  void *mvalue,
                  char *buf, 
                  unsigned int buflen)
{
  const char *func = __FUNCTION__;
#if CEREBRO_DEBUG
  int mlen_flag;
#endif /* CEREBRO_DEBUG */
  int rv = 0;

  assert(mvalue && buf && buflen > 0);

#if CEREBRO_DEBUG
  if (mtype == CEREBRO_DATA_VALUE_TYPE_NONE)
    mlen_flag = (mlen) ? 1 : 0;
  else if (mtype == CEREBRO_DATA_VALUE_TYPE_INT32)
    mlen_flag = (mlen != sizeof(int32_t)) ? 1 : 0;
  else if (mtype == CEREBRO_DATA_VALUE_TYPE_U_INT32)
    mlen_flag = (mlen != sizeof(u_int32_t)) ? 1 : 0;
  else if (mtype == CEREBRO_DATA_VALUE_TYPE_FLOAT)
    mlen_flag = (mlen != sizeof(float)) ? 1 : 0;
  else if (mtype == CEREBRO_DATA_VALUE_TYPE_DOUBLE)
    mlen_flag = (mlen != sizeof(double)) ? 1 : 0;
  else if (mtype == CEREBRO_DATA_VALUE_TYPE_STRING)
    mlen_flag = (mlen > CEREBRO_MAX_DATA_STRING_LEN) ? 1 : 0;
  else if (mtype == CEREBRO_DATA_VALUE_TYPE_INT64)
    mlen_flag = (mlen != sizeof(int64_t)) ? 1 : 0;
  else if (mtype == CEREBRO_DATA_VALUE_TYPE_U_INT64)
    mlen_flag = (mlen != sizeof(u_int64_t)) ? 1 : 0;
  else
    err_exit("%s: invalid metric type: %d", func, mtype);

  if (mlen_flag)
    err_exit("%s: invalid metric length: %d %d", func, mtype, mlen);
#endif /* CEREBRO_DEBUG */

  if (mtype == CEREBRO_DATA_VALUE_TYPE_NONE)
    rv = snprintf(buf, buflen, "%s", CEREBRO_STAT_NONE_STRING);
  else if (mtype == CEREBRO_DATA_VALUE_TYPE_INT32)
    rv = snprintf(buf, buflen, "%d", *((int32_t *)mvalue));
  else if (mtype == CEREBRO_DATA_VALUE_TYPE_U_INT32)
    rv = snprintf(buf, buflen, "%u", *((u_int32_t *)mvalue));
  else if (mtype == CEREBRO_DATA_VALUE_TYPE_FLOAT)
    rv = snprintf(buf, buflen, "%f", *((float *)mvalue));
  else if (mtype == CEREBRO_DATA_VALUE_TYPE_DOUBLE)
    rv = snprintf(buf, buflen, "%f", *((double *)mvalue));
  else if (mtype == CEREBRO_DATA_VALUE_TYPE_STRING)
    rv = snprintf(buf, buflen, "%s", (char *)mvalue);
#if SIZEOF_LONG == 4
  else if (mtype == CEREBRO_DATA_VALUE_TYPE_INT64)
    rv = snprintf(buf, buflen, "%lld", *((int64_t *)mvalue));
  else if (mtype == CEREBRO_DATA_VALUE_TYPE_U_INT64)
    rv = snprintf(buf, buflen, "%llu", *((u_int64_t *)mvalue));
#else  /* SIZEOF_LONG == 8 */
  else if (mtype == CEREBRO_DATA_VALUE_TYPE_INT64)
    rv = snprintf(buf, buflen, "%ld", *((int64_t *)mvalue));
  else if (mtype == CEREBRO_DATA_VALUE_TYPE_U_INT64)
    rv = snprintf(buf, buflen, "%lu", *((u_int64_t *)mvalue));
#endif /* SIZEOF_LONG == 8 */
  
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
      char vbuf[CEREBRO_STAT_BUFLEN];

      memset(vbuf, '\0', CEREBRO_STAT_BUFLEN);
      _metric_value_str(data->metric_value_type,
                        data->metric_value_len,
                        data->metric_value,
                        vbuf,
                        CEREBRO_STAT_BUFLEN);
      if (metric_received_time_flag)
        {
          char tbuf[CEREBRO_STAT_BUFLEN];
          
          memset(tbuf, '\0', CEREBRO_STAT_BUFLEN);
          if (data->metric_value_received_time)
            {
              time_t t = (time_t)data->metric_value_received_time;
              struct tm *tm = Localtime(&t);
              strftime(tbuf, CEREBRO_STAT_BUFLEN, "%F %I:%M:%S%P", tm);
            }
          else
            snprintf(tbuf, CEREBRO_STAT_BUFLEN, CEREBRO_STAT_UNKNOWN_STRING);

          fprintf(stdout, "%s(%s): %s\n", data->nodename, tbuf, vbuf);
        }
      else
        fprintf(stdout, "%s: %s\n", data->nodename, vbuf);

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
  Hostlist_sort(hd->hl);
  Hostlist_ranged_string(hd->hl, CEREBRO_STAT_BUFLEN, hstr);

  fprintf(stdout, "%s: %s\n", hstr, (char *)key);
  return 1;
}

/* 
 * _hostrange_output
 *
 * Output metric data in hostrange format.  The algorithm involves
 * using the metric_value as a hash key.  Each hash item will then
 * store the hosts with the same metric_value/key.
 */
static void
_hostrange_output(List l)
{
#if CEREBRO_DEBUG
  const char *func = __FUNCTION__;
#endif /* CEREBRO_DEBUG */
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

      _metric_value_str(data->metric_value_type,
                        data->metric_value_len,
                        data->metric_value, 
                        buf, 
                        CEREBRO_STAT_BUFLEN);

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
 * _event_data
 *
 * Monitor event data
 */
static void
_event_data(void)
{
  const char *func = __FUNCTION__;
  int fd;

  if ((fd = cerebro_event_register(handle, event_name)) < 0)
    {
      char *msg = cerebro_strerror(cerebro_errnum(handle));

      _clean_err_exit(cerebro_errnum(handle));
      err_exit("%s: cerebro_event_register: %s", func, msg);
    }
  
  while (1)
    {
      struct pollfd pfd;
      int n;

      pfd.fd = fd;
      pfd.events = POLLIN;
      pfd.revents = 0;
  
      n = Poll(&pfd, 1, -1);

      if (n && pfd.revents & POLLIN)
        {
          char vbuf[CEREBRO_STAT_BUFLEN];
          char tbuf[CEREBRO_STAT_BUFLEN];
          char *nodename;
          unsigned int event_value_type;
          unsigned int event_value_len;
          void *event_value;
          time_t t;
          struct tm *tm;

          if (cerebro_event_parse(handle,
                                  fd,
                                  &nodename,
                                  &event_value_type,
                                  &event_value_len,
                                  &event_value) < 0)
            {
              char *msg = cerebro_strerror(cerebro_errnum(handle));
              
              _clean_err_exit(cerebro_errnum(handle));
              err_exit("%s: cerebro_event_parse: %s", func, msg);
            }

          
          memset(vbuf, '\0', CEREBRO_STAT_BUFLEN);
          _metric_value_str(event_value_type,
                            event_value_len,
                            event_value,
                            vbuf,
                            CEREBRO_STAT_BUFLEN);
          
          memset(tbuf, '\0', CEREBRO_STAT_BUFLEN);
          t = time(NULL);
          tm = Localtime(&t);
          strftime(tbuf, CEREBRO_STAT_BUFLEN, "%F %I:%M:%S%P", tm);
          
          fprintf(stdout, "%s(%s): %s\n", nodename, tbuf, vbuf);
          
          free(nodename);
          free(event_value);
        }     
    }

  (void)cerebro_event_unregister(handle, fd);
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

  if (!(n = cerebro_get_metric_data(handle, metric_name)))
    {
      char *msg = cerebro_strerror(cerebro_errnum(handle));

      _clean_err_exit(cerebro_errnum(handle));
      err_exit("%s: cerebro_get_metric_data: %s", func, msg);
    }
  
  if (!(nitr = cerebro_nodelist_iterator_create(n)))
    {
      char *msg = cerebro_strerror(cerebro_nodelist_errnum(n));
      err_exit("%s: cerebro_nodelist_iterator_create: %s", func, msg);
    }

  l = List_create((ListDelF)_Free);

  while (!cerebro_nodelist_iterator_at_end(nitr))
    {
      struct node_metric_data *data = NULL;

      data = Malloc(sizeof(struct node_metric_data));
      memset(data, '\0', sizeof(struct node_metric_data));

      if (cerebro_nodelist_iterator_nodename(nitr, &(data->nodename)) < 0)
        {
          char *msg = cerebro_strerror(cerebro_nodelist_iterator_errnum(nitr));
          err_exit("%s: cerebro_nodelist_iterator_nodename: %s", func, msg);
        }

      if (cerebro_nodelist_iterator_metric_value(nitr, 
                                                 &(data->metric_value_received_time),
                                                 &(data->metric_value_type),
                                                 &(data->metric_value_len),
                                                 &(data->metric_value)) < 0)
        {
          char *msg = cerebro_strerror(cerebro_nodelist_iterator_errnum(nitr));
          err_exit("%s: cerebro_nodelist_iterator_metric_value: %s", func, msg);
        }

      List_append(l, data);
      
      if (cerebro_nodelist_iterator_next(nitr) < 0)
        {
          char *msg = cerebro_strerror(cerebro_nodelist_iterator_errnum(nitr));
          err_exit("%s: cerebro_nodelist_iterator_next: %s", func, msg);
        }
    }

  if (!strcmp(metric_name, CEREBRO_METRIC_CLUSTER_NODES))
    _cluster_nodes_output(l);
  else if (output_type == CEREBRO_STAT_NEWLINE)
    _newline_output(l);
  else if (output_type == CEREBRO_STAT_HOSTRANGE)
    _hostrange_output(l);

  /* List_destroy() and cerebro_nodelist_destory() destroy iterators too */
  List_destroy(l);
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
  
  /* Checks in _cmdline_parse ensure only one of four below will be called */

  if (event_list_flag 
      || (event_name && !strcmp(event_name, CEREBRO_EVENT_NAMES)))
    _event_list();

  if (metric_list_flag 
      || (metric_name && !strcmp(metric_name, CEREBRO_METRIC_METRIC_NAMES)))
    _metric_list();

  if (metric_name)
    _metric_data();

  if (event_name)
    _event_data();

  _cleanup_cerebro_stat();
  exit(0);
}
