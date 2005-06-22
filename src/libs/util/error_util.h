/*****************************************************************************\
 *  $Id: error_util.h,v 1.1 2005-06-22 18:11:00 achu Exp $
\*****************************************************************************/

#ifndef _ERROR_UTIL_H
#define _ERROR_UTIL_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */

#include "cerebro/cerebro_error.h"

#define ERROR_BUFFER_LEN 8192

#define CEREBRO_MSG_CREATE(msg) \
    char errbuf[ERROR_BUFFER_LEN]; \
    char *buf; \
    int len; \
    \
    memset(errbuf, '\0', ERROR_BUFFER_LEN); \
    \
    len = snprintf(errbuf, \
                   ERROR_BUFFER_LEN, \
                   "(%s, %s, %d): ", \
                   __FILE__, \
                   __FUNCTION__, \
                   __LINE__); \
    \
    if (len < ERROR_BUFFER_LEN) \
      { \
        if ((buf = error_msg_create msg)) \
          { \
            strncat(errbuf, buf, ERROR_BUFFER_LEN - len - 1); \
            free(buf); \
          } \
      }

#define CEREBRO_ERR_DEBUG(msg) \
  do { \
    CEREBRO_MSG_CREATE \
    cerebro_err_debug(errbuf); \
  } while(0)

#define CEREBRO_ERR_EXIT(msg)
  do { \
    CEREBRO_MSG_CREATE \
    cerebro_err_exit(errbuf); \
  } while(0)

/* 
 * error_msg_create
 *
 * create a buffer and put the a mesage inside it
 *
 * Returns message buffer or NULL on error
 */
char *error_msg_create(char *fmt, ...);

#endif /* _ERROR_UTIL_H */
