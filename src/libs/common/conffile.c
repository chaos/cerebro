/*****************************************************************************\
 *  $Id: conffile.c,v 1.4 2010-02-02 01:01:20 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2010 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2003-2007 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>
 *  UCRL-CODE-155699
 *
 *  This file is part of Whatsup, tools and libraries for determining up and
 *  down nodes in a cluster. For details, see http://www.llnl.gov/linux/.
 *
 *  Whatsup is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  Whatsup is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with Whatsup.  If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <ctype.h>
#include <string.h>
#endif /* STDC_HEADERS */
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */
#include <sys/types.h>
#include <sys/stat.h>

#include "conffile.h"
#include "fd.h"

#define CONFFILE_MAGIC           0x0a1b2c3d

/* struct conffile
 * 
 * structure for conffile handle.  
 * - magic - magic number
 * - errnum - error code
 * - options - options array passed in conffile_parse()
 * - options_len - options length passed in conffile_parse()
 * - app_ptr - app_ptr passed in conffile_parse()
 * - app_data - app_data passed in conffile_parse()
 * - flags - flags passed in conffile_parse()
 * - line_num - line number, does not count continuation lines
 * - line_count - line number, counting continuation lines
 * - end_of_file - flag indicating if we've read EOF
 * - optionname - the current option name read from the conf file
 */
struct conffile {
    int magic;
    int errnum;
    int fd;
    struct conffile_option *options;
    int options_len;
    void *app_ptr;
    int app_data;
    int flags;
    int line_num;
    int line_count;
    int end_of_file;
    char optionname[CONFFILE_MAX_OPTIONNAMELEN];
};

static char *_errmsg[] = {
    "success",
    "unknown configuration option \"%s\" on line %d",
    "configuration option \"%s\" listed too many times",
    "configuration option \"%s\" listed too few times",
    "overflow line length on line %d",
    "overflow option name length on line %d",
    "overflow arg length on line %d",
    "missing argument for option \"%s\" on line %d",
    "too many arguments listed for option \"%s\" on line %d",
    "invalid argument listed for option \"%s\" on line %d",
    "quotation marks used improperly on line %d",
    "continuation character '\\' used improperly on line %d",
    "callback function generated error",
    "configuration file does not exist",
    "configuration file cannot be opened",
    "configuration file read error",
    "out of memory",
    "invalid parameters",
    "incorrect magic number",
    "null handle", 
    "internal error",
    "error number out of range"
};

conffile_t
conffile_handle_create(void) 
{
    conffile_t cf;

    if ((cf = (conffile_t)malloc(sizeof(struct conffile))) == NULL)
        return NULL;

    cf->magic = CONFFILE_MAGIC;
    cf->errnum = CONFFILE_ERR_SUCCESS;
    cf->fd = -1;

    return cf;
}

int
conffile_handle_destroy(conffile_t cf) 
{
    if (cf == NULL || cf->magic != CONFFILE_MAGIC)
        return -1;

    cf->magic = ~CONFFILE_MAGIC;
    free(cf);
    return 0;
}

int
conffile_errnum(conffile_t cf)
{
    if (cf == NULL)
        return CONFFILE_ERR_NULLHANDLE;
    else if (cf->magic != CONFFILE_MAGIC)
        return CONFFILE_ERR_MAGIC;
    else
        return cf->errnum;
}

