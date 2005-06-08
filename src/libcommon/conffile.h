/*****************************************************************************\
 *  $Id: conffile.h,v 1.4 2005-06-08 00:10:49 achu Exp $
 *****************************************************************************
 *  Copyright (C) 2003 The Regents of the University of California.
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
 *  with Whatsup; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
\*****************************************************************************/

#ifndef _CONFFILE_H
#define _CONFFILE_H 1

/*
 * This is a generic configuration file parsing library.  Some library
 * ideas were from libdotconf by Lukas Schroder <lukas@azzit.de>.
 * http://www.azzit.de/dotconf/.
 * 
 * This library parses configuration files in the form:
 *
 * optionname arg1 arg2 arg3 ...
 *
 * Different option names are listed on different lines.  They are
 * separated from their arguments by whitespace.  Each argument is
 * also separated by whitespace.  Options may have a specified or
 * varying number of arguments, depending on the option's type
 * specification listed below under "OPTION TYPES"
 *
 * Comments can be listed by the '#' sign.  The '#' and any characters
 * to the right of it are ignored.  Lines can be continued on the next
 * line if a '\' character is the last non-whitespace character on a
 * line.  Strings can be quoted using double quotation marks '"'.  The
 * '#', '"', and '\' characters can be escaped using the '\'
 * character.
 *
 * The '#' character takes precedence over the '"' and '\' character in
 * parsing.  For example, the following would be parse errors:
 *
 * optionname1   "#"
 * optionname2   arg1 # my comment \
 *               arg2
 *
 * The proper way to use the # and \ character are as follows.
 *
 * optionname1   "\#"
 * optionname2   arg1 \ # my comment
 *               arg2
 *
 * A continuation character at the end of a line takes precendence
 * over escape characters.  For example, the following results in a
 * parse error:
 *
 * optioname     arg1 arg2 \\
 *               arg3 arg4
 * 
 * It is necessary to use an additional escape character, as follows
 *
 * optionname    arg1 arg2 \\\
 *               arg3 arg4
 *
 * The writer of the configuration file is still responsible for white
 * space between arguments when continuation characters are used.  For
 * example:
 *
 * optionname    arg1 arg2\
 * arg3
 *
 * This example has 2 arguments, "arg1" and "arg2arg3".  
 *
 * When a parse error with quotes occurs, a PARSE_QUOTE error code is
 * returned.  When a parse error occurs with a '\' character, a
 * PARSE_CONTINUATION error code is returned.  A number of other error
 * codes can be returned.  See ERROR CODES for details.
 *
 */

/* OPTION TYPES
 * 
 * The following are option types an option may take.
 * 
 * IGNORE - up to MAX_ARGS arguments, will not call a callback function
 *        - useful for deprecating old configuration options
 * FLAG - no arguments, returns no arguments
 * BOOL - 1 argument, returns 1 or 0 
 *      - true strings - "1", "y", "yes", "on", "t", "true", "enable"
 *      - false strings- "0", "n", "no" "off", "f", "false", "disable"
 * INT - 1 argument, returns an integer
 * DOUBLE - 1 argument, returns a double
 * STRING - 1 argument, returns a string
 * LIST_INT - up to MAX_ARGS integer args
 * LIST_DOUBLE - up to MAX_ARGS double args
 * LIST_STRING - up to MAX_ARGS string args, each string up to 
 *               MAX_ARGLEN in length
 *  
 * For LIST_INT, LIST_DOUBLE, and LIST_STRING, option_type_arg in
 * struct conffile_option can be set to the length of the list
 * desired, or < 0 for a variable length list.  For all other option
 * types option_type_arg will be ignored.
 *
 * Callback functions are required for all of the above option types
 * except for IGNORE and FLAG
 *
 * If an argument is missing a PARSE_NO_ARG error is returned.  If the
 * incorrect number of arguments is listed, PARSE_NUM_ARGS is
 * returned.  If an invalid argument is listed, PARSE_INVALID_ARG is
 * returned.
 *   
 */
#define CONFFILE_OPTION_IGNORE                 0x00
#define CONFFILE_OPTION_FLAG                   0x01
#define CONFFILE_OPTION_BOOL                   0x02
#define CONFFILE_OPTION_INT                    0x03
#define CONFFILE_OPTION_DOUBLE                 0x04
#define CONFFILE_OPTION_STRING                 0x05
#define CONFFILE_OPTION_LIST_INT               0x06
#define CONFFILE_OPTION_LIST_DOUBLE            0x07
#define CONFFILE_OPTION_LIST_STRING            0x08

/* LENGTHS
 *
 * The following are the maximum values and lengths throughout
 * the conffile parser.
 */

#define CONFFILE_MAX_LINELEN                  32778
#define CONFFILE_MAX_OPTIONNAMELEN              256
#define CONFFILE_MAX_ARGS                        64
#define CONFFILE_MAX_ARGLEN                     512
#define CONFFILE_MAX_ERRMSGLEN                 1024

