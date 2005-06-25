/*****************************************************************************\
 *  $Id: debug.h,v 1.1 2005-06-25 00:14:28 achu Exp $
\*****************************************************************************/

#ifndef _DEBUG_H
#define _DEBUG_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if CEREBRO_DEBUG

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <errno.h>

#include "cerebro/cerebro_error.h"

#define ERROR_BUFFER_LEN 8192

#define CEREBRO_MSG_CREATE(msg) \
    char errbuf[ERROR_BUFFER_LEN]; \
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
        char *msgbuf; \
        if ((msgbuf = error_msg_create msg)) \
          { \
            strncat(errbuf, msgbuf, ERROR_BUFFER_LEN - len - 1); \
            len += strlen(msgbuf); \
            free(msgbuf); \
          } \
      }

#define CEREBRO_ERR_DEBUG(msg) \
    do { \
      CEREBRO_MSG_CREATE(msg) \
      cerebro_err_debug(errbuf); \
    } while(0)

#define CEREBRO_ERR_DEBUG_ERRNO(msg) \
    do { \
      CEREBRO_MSG_CREATE(msg) \
      if (len < ERROR_BUFFER_LEN) \
        { \
          char errnobuf[ERROR_BUFFER_LEN]; \
          snprintf(errnobuf, ERROR_BUFFER_LEN, ": %s", strerror(errno)); \
          strncat(errbuf, errnobuf, ERROR_BUFFER_LEN - len -1); \
          cerebro_err_debug(errbuf); \
        } \
    } while(0)

/*
 * debug_msg_create
 *
 * create a buffer and put the a mesage inside it
 *
 * Returns message buffer or NULL on error
 */
char *debug_msg_create(const char *fmt, ...);
    
#else /* !CEREBRO_DEBUG */

#define CEREBRO_ERR_DEBUG(msg)

#define CEREBRO_ERR_DEBUG_ERRNO(msg)

#endif /* !CEREBRO_DEBUG */

#endif /* _DEBUG_H */