int
conffile_errmsg(conffile_t cf, char *buf, int buflen)
{
    int rv;
    char errbuf[CONFFILE_MAX_ERRMSGLEN];

    if (cf == NULL) 
        rv = snprintf(errbuf, CONFFILE_MAX_ERRMSGLEN, 
                      _errmsg[CONFFILE_ERR_NULLHANDLE]);
    else if (cf->magic != CONFFILE_MAGIC)
        rv = snprintf(errbuf, CONFFILE_MAX_ERRMSGLEN, 
                      _errmsg[CONFFILE_ERR_MAGIC]);
    else if (cf->errnum < CONFFILE_ERR_SUCCESS 
             || cf->errnum > CONFFILE_ERR_ERRNUMRANGE)
        rv = snprintf(errbuf, CONFFILE_MAX_ERRMSGLEN, 
                      _errmsg[CONFFILE_ERR_ERRNUMRANGE]);
    else if (cf->errnum == CONFFILE_ERR_PARSE_OPTION_UNKNOWN
             || cf->errnum == CONFFILE_ERR_PARSE_ARG_MISSING
             || cf->errnum == CONFFILE_ERR_PARSE_ARG_TOOMANY
             || cf->errnum == CONFFILE_ERR_PARSE_ARG_INVALID)
        rv = snprintf(errbuf, CONFFILE_MAX_ERRMSGLEN, 
                      _errmsg[cf->errnum], cf->optionname, cf->line_num);
    else if (cf->errnum == CONFFILE_ERR_PARSE_OPTION_TOOMANY
             || cf->errnum == CONFFILE_ERR_PARSE_OPTION_TOOFEW)
        rv = snprintf(errbuf, CONFFILE_MAX_ERRMSGLEN, 
                      _errmsg[cf->errnum], cf->optionname);
    else if (cf->errnum == CONFFILE_ERR_PARSE_OVERFLOW_LINELEN
             || cf->errnum == CONFFILE_ERR_PARSE_OVERFLOW_OPTIONLEN
             || cf->errnum == CONFFILE_ERR_PARSE_OVERFLOW_ARGLEN
             || cf->errnum == CONFFILE_ERR_PARSE_QUOTE
             || cf->errnum == CONFFILE_ERR_PARSE_CONTINUATION)
        rv = snprintf(errbuf, CONFFILE_MAX_ERRMSGLEN, 
                      _errmsg[cf->errnum], cf->line_num);
    else
        rv = snprintf(errbuf, CONFFILE_MAX_ERRMSGLEN, 
                      _errmsg[cf->errnum]);

    if (rv >= buflen)
        return -1;
             
    strcpy(buf, errbuf);
    return 0;
}

int
conffile_seterrnum(conffile_t cf, int errnum)
{
    if (cf == NULL || cf->magic != CONFFILE_MAGIC)
        return -1;

    if (errnum < CONFFILE_ERR_SUCCESS || errnum > CONFFILE_ERR_ERRNUMRANGE) {
        cf->errnum = CONFFILE_ERR_PARAMETERS;
        return -1;
    }

    cf->errnum = errnum;
    return 0;
}

int
conffile_line_number(conffile_t cf)
{
    if (cf == NULL || cf->magic != CONFFILE_MAGIC)
        return -1;

    return cf->line_num;
} 

static int
_setup(conffile_t cf,
       const char *filename,
       struct conffile_option *options,
       int options_len,
       void *app_ptr,
       int app_data,
       int flags)
{
    int i;
    
    /* If it doesn't exist for real or can't be read, we consider it
     * non-existant 
     */
    if (access(filename, R_OK) < 0) {
        cf->errnum = CONFFILE_ERR_EXIST;
        return -1;
    }

    if ((cf->fd = open(filename, O_RDONLY)) < 0) {
        cf->errnum = CONFFILE_ERR_OPEN;
        return -1;
    }

    cf->options = options;
    cf->options_len = options_len;
    for (i = 0; i < options_len; i++) {
        if (cf->options[i].count_ptr)
            *cf->options[i].count_ptr = 0;
    }
    cf->app_ptr = app_ptr;
    cf->app_data = app_data;
    cf->flags = flags;
    cf->line_num = 0;
    cf->line_count = 0;
    cf->end_of_file = 0;
    memset(cf->optionname, '\0', CONFFILE_MAX_OPTIONNAMELEN);

    cf->errnum = CONFFILE_ERR_SUCCESS;
    return 0;
}

static int
_remove_trailing_whitespace(conffile_t cf, char *linebuf, int linebuflen) 
{
    char *temp;

    temp = linebuf + linebuflen;
    for (--temp; temp >= linebuf; temp--) {
        if (isspace(*temp))
            *temp = '\0';
        else
            break;
        linebuflen--;
    }

    return linebuflen;
}

static int
_remove_comments(conffile_t cf, char *linebuf, int linebuflen)
{
    int i, newlen, previous_is_escape = 0, comment_remaining_buf = 0;
  
    /* Cannot do check like the following:
     *
     * if (linebuf[i] == '#' && linebuf[i-1] != '\\')
     *
     * Would fail on parse situation such as the following:
     *
     * \\# 
     *
     */
    i = 0;
    newlen = linebuflen;
    while (i < linebuflen) {
        if (comment_remaining_buf) {
            linebuf[i] = '\0';
            newlen--;
        }
        else if (linebuf[i] == '\\' && previous_is_escape == 0)
            previous_is_escape++;
        else if (linebuf[i] == '#' && previous_is_escape == 0) {
            linebuf[i] = '\0';
            comment_remaining_buf++;
            newlen--;
        }
        else if (previous_is_escape > 0)
            previous_is_escape = 0;
        i++;
    }
    
    return newlen;
} 

