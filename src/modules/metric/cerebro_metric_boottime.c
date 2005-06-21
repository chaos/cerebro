/*****************************************************************************\
 *  $Id: cerebro_metric_boottime.c,v 1.7 2005-06-21 19:16:56 achu Exp $
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
#include "cerebro/cerebro_metric_module.h"

#define BOOTTIME_FILE                "/proc/stat"
#define BOOTTIME_BUFLEN              4096
#define BOOTTIME_KEYWORD             "btime"
#define BOOTTIME_METRIC_MODULE_NAME  "boottime"
#define BOOTTIME_METRIC_NAME         "boottime"

/*
 * metric_boottime
 *
 * cached system boottime
 *
 * On some systems, due to kernel bugs, the boottime value may change
 * as the system executes.  We will read the boottime value from
 * /proc, cache it, and assume that it is always correct.
 */
static u_int32_t metric_boottime = 0;

/*
 * boottime_metric_setup
 *
 * boottime metric module setup function
 */
static int
boottime_metric_setup(void)
{
  int fd, len;
  char *bootvalptr, *endptr, *tempptr;
  char buf[BOOTTIME_BUFLEN];
  long int bootval;
 
  if ((fd = open(BOOTTIME_FILE, O_RDONLY, 0)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): open: %s",
			__FILE__, __FUNCTION__, __LINE__,
			strerror(errno));
      goto cleanup;
    }

  if ((len = read(fd, buf, BOOTTIME_BUFLEN)) < 0)
    {
      cerebro_err_debug("%s(%s:%d): read: %s",
			__FILE__, __FUNCTION__, __LINE__,
			strerror(errno));
      goto cleanup;
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
      cerebro_err_debug("%s(%s:%d): boottime file parse error",
			__FILE__, __FUNCTION__, __LINE__);
      goto cleanup;
    }
                                                                                      
  errno = 0;
  bootval = (u_int32_t)strtol(bootvalptr, &endptr, 10);
  if ((bootval == LONG_MIN || bootval == LONG_MAX) && errno == ERANGE)
    {
      cerebro_err_debug("%s(%s:%d): boottime out of range",
			__FILE__, __FUNCTION__, __LINE__);
      goto cleanup;
    }
  if ((bootvalptr + strlen(bootvalptr)) != endptr)
    {
      cerebro_err_debug("%s(%s:%d): boottime value parse error",
			__FILE__, __FUNCTION__, __LINE__);
      goto cleanup;
    }
                                                                                      
  metric_boottime = (u_int32_t)bootval;
  return 0;

 cleanup:
  close(fd);
  return -1;
}

/*
 * boottime_metric_cleanup
 *
 * boottime metric module cleanup function
 */
static int
boottime_metric_cleanup(void)
{
  /* nothing to do */
  return 0;
}

/* 
 * boottime_metric_get_metric_name
 *
 * boottime metric module get_metric_name function
 *
 * Returns string on success, -1 on error
 */
static char *
boottime_metric_get_metric_name(void)
{
  return BOOTTIME_METRIC_NAME;
}

/* 
 * boottime_metric_get_metric_period
 *
 * boottime metric module get_metric_period function
 *
 * Returns period on success, -1 on error
 */
static int
boottime_metric_get_metric_period(void)
{
  /* Propogated all of the time, b/c we want to detect boottime
   * changes 
   */
  return 0;
}

/* 
 * boottime_metric_get_metric_value
 *
 * boottime metric module get_metric_value function
 *
 * Returns 0 on success, -1 on error
 */
static int
boottime_metric_get_metric_value(unsigned int *metric_value_type,
                                 unsigned int *metric_value_len,
                                 void **metric_value)
{
  if (!metric_value_type)
    {
      cerebro_err_debug("%s(%s:%d): metric_value_type null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!metric_value_len)
    {
      cerebro_err_debug("%s(%s:%d): metric_value_len null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!metric_value)
    {
      cerebro_err_debug("%s(%s:%d): metric_value null",
			__FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  *metric_value_type = CEREBRO_METRIC_VALUE_TYPE_U_INT32;
  *metric_value_len = sizeof(u_int32_t);
  *metric_value = (void *)&metric_boottime;
  return 0;
}

/* 
 * boottime_metric_destroy_metric_value
 *
 * boottime metric module destroy_metric_value function
 *
 * Returns 0 on success, -1 on error
 */
static int
boottime_metric_destroy_metric_value(void *metric_value)
{
  return 0;
}

struct cerebro_metric_module_info metric_module_info =
  {
    BOOTTIME_METRIC_MODULE_NAME,
    &boottime_metric_setup,
    &boottime_metric_cleanup,
    &boottime_metric_get_metric_name,
    &boottime_metric_get_metric_period,
    &boottime_metric_get_metric_value,
    &boottime_metric_destroy_metric_value,
  };
