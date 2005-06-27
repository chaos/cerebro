/*****************************************************************************\
 *  $Id: debug.h,v 1.5 2005-06-27 17:24:09 achu Exp $
\*****************************************************************************/

#ifndef _DEBUG_H
#define _DEBUG_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */

#include "cerebro/cerebro_error.h"

#define DEBUG_BUFFER_LEN 8192

#define CEREBRO_MSG_CREATE(msg) \
    char errbuf[DEBUG_BUFFER_LEN]; \
    int len; \
    \
    memset(errbuf, '\0', DEBUG_BUFFER_LEN); \
    \
    len = snprintf(errbuf, \
                   DEBUG_BUFFER_LEN, \
                   "(%s, %s, %d): ", \
                   __FILE__, \
                   __FUNCTION__, \
                   __LINE__); \
    \
    if (len < DEBUG_BUFFER_LEN) \
      { \
        char *msgbuf; \
        if ((msgbuf = debug_msg_create msg)) \
          { \
            strncat(errbuf, msgbuf, DEBUG_BUFFER_LEN - len - 1); \
            len += strlen(msgbuf); \
            free(msgbuf); \
          } \
      }

/*
 * debug_msg_create
 *
 * create a buffer and put the a mesage inside it
 *
 * Returns message buffer or NULL on error
 */
char *debug_msg_create(const char *fmt, ...);

#if CEREBRO_DEBUG

#define CEREBRO_DBG(msg) \
    do { \
      CEREBRO_MSG_CREATE(msg) \
      cerebro_err_debug(errbuf); \
    } while(0)

#define CEREBRO_EXIT(msg) \
    do { \
      CEREBRO_MSG_CREATE(msg) \
      cerebro_err_exit(errbuf); \
    } while(0)
   
#else /* !CEREBRO_DEBUG */

#define CEREBRO_DBG(msg)

#define CEREBRO_EXIT(msg) \
    do { \
      cerebro_err_exit msg; \
    } while(0)

#endif /* !CEREBRO_DEBUG */

#endif /* _DEBUG_H */