static int
_readline(conffile_t cf, char *linebuf, int linebuflen)
{
    int ret, len = 0;
    int continuation = 0;
    char buf[CONFFILE_MAX_LINELEN];
    
    if (linebuflen < CONFFILE_MAX_LINELEN) {
        cf->errnum = CONFFILE_ERR_INTERNAL;
        return -1;
    }

    cf->line_num = cf->line_count + 1;
    while (1) {
        ret = fd_read_line(cf->fd, buf, CONFFILE_MAX_LINELEN);
        if (ret < 0) {
            cf->errnum = CONFFILE_ERR_READ;
            return -1;
        }

        if (ret == 0) {
            /* Ok to break here. All continuation characters and
             * comments taken care of earlier
             */
            cf->end_of_file++; 
            break;
        }

        /* MAX_LINELEN - 1 b/c fd_read_line guarantees null termination */
        if (ret >= (CONFFILE_MAX_LINELEN-1)) {
            cf->errnum = CONFFILE_ERR_PARSE_OVERFLOW_LINELEN;
            return -1;
        }

        if ((ret + len) >= linebuflen) {
            cf->errnum = CONFFILE_ERR_PARSE_OVERFLOW_LINELEN;
            return -1;
        }

        cf->line_count++;
        memcpy(linebuf + len, buf, ret);
        len += ret;

        len = _remove_trailing_whitespace(cf, linebuf, len);
        if (len == 0) {
            cf->line_num = cf->line_count + 1;
            continue;
        }

        if ((len = _remove_comments(cf, linebuf, len)) < 0)
            return -1;

        if (len == 0) {
            cf->line_num = cf->line_count + 1;
            continue;
        }

        /* Need to call a second time, for following case
         *
         * optionname arg1 \    # my comment
         *
         *                  ^^^^ need to remove this whitespace 
         */
        len = _remove_trailing_whitespace(cf, linebuf, len);
        if (len == 0) {
            cf->line_num = cf->line_count + 1;
            continue;
        }

        if (linebuf[len-1] == '\\') {
            continuation++;
            linebuf[len-1] = '\0'; 
            len--;
            continue;
        }
        else
            break;
    }

    return len;
}

static char *
_move_past_whitespace(conffile_t cf, char *linebuf)
{
    while (*linebuf != '\0' && isspace(*linebuf))
        linebuf++;

    if (*linebuf == '\0')
        return NULL;
    
    return linebuf;
}

int
_parse_args(conffile_t cf, 
            char *linebuf, 
            char args[CONFFILE_MAX_ARGS][CONFFILE_MAX_ARGLEN])
{
    int quote_flag, numargs = 0;

    while (1) {
        int arglen = 0;

        /* Following is needed for the following corner case
         *
         * optionname1    arg        \
         *
         * optionname2    arg
         *
         * In other words, a continuation character is used, but
         * there is no data on the next line.  There may be
         * remaining whitespace.
         */
         if ((linebuf = _move_past_whitespace(cf, linebuf)) == NULL)
            break;
        
        quote_flag = 0;
        memset(args[numargs], '\0', CONFFILE_MAX_ARGLEN);
        while (*linebuf != '\0' 
               && (quote_flag == 1 || !isspace(*linebuf))) {

            if (*linebuf == '"') {
                quote_flag = !quote_flag;
                linebuf++;
                continue;
            }

            /* All continuation character parse errors should be
             * discovered by _readline and _remove_comments.  But
             * we'll check again just in case.
             */
            if (*linebuf == '\\') {
                linebuf++;
                if (*linebuf != '\\'
                    && *linebuf != '#'
                    && *linebuf != '"') {
                    cf->errnum = CONFFILE_ERR_PARSE_CONTINUATION;
                    return -1;
                }
            }

            args[numargs][arglen] = *linebuf;
            linebuf++;
            arglen++;

            /* minus one to guarantee null termination */
            if (arglen == (CONFFILE_MAX_ARGLEN-1)) {
                cf->errnum = CONFFILE_ERR_PARSE_OVERFLOW_ARGLEN;
                return -1;
            }

        }

        if (quote_flag > 0) {
            cf->errnum = CONFFILE_ERR_PARSE_QUOTE;
            return -1;
        }

        numargs++;
                                
        if (*linebuf == '\0')
            break;
    }

    return numargs;
}

