/*****************************************************************************\
 *  $Id: argv.c,v 1.2 2005-03-17 00:23:33 achu Exp $
 *****************************************************************************
 *  Copyright (C) 2003 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Andrew Uselton (uselton2@llnl.gov>
 *  UCRL-CODE-2002-008.
 *  
 *  This file is part of PowerMan, a remote power management program.
 *  For details, see <http://www.llnl.gov/linux/powerman/>.
 *  
 *  PowerMan is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *  
 *  PowerMan is distributed in the hope that it will be useful, but WITHOUT 
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
 *  for more details.
 *  
 *  You should have received a copy of the GNU General Public License along
 *  with PowerMan; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
\*****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if STDC_HEADERS
#include <string.h>
#include <ctype.h>
#endif /* STDC_HEADERS */
#include <assert.h>

#include "argv.h"

/* make a copy of the first word in str and advance str past it */
static char *_nextargv(char **strp, char *ignore)
{
    char *str = *strp;
    char *word; 
    int len;
    char *cpy = NULL;

    while (*str && (isspace(*str) || strchr(ignore, *str)))
        str++;
    word = str;
    while (*str && !(isspace(*str) || strchr(ignore, *str)))
        str++;
    len = str - word;

    if (len > 0) {
        if (!(cpy = (char *)malloc(len + 1)))
            return NULL;
        memcpy(cpy, word, len);
        cpy[len] = '\0';
    }

    *strp = str;
    return cpy;
}

/* return number of space seperated words in str */
static int _sizeargv(char *str, char *ignore)
{
    int count = 0;

    do {
        while (*str && (isspace(*str) || strchr(ignore, *str)))
            str++;
        if (*str)
            count++;
        while (*str && !(isspace(*str) || strchr(ignore, *str)))
            str++;
    } while (*str);

    return count;
}

/* Create a null-terminated argv array given a command line.
 * Characters in the 'ignore' set are treated like white space. 
 */
int argv_create(char *cmdline, char *ignore, int *argcPtr, char ***argvPtr)
{
    int argc; 
    char **argv; 
    int i, j;

    if (!cmdline || !ignore || !argcPtr || !argvPtr) {
        errno = EINVAL;
        return -1;
    }

    if ((argc = _sizeargv(cmdline, ignore)) == 0)
        return 0;
    
    if (!(argv = (char **)malloc(sizeof(char *) * (argc + 1))))
        return -1;

    for (i = 0; i < argc; i++) {
        if (!(argv[i] = _nextargv(&cmdline, ignore))) {
            for (j = 0; j < i; j++)
                free(argv[j]);
            free(argv);
            return -1;
        }
    }
    argv[i] = NULL;

    *argcPtr = argc;
    *argvPtr = argv;
    return 0;
}

/* Destroy a null-terminated argv array.
 */
int argv_destroy(char **argv)
{
    int i;

    if (!argv) {
        errno = EINVAL;
        return -1;
    }

    for (i = 0; argv[i] != NULL; i++)
        free((void *)argv[i]);
    free((void *)argv);

    return 0;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