/* ERROR CODES
 * 
 * The following are the error codes that may be returned to the user.
 * The error codes and strings describing the error codes can be
 * accessed through conffile_errnum(), conffile_errmsg(), and
 * conffile_errmsg().  The error code can be set using
 * conffile_seterrnum().
 */

#define CONFFILE_ERR_SUCCESS                   0x00
#define CONFFILE_ERR_PARSE_OPTION_UNKNOWN      0x01
#define CONFFILE_ERR_PARSE_OPTION_TOOMANY      0x02
#define CONFFILE_ERR_PARSE_OPTION_TOOFEW       0x03
#define CONFFILE_ERR_PARSE_OVERFLOW_LINELEN    0x04
#define CONFFILE_ERR_PARSE_OVERFLOW_OPTIONLEN  0x05
#define CONFFILE_ERR_PARSE_OVERFLOW_ARGLEN     0x06
#define CONFFILE_ERR_PARSE_ARG_MISSING         0x07
#define CONFFILE_ERR_PARSE_ARG_TOOMANY         0x08
#define CONFFILE_ERR_PARSE_ARG_INVALID         0x09
#define CONFFILE_ERR_PARSE_QUOTE               0x0a
#define CONFFILE_ERR_PARSE_CONTINUATION        0x0b
#define CONFFILE_ERR_PARSE_CALLBACK            0x0c
#define CONFFILE_ERR_EXIST                     0x0d
#define CONFFILE_ERR_OPEN                      0x0e
#define CONFFILE_ERR_READ                      0x0f
#define CONFFILE_ERR_OUTMEM                    0x10
#define CONFFILE_ERR_PARAMETERS                0x11
#define CONFFILE_ERR_MAGIC                     0x12
#define CONFFILE_ERR_NULLHANDLE                0x13
#define CONFFILE_ERR_INTERNAL                  0x14
#define CONFFILE_ERR_ERRNUMRANGE               0x15

#define CONFFILE_IS_PARSE_ERR(x)  ((x) >= CONFFILE_ERR_PARSE_OPTION_UNKNOWN \
                                   && (x) <= CONFFILE_ERR_PARSE_CALLBACK)

/* FLAGS
 * 
 * The following flags can be passed to conffile_parse() to alter
 * behavior of conffile parsing.
 *
 * OPTION_CASESENSITIVE - By default option names are case
 *                        insensitive.  This flag informs the parser
 *                        to make option names case sensitive
 * 
 * OPTION_IGNORE_UNKNOWN - Instead of returning a
 *                         CONFFILE_ERR_PARSE_OPTION_UNKNOWN error
 *                         when a unknown configuration file option
 *                         is found, just ignore it and move on.
 */
 
#define CONFFILE_FLAG_OPTION_CASESENSITIVE      0x00000001
#define CONFFILE_FLAG_OPTION_IGNORE_UNKNOWN     0x00000002

/* DATA TYPES */

/* conffile_t
 * - conffile library handle type
 */
typedef struct conffile *conffile_t;

/* conffile_data
 * 
 * This stores data from an options arguments and is passed to the
 * callback function so the data can be read or copied.
 *
 * Buffers will be destroyed when parsing has completed, therefore a
 * callback function should not save pointers, it should copy any data
 * it wishes to save.
 */
struct conffile_data {
    int boolval;
    int intval;
    double doubleval;
    char string[CONFFILE_MAX_ARGLEN];
    int intlist[CONFFILE_MAX_ARGS];
    int intlist_len;
    double doublelist[CONFFILE_MAX_ARGS];
    int doublelist_len;
    char stringlist[CONFFILE_MAX_ARGS][CONFFILE_MAX_ARGLEN];
    int stringlist_len;
};

/* conffile_option_func
 * 
 * This is the callback function type, functions that are called after
 * an option name and its potential arguments have been parsed by the
 * conffile parser.
 *
 * 'cf' the conffile handle
 * 'data' is a pointer to argument data.  The data that should be
 *     accessed depends on the option type.
 * 'optionname' is the option name that was just parsed.
 * 'option_type' is the option type specified in conffile through a 
 *     struct conffile_option type.  See below.
 * 'option_ptr' is a pointer to data specified through a
 *     struct conffile_option type.  See below.
 * 'option_data' is an integer to data specified through a
 *     struct conffile_option type.  See below.
 * 'app_ptr' is a pointer to data specified in conffile_parse().
 * 'app_data' is an integer specified in conffile_parse().
 *
 * Typically, the option_ptr and option_data will point to a buffer
 * and its length to store argument data.  The The callback function
 * then copies data into the buffer from the data pointed at by the
 * 'data' pointer.  However, they may be used for any purpose.
 *
 * The app_ptr and app_data are passed via the conffile_parse()
 * function.  Typically, they are used to handle contexts within 
 * the configuration file, but they may be used for any purpose.
 *
 * The function should return 0 if the argument was read properly and
 * the parser should continue parsing.  Return -1 if an error has
 * occurred and you wish the parser to quit.  If an error code is set
 * in a callback function, using conffile_seterrnum(), that errnum
 * will be passed back to the original caller of conffile_parse().  If
 * no error code is set, ERR_CALLBACK will be passed back from
 * conffile_parse().
 */