static int
_parseline(conffile_t cf, char *linebuf, int linebuflen)
{
    int i, optionlen, rv, numargs = 0;
    char args[CONFFILE_MAX_ARGS][CONFFILE_MAX_ARGLEN];
    struct conffile_option *option = NULL;
    struct conffile_data data;
    char *ptr;

    memset(&data, '\0', sizeof(struct conffile_data));

    linebuflen = _remove_trailing_whitespace(cf, linebuf, linebuflen);
    if (linebuflen == 0)
        return 0;

    if ((linebuf = _move_past_whitespace(cf, linebuf)) == NULL)
        return 0;

    optionlen = 0;
    memset(cf->optionname, '\0', CONFFILE_MAX_OPTIONNAMELEN);
    while (optionlen < (CONFFILE_MAX_OPTIONNAMELEN-1) 
           && !isspace(*linebuf)
           && *linebuf != '\0') {
        cf->optionname[optionlen++] = *linebuf;
        linebuf++;
    }

    /* minus one to guarantee null termination */
    if (optionlen == (CONFFILE_MAX_OPTIONNAMELEN-1)) {
        cf->errnum = CONFFILE_ERR_PARSE_OVERFLOW_OPTIONLEN;
        return -1;
    }

    for (i = 0; i < cf->options_len; i++) {
        int rv;
        if (cf->flags & CONFFILE_FLAG_OPTION_CASESENSITIVE) 
            rv = strcmp(cf->options[i].optionname, cf->optionname);
        else
            rv = strcasecmp(cf->options[i].optionname, cf->optionname);

        if (rv == 0) {
            option = &(cf->options[i]);
            break;
        }
    }

    if (option == NULL) {
        if (cf->flags & CONFFILE_FLAG_OPTION_IGNORE_UNKNOWN)
	    return 0;
        cf->errnum = CONFFILE_ERR_PARSE_OPTION_UNKNOWN;
        return -1;
    }

    if (option->option_type == CONFFILE_OPTION_IGNORE)
        return 0;

    (*option->count_ptr)++;
    if ((*option->count_ptr) > option->max_count) {
        cf->errnum = CONFFILE_ERR_PARSE_OPTION_TOOMANY;
        return -1;
    }

    if ((linebuf = _move_past_whitespace(cf, linebuf)) != NULL) {
        if ((numargs = _parse_args(cf, linebuf, args)) < 0)
            return -1;
    }

    /* Argument checks */

    if (option->option_type == CONFFILE_OPTION_FLAG && numargs != 0) {
        cf->errnum = CONFFILE_ERR_PARSE_ARG_TOOMANY;
        return -1;
    }

    if (((option->option_type == CONFFILE_OPTION_BOOL
          || option->option_type == CONFFILE_OPTION_INT
          || option->option_type == CONFFILE_OPTION_DOUBLE
          || option->option_type == CONFFILE_OPTION_STRING
          || option->option_type == CONFFILE_OPTION_LIST_INT
          || option->option_type == CONFFILE_OPTION_LIST_DOUBLE
          || option->option_type == CONFFILE_OPTION_LIST_STRING)
         && numargs == 0)) {
        cf->errnum = CONFFILE_ERR_PARSE_ARG_MISSING;
        return -1;
    }

    if (((option->option_type == CONFFILE_OPTION_BOOL
          || option->option_type == CONFFILE_OPTION_INT
          || option->option_type == CONFFILE_OPTION_DOUBLE
          || option->option_type == CONFFILE_OPTION_STRING)
         && numargs > 1)) {
        cf->errnum = CONFFILE_ERR_PARSE_ARG_TOOMANY;
        return -1;
    }

    if ((option->option_type == CONFFILE_OPTION_LIST_INT
         || option->option_type == CONFFILE_OPTION_LIST_DOUBLE
         || option->option_type == CONFFILE_OPTION_LIST_STRING)
        && ((int)option->option_type_arg) > 0
        && numargs != ((int)option->option_type_arg)) {
        if (numargs < ((int)option->option_type_arg))
            cf->errnum = CONFFILE_ERR_PARSE_ARG_MISSING;
        else
            cf->errnum = CONFFILE_ERR_PARSE_ARG_TOOMANY;
        return -1;
    }

    if (option->option_type == CONFFILE_OPTION_BOOL) {
        if (strcmp(args[0], "1") != 0
            && strcmp(args[0], "0") != 0
            && strcasecmp(args[0], "y") != 0
            && strcasecmp(args[0], "n") != 0
            && strcasecmp(args[0], "yes") != 0
            && strcasecmp(args[0], "no") != 0
            && strcasecmp(args[0], "on") != 0
            && strcasecmp(args[0], "off") != 0
            && strcasecmp(args[0], "t") != 0
            && strcasecmp(args[0], "f") != 0
            && strcasecmp(args[0], "true") != 0
            && strcasecmp(args[0], "false") != 0
            && strcasecmp(args[0], "enable") != 0
            && strcasecmp(args[0], "disable") != 0) {
            cf->errnum = CONFFILE_ERR_PARSE_ARG_INVALID;
            return -1;
        }
    }

    /* Calculate Data */

    if (option->option_type == CONFFILE_OPTION_BOOL) {
        if (!strcmp(args[0], "1")
            || !strcasecmp(args[0], "y")
            || !strcasecmp(args[0], "yes")
            || !strcasecmp(args[0], "on")
            || !strcasecmp(args[0], "t")
            || !strcasecmp(args[0], "true")
            || !strcasecmp(args[0], "enable"))
            data.boolval = 1;
        else
            data.boolval = 0;
    }
    else if (option->option_type == CONFFILE_OPTION_INT) {
        data.intval = strtol(args[0], &ptr, 0);
        if ((args[0] + strlen(args[0])) != ptr) {
            cf->errnum = CONFFILE_ERR_PARSE_ARG_INVALID;
            return -1;
        }
    }
    else if (option->option_type == CONFFILE_OPTION_DOUBLE) {
        data.doubleval = strtod(args[0], &ptr);
        if ((args[0] + strlen(args[0])) != ptr) {
            cf->errnum = CONFFILE_ERR_PARSE_ARG_INVALID;
            return -1;
        }
    }
    else if (option->option_type == CONFFILE_OPTION_STRING) {
        strncpy(data.string, args[0], CONFFILE_MAX_ARGLEN);
        data.string[CONFFILE_MAX_ARGLEN - 1] = '\0';
    }
    else if (option->option_type == CONFFILE_OPTION_LIST_INT) {
        int i;
        for (i = 0; i < numargs; i++) {
            data.intlist[i] = strtol(args[i], &ptr, 0);
            if ((args[i] + strlen(args[i])) != ptr) {
                cf->errnum = CONFFILE_ERR_PARSE_ARG_INVALID;
                return -1;
            }
        }
        data.intlist_len = numargs;
    }
    else if (option->option_type == CONFFILE_OPTION_LIST_DOUBLE) {
        int i;
        for (i = 0; i < numargs; i++) {
            data.doublelist[i] = strtod(args[i], &ptr);
            if ((args[i] + strlen(args[i])) != ptr) {
                cf->errnum = CONFFILE_ERR_PARSE_ARG_INVALID;
                return -1;
            }
        }
        data.doublelist_len = numargs;
    }
    else if (option->option_type == CONFFILE_OPTION_LIST_STRING) {
        int i;
        for (i = 0; i < numargs; i++) {
            strncpy(data.stringlist[i], args[i], CONFFILE_MAX_ARGLEN);
            data.stringlist[i][CONFFILE_MAX_ARGLEN - 1] = '\0';
        }
        data.stringlist_len = numargs;
    }

    cf->errnum = CONFFILE_ERR_SUCCESS;
    if (option->callback_func) {
        rv = (option->callback_func)(cf,
                                     &data,
                                     option->optionname,
                                     option->option_type,
                                     option->option_ptr,
                                     option->option_data,
                                     cf->app_ptr,
                                     cf->app_data);
        if (rv < 0) {
            if (cf->errnum == CONFFILE_ERR_SUCCESS)
                cf->errnum = CONFFILE_ERR_PARSE_CALLBACK;
            return -1;
        }
    }

    return 0;
}

