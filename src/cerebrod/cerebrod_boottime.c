/*****************************************************************************\
 *  $Id: cerebrod_boottime.c,v 1.4 2004-11-08 19:07:51 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#include <ctype.h>
#endif /* STDC_HEADERS */
#include <limits.h>
#include <errno.h>

#include "cerebrod_boottime.h"
#include "error.h"
#include "wrappers.h"

#define CEREBROD_BOOTTIME_BUFLEN   4096
#define CEREBROD_BOOTTIME_FILE     "/proc/stat"
#define CEREBROD_BOOTTIME_KEYWORD  "btime"

u_int32_t cerebrod_boottime = 0;

u_int32_t
cerebrod_get_boottime(void)
{
  int fd, len;
  char *bootvalptr, *endptr, *tempptr;
  char buf[CEREBROD_BOOTTIME_BUFLEN];
  u_int32_t ret;
  
  if (cerebrod_boottime)
    return cerebrod_boottime;
  
  fd = Open(CEREBROD_BOOTTIME_FILE, O_RDONLY, 0);
  len = Read(fd, buf, CEREBROD_BOOTTIME_BUFLEN);

  bootvalptr = strstr(buf, CEREBROD_BOOTTIME_KEYWORD);
  bootvalptr += strlen(CEREBROD_BOOTTIME_KEYWORD);
  if (bootvalptr < (buf + CEREBROD_BOOTTIME_BUFLEN))
    {
      while(isspace(*bootvalptr))
        bootvalptr++;

      tempptr = bootvalptr;

      while(!isspace(*tempptr) && *tempptr != '\0')
        tempptr++;
      *tempptr = '\0';
    }
  else
    err_exit("cerebrod_boottime: boottime file parse error");

  errno = 0;
  ret = (u_int32_t)strtol(bootvalptr, &endptr, 10);
  if ((ret == LONG_MIN || ret == LONG_MAX) && errno == ERANGE)
    err_exit("cerebrod_boottime: boottime out of range");
  if ((bootvalptr + strlen(bootvalptr)) != endptr)
    err_exit("cerebrod_boottime: boottime value parse error");

  cerebrod_boottime = ret;
  return ret;
}
