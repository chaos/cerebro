/*****************************************************************************\
 *  $Id: cerebrod_data.c,v 1.12 2005-06-06 20:39:55 achu Exp $
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
#include <assert.h>
#include <errno.h>

#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_error.h"

#include "cerebrod_data.h"
#include "wrappers.h"

#define CEREBROD_BOOTTIME_BUFLEN   4096
#define CEREBROD_BOOTTIME_FILE     "/proc/stat"
#define CEREBROD_BOOTTIME_KEYWORD  "btime"

/* 
 * cerebrod_boottime
 *
 * cached system boottime 
 *
 * On some systems, due to kernel bugs, the boottime value may change
 * as the system executes.  We will read the boottime value from
 * /proc, cache it, and assume that is always correct.
 */
static u_int32_t cerebrod_boottime = 0;

/*
 * cerebrod_nodename
 * cerebrod_nodename_len
 *
 * cached system nodename and nodename length
 */
static char cerebrod_nodename[CEREBRO_MAXNODENAMELEN+1];
static int cerebrod_nodename_len = 0;

/*
 * _cerebrod_cache_boottime
 *
 * cache the system boottime
 */
static void
_cerebrod_cache_boottime(void)
{
  int fd, len;
  char *bootvalptr, *endptr, *tempptr;
  char buf[CEREBROD_BOOTTIME_BUFLEN];
  long int bootval;

  assert(!cerebrod_boottime);

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
    cerebro_err_exit("%s(%s:%d): boottime file parse error",
                     __FILE__, __FUNCTION__, __LINE__);

  errno = 0;
  bootval = (u_int32_t)strtol(bootvalptr, &endptr, 10);
  if ((bootval == LONG_MIN || bootval == LONG_MAX) && errno == ERANGE)
    cerebro_err_exit("%s(%s:%d): boottime out of range",
                     __FILE__, __FUNCTION__, __LINE__);
  if ((bootvalptr + strlen(bootvalptr)) != endptr)
    cerebro_err_exit("%s(%s:%d): boottime value parse error",
                     __FILE__, __FUNCTION__, __LINE__);

  cerebrod_boottime = (u_int32_t)bootval;
}

/*
 * _cerebrod_cache_nodename
 *
 * cache the system nodename and length
 */
static void
_cerebrod_cache_nodename(void)
{
  assert(!cerebrod_nodename_len);
  memset(cerebrod_nodename, '\0', CEREBRO_MAXNODENAMELEN+1);
  Gethostname(cerebrod_nodename, CEREBRO_MAXNODENAMELEN);
  cerebrod_nodename_len = strlen(cerebrod_nodename);
}

void
cerebrod_load_data(void)
{
  _cerebrod_cache_boottime();
  _cerebrod_cache_nodename();
}

u_int32_t
cerebrod_get_boottime(void)
{
  assert(cerebrod_boottime);
  
  return cerebrod_boottime;
}

void
cerebrod_get_nodename(char *buf, unsigned int len)
{
  assert(buf);
  assert(len > cerebrod_nodename_len);
  
  memcpy(buf, cerebrod_nodename, cerebrod_nodename_len);
}