int
conffile_parse(conffile_t cf,
               const char *filename,
               struct conffile_option *options,
               int options_len,
               void *app_ptr,
               int app_data,
               int flags)
{
    int i, temp, len = 0, retval = -1;
    char linebuf[CONFFILE_MAX_LINELEN];

    if (cf == NULL || cf->magic != CONFFILE_MAGIC)
        return -1;

    if (options == NULL || options_len <= 0) {
        cf->errnum = CONFFILE_ERR_PARAMETERS;
        return -1;
    }

    /* Ensure option array is legitimate */ 
    for (i = 0; i < options_len; i++) {
        if (options[i].optionname == NULL
            || strlen(options[i].optionname) >= CONFFILE_MAX_OPTIONNAMELEN
            || options[i].option_type < CONFFILE_OPTION_IGNORE
            || options[i].option_type > CONFFILE_OPTION_LIST_STRING
            || ((options[i].option_type != CONFFILE_OPTION_IGNORE
                 && options[i].option_type != CONFFILE_OPTION_FLAG)
                && options[i].callback_func == NULL)
            || ((options[i].option_type == CONFFILE_OPTION_LIST_INT
                 || options[i].option_type == CONFFILE_OPTION_LIST_DOUBLE
                 || options[i].option_type == CONFFILE_OPTION_LIST_STRING)
                && ((int)options[i].option_type_arg) == 0)
            || options[i].max_count < 0
            || options[i].required_count < 0
            || (options[i].option_type != CONFFILE_OPTION_IGNORE
                && options[i].count_ptr == NULL)) {
            cf->errnum = CONFFILE_ERR_PARAMETERS;
            return -1;
        }
    }

    /* Ensure flags are appropriate */
    temp = flags;
    temp &= ~CONFFILE_FLAG_OPTION_CASESENSITIVE;
    temp &= ~CONFFILE_FLAG_OPTION_IGNORE_UNKNOWN;
    if (temp) {
        cf->errnum = CONFFILE_ERR_PARAMETERS;
        return -1;
    }

    if (_setup(cf, filename, options, options_len, app_ptr, app_data, flags) < 0)
        goto cleanup;

    while (cf->end_of_file == 0 && 
           (len = _readline(cf, linebuf, CONFFILE_MAX_LINELEN)) > 0) {

        if (_parseline(cf, linebuf, len) < 0)
            goto cleanup;
    }
  
    if (len < 0)
        goto cleanup;

    cf->line_num = 0;
    /* Check required counts */
    for (i = 0; i < cf->options_len; i++) {
        if (cf->options[i].count_ptr == NULL)
            continue;
        if ((*cf->options[i].count_ptr) < cf->options[i].required_count) {
            strcpy(cf->optionname, cf->options[i].optionname);
            cf->errnum = CONFFILE_ERR_PARSE_OPTION_TOOFEW;
            goto cleanup;
        }
    }

    cf->errnum = CONFFILE_ERR_SUCCESS;
    retval = 0;

 cleanup:
    /* ignore potential error, just return result to user */
    close(cf->fd);
    return retval;
}

     
CONFFILE_OPTION_FUNC(conffile_empty)
{
    return 0;
}

CONFFILE_OPTION_FUNC(conffile_bool)
{
    if (option_ptr == NULL) {
        conffile_seterrnum(cf, CONFFILE_ERR_PARAMETERS);
        return -1;
    }

    *((int *)option_ptr) = data->boolval;
    return 0;
}

CONFFILE_OPTION_FUNC(conffile_int)
{
    if (option_ptr == NULL) {
        conffile_seterrnum(cf, CONFFILE_ERR_PARAMETERS);
        return -1;
    }

    *((int *)option_ptr) = data->intval;
    return 0;
}

CONFFILE_OPTION_FUNC(conffile_double)
{
    if (option_ptr == NULL) {
        conffile_seterrnum(cf, CONFFILE_ERR_PARAMETERS);
        return -1;
    }

    *((double *)option_ptr) = data->doubleval;
    return 0;
}

CONFFILE_OPTION_FUNC(conffile_string)
{
    if (option_ptr == NULL || option_data <= 0) {
        conffile_seterrnum(cf, CONFFILE_ERR_PARAMETERS);
        return -1;
    }

    strncpy((char *)option_ptr, data->string, option_data);
    ((char *)option_ptr)[option_data - 1] = '\0';
    return 0;
}

