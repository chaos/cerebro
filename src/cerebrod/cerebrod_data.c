/*****************************************************************************\
 *  $Id: cerebrod_data.c,v 1.1 2005-03-20 20:10:14 achu Exp $
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

#include "cerebrod.h"
#include "cerebrod_data.h"
#include "error.h"
#include "wrappers.h"

#define CEREBROD_BOOTTIME_BUFLEN   4096
#define CEREBROD_BOOTTIME_FILE     "/proc/stat"
#define CEREBROD_BOOTTIME_KEYWORD  "btime"

/* cerebrod_starttime 
 *
 * cached cerebrod starttime value
 */
static u_int32_t cerebrod_starttime = 0;


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
 * cerebrod_hostname
 * cerebrod_hostname_len
 *
 * cached system hostname and hostname length
 */
static char cerebrod_hostname[CEREBROD_MAXHOSTNAMELEN+1];
static int cerebrod_hostname_len = 0;

/*
 * _cerebrod_cache_starttime
 *
 * cache the cerebrod starttime
 */
static void
_cerebrod_cache_starttime(void)
{
  struct timeval tv;

  assert(!cerebrod_starttime);

  Gettimeofday(&tv, NULL);

  cerebrod_starttime = tv.tv_sec;
}

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
  u_int32_t ret;

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
    err_exit("cerebrod_boottime: boottime file parse error");

  errno = 0;
  ret = (u_int32_t)strtol(bootvalptr, &endptr, 10);
  if ((ret == LONG_MIN || ret == LONG_MAX) && errno == ERANGE)
    err_exit("cerebrod_boottime: boottime out of range");
  if ((bootvalptr + strlen(bootvalptr)) != endptr)
    err_exit("cerebrod_boottime: boottime value parse error");

  cerebrod_boottime = ret;
}

/*
 * _cerebrod_cache_hostname
 *
 * cache the system hostname and length
 */
static void
_cerebrod_cache_hostname(void)
{
  assert(!cerebrod_hostname_len);
  memset(cerebrod_hostname, '\0', CEREBROD_MAXHOSTNAMELEN+1);
  Gethostname(cerebrod_hostname, CEREBROD_MAXHOSTNAMELEN);
  cerebrod_hostname_len = strlen(cerebrod_hostname);
}

/* 
 * cerebrod_load_data
 *
 * Load and cache cerebrod starttme, machine boottime, and machine
 * hostname
 */
void
cerebrod_load_data(void)
{
  _cerebrod_cache_starttime();
  _cerebrod_cache_boottime();
  _cerebrod_cache_hostname();
}

/* 
 * cerebrod_get_starttime
 *
 * Return the cached cerebrod starttime
 */
u_int32_t
cerebrod_get_starttime(void)
{
  assert(cerebrod_starttime);

  return cerebrod_starttime;
}

/* 
 * cerebrod_get_boottime
 *
 * Return the cached system boottime
 */
u_int32_t
cerebrod_get_boottime(void)
{
  assert(cerebrod_boottime);
  
  return cerebrod_boottime;
}

/* 
 * cerebrod_get_hostname
 *
 * Return the cached system hostname
 */
void
cerebrod_get_hostname(char *buf, unsigned int len)
{
  assert(buf && len > 0);
  assert(len > cerebrod_hostname_len);
  
  memcpy(buf, cerebrod_hostname, cerebrod_hostname_len);
}
