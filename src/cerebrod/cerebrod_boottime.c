/*****************************************************************************\
 *  $Id: cerebrod_boottime.c,v 1.1.1.1 2004-07-02 22:31:29 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#include <ctype.h>
#endif
#include <limits.h>
#include <errno.h>

#include "cerebrod_boottime.h"
#include "error.h"
#include "fd.h"
#include "wrappers.h"

#define CEREBROD_BOOTTIME_BUFLEN   4096
#define CEREBROD_BOOTTIME_FILE     "/proc/stat"
#define CEREBROD_BOOTTIME_KEYWORD  "btime"

/* achu: Its important to call this function only one time at the
 * beginning of the program.  Some systems have a bug in which system
 * boottime in /proc/stat will change +/- 1 second.
 */
time_t
cerebrod_boottime(void)
{
  int fd, len;
  char *bootvalptr, *endptr, *tempptr;
  char buf[CEREBROD_BOOTTIME_BUFLEN];
  time_t ret;
  
  fd = Open(CEREBROD_BOOTTIME_FILE, O_RDONLY, 0);
  len = Read(fd, buf, CEREBROD_BOOTTIME_BUFLEN);

  bootvalptr = strstr(buf, CEREBROD_BOOTTIME_KEYWORD);
  bootvalptr += strlen(CEREBROD_BOOTTIME_KEYWORD);
  if (bootvalptr < (buf + CEREBROD_BOOTTIME_BUFLEN))
    {
      while(isspace(*bootvalptr))
        bootvalptr++;

      tempptr = bootvalptr;

      while(!isspace(*tempptr))
        tempptr++;
      *tempptr = '\0';
    }
  else
    err_exit("cerebrod_boottime: boottime file parse error");

  errno = 0;
  ret = (time_t)strtol(bootvalptr, &endptr, 10);
  if ((ret == LONG_MIN || ret == LONG_MAX) && errno == ERANGE)
    err_exit("cerebrod_boottime: boottime out of range");
  if ((bootvalptr + strlen(bootvalptr)) != endptr)
    err_exit("cerebrod_boottime: boottime value parse error");

  return ret;
}
