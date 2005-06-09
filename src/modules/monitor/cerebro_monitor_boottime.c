/*****************************************************************************\
 *  $Id: cerebro_monitor_boottime.c,v 1.1 2005-06-09 20:09:51 achu Exp $
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
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */

#include "cerebro.h"
#include "cerebro/cerebro_constants.h"
#include "cerebro/cerebro_error.h"
#include "cerebro/cerebro_monitor_module.h"

#define BOOTTIME_FILE                "/proc/stat"
#define BOOTTIME_BUFLEN              4096
#define BOOTTIME_KEYWORD             "btime"
#define BOOTTIME_MONITOR_MODULE_NAME "boottime"
#define BOOTTIME_METRIC_NAME         "boottime"

/*
 * monitor_boottime
 *
 * cached system boottime
 *
 * On some systems, due to kernel bugs, the boottime value may change
 * as the system executes.  We will read the boottime value from
 * /proc, cache it, and assume that it is always correct.
 */
static u_int32_t monitor_boottime = 0;

/*
 * boottime_monitor_setup
 *
 * boottime monitor module setup function
 */
static int
boottime_monitor_setup(void)
{
  int fd, len;
  char *bootvalptr, *endptr, *tempptr;
  char buf[BOOTTIME_BUFLEN];
  long int bootval;
 
  if ((fd = open(BOOTTIME_FILE, O_RDONLY, 0)) < 0)
    {
      cerebro_err_debug_module("%s(%s:%d): open: %s",
                               __FILE__, __FUNCTION__, __LINE__,
                               strerror(errno));
      return -1;
    }

  if ((len = read(fd, buf, BOOTTIME_BUFLEN)) < 0)
    {
      cerebro_err_debug_module("%s(%s:%d): read: %s",
                               __FILE__, __FUNCTION__, __LINE__,
                               strerror(errno));
      return -1;
    }
                                                                                      
  bootvalptr = strstr(buf, BOOTTIME_KEYWORD);
  bootvalptr += strlen(BOOTTIME_KEYWORD);
  if (bootvalptr < (buf + BOOTTIME_BUFLEN))
    {
      while(isspace(*bootvalptr))
        bootvalptr++;
                                                                                      
      tempptr = bootvalptr;
                                                                                      
      while(!isspace(*tempptr) && *tempptr != '\0')
        tempptr++;
      *tempptr = '\0';
    }
  else
    {
      cerebro_err_debug_module("%s(%s:%d): boottime file parse error",
                               __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }
                                                                                      
  errno = 0;
  bootval = (u_int32_t)strtol(bootvalptr, &endptr, 10);
  if ((bootval == LONG_MIN || bootval == LONG_MAX) && errno == ERANGE)
    cerebro_err_debug_module("%s(%s:%d): boottime out of range",
                             __FILE__, __FUNCTION__, __LINE__);
  if ((bootvalptr + strlen(bootvalptr)) != endptr)
    cerebro_err_debug_module("%s(%s:%d): boottime value parse error",
                             __FILE__, __FUNCTION__, __LINE__);
                                                                                      
  monitor_boottime = (u_int32_t)bootval;
}

/*
 * boottime_monitor_cleanup
 *
 * boottime monitor module cleanup function
 */
static int
boottime_monitor_cleanup(void)
{
  /* nothing to do */
  return 0;
}

/* 
 * boottime_monitor_get_metric_name
 *
 * boottime monitor module get_metric_name function
 *
 * Returns 0 on success, -1 on error
 */
char *
boottime_monitor_get_metric_name(void)
{
  return BOOTTIME_METRIC_NAME;
}

/* 
 * boottime_monitor_get_metric_value
 *
 * boottime monitor module get_metric_value function
 *
 * Returns 0 on success, -1 on error
 */
int
boottime_monitor_get_metric_value(void *metric_value_buf,
                                  unsigned int metric_value_buflen,
                                  unsigned int *metric_value_type,
                                  unsigned int *metric_value_len)
{
  if (!metric_value_buf)
    {
      cerebro_err_debug_module("%s(%s:%d): metric_value_buf null",
                               __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!metric_value_buflen)
    {
      cerebro_err_debug_module("%s(%s:%d): metric_value_buflen invalid",
                               __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!metric_value_type)
    {
      cerebro_err_debug_module("%s(%s:%d): metric_value_type null",
                               __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!metric_value_len)
    {
      cerebro_err_debug_module("%s(%s:%d): metric_value_len null",
                               __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (metric_value_buflen < sizeof(u_int32_t))
    return 0;

  memcpy(metric_value_buf, &monitor_boottime, sizeof(u_int32_t));
  *metric_value_type = CEREBRO_METRIC_VALUE_TYPE_UNSIGNED_INT32;
  *metric_value_len = sizeof(u_int32_t);

  return sizeof(u_int32_t);
}

#if WITH_STATIC_MODULES
struct cerebro_monitor_module_info boottime_monitor_module_info =
#else /* !WITH_STATIC_MODULES */
struct cerebro_monitor_module_info monitor_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    BOOTTIME_MONITOR_MODULE_NAME,
    &boottime_monitor_setup,
    &boottime_monitor_cleanup,
    &boottime_monitor_get_metric_name,
    &boottime_monitor_get_metric_value,
  };
