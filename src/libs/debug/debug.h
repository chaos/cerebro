/*****************************************************************************\
 *  $Id: debug.h,v 1.12 2010-02-02 01:01:20 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2011 Lawrence Livermore National Security, LLC.
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
 *  with Cerebro. If not, see <http://www.gnu.org/licenses/>.
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

#define CEREBRO_ERR(msg) \
    do { \
      CEREBRO_MSG_CREATE(msg) \
      cerebro_err_output(errbuf); \
    } while(0)

#define CEREBRO_EXIT(msg) \
    do { \
      CEREBRO_MSG_CREATE(msg) \
      cerebro_err_exit(errbuf); \
    } while(0)
   
#else /* !CEREBRO_DEBUG */

#define CEREBRO_DBG(msg)

#define CEREBRO_ERR(msg) \
    do { \
      cerebro_err_output msg; \
    } while(0)

#define CEREBRO_EXIT(msg) \
    do { \
      cerebro_err_exit msg; \
    } while(0)

#endif /* !CEREBRO_DEBUG */

#endif /* _DEBUG_H */