typedef int (*conffile_option_func)(conffile_t cf,
                                    struct conffile_data *data,
                                    char *optionname,
                                    int option_type,
                                    void *option_ptr,
                                    int option_data,
                                    void *app_ptr,
                                    int app_data);

#define CONFFILE_OPTION_FUNC(func_name) \
    int \
    func_name(conffile_t cf, \
              struct conffile_data *data, \
              char *optionname, \
              int option_type, \
              void *option_ptr, \
              int option_data, \
              void *app_ptr, \
              int app_data)

/* conffile_option
 *
 * An array of this structure specifies the options to be searched for
 * in the configuration file.
 *
 * 'optionname' is the option name that should be serached for.
 * 'option_type' is the option type specified in conffile.  See 
 *      OPTION TYPES above.
 * 'option_type_arg' argument for the option type.  See OPTION TYPES above.
 * 'callback_func' is the callback function to be called when the
 *     option has been found.
 * 'max_count' is the maximum number of times this option can be
 *     listed in the configuration file.  Typically this is one.
 * 'required_count' is the required number of times this option should
 *     be listed in the configuration file.  Typically this is 0 for not
 *     required, or identical to 'max_count'.
 * 'count_ptr' points to an integer that will be incremented to the
 *     number of times this option has been listed in the configuration
 *     file.
 * 'option_ptr' is a pointer to data that will be passed to the callback
 *     function.  Typically, a buffer pointer is passed.  This parameter
 *     is optional and can be set to NULL.
 * 'option_data' is an integer that will be passed to the callback
 *     function.  Typically this is a buffer length.
 *
 */
struct conffile_option {
    char *optionname;
    int option_type;
    int option_type_arg;
    conffile_option_func callback_func;
    int max_count;
    int required_count;
    int *count_ptr;
    void *option_ptr;
    int option_data;
};

/* API */

/* conffile_handle_create
 * 
 * Create a conffile handle.  
 * Returns handle on success, NULL on error.
 */
conffile_t conffile_handle_create(void);

/* conffile_handle_destroy
 * 
 * Destroy a conffile handle.
 * Returns 0 on success, -1 on error.
 */
int conffile_handle_destroy(conffile_t cf);

/* conffile_errnum
 * 
 * Get the most recent error code number.
 * Returns errnum
 */
int conffile_errnum(conffile_t cf);

/* conffile_errmsg
 * 
 * Get an error message of the most recent error.  When appropriate,
 * the error message returned in the buffer will include information
 * on the optionname or line number of the parse error.  Returns 0 on
 * success, -1 if the buffer passed in is not large enough.
 */
int conffile_errmsg(conffile_t cf, char *buf, int buflen);

/* conffile_seterrnum
 * 
 * Set the error code number in a conffile handle.  This is primarily
 * used in an error handler or callback function to set error codes
 * when an error has occurred within user space parsing.  Returns 0 on
 * success, -1 if errnum is out of range.
 */   
int conffile_seterrnum(conffile_t cf, int errnum);

/* conffile_parse
 *
 * Parse a configuration file.  
 * Returns 0 on success, -1 on error.
 */
int conffile_parse(conffile_t cf, const char *filename,
                   struct conffile_option *options,
                   int options_len, void *app_ptr, int app_data,
                   int flags);

/* conffile_empty
 *
 * An empty callback function that returns 0.
 */
CONFFILE_OPTION_FUNC(conffile_empty);

/* conffile_bool
 *
 * Generic callback function for bools.  Assumes option_ptr is a
 * pointer to an integer, stores the argument, and returns 0.  If
 * option_ptr is NULL, sets errnum to ERR_PARAMETERS, and returns -1.
 */
CONFFILE_OPTION_FUNC(conffile_bool);

/* conffile_int
 *
 * Generic callback function for ints.  Assumes option_ptr is a
 * pointer to an integer, stores the argument, and returns 0.  If
 * option_ptr is NULL, sets errnum to ERR_PARAMETERS, and returns -1.
 */
CONFFILE_OPTION_FUNC(conffile_int);

/* conffile_double
 *
 * Generic callback function for doubles.  Assumes option_ptr is a
 * pointer to a double, stores the argument, and returns 0.  If
 * option_ptr is NULL, sets errnum to ERR_PARAMETERS, and returns -1.
 */
CONFFILE_OPTION_FUNC(conffile_double);

/* conffile_string
 *
 * Generic callback function for strings.  Assumes option_ptr is a
 * pointer to a char buffer and option_data is the buffer's length,
 * stores the argument, and returns 0.  If option_ptr is NULL or
 * option_data is <= 0, sets errnum to ERR_PARAMETERS, and returns
 * -1.
 */
CONFFILE_OPTION_FUNC(conffile_string);

#endif /* _CONFFILE_H */
