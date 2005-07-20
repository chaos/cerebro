/*****************************************************************************\
 *  $Id: cerebro_metric.h,v 1.1 2005-07-20 21:14:33 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_METRIC_H
#define _CEREBRO_METRIC_H

#include <sys/types.h>

#include <cerebro/cerebro
/*
 * struct cerebro_metric
 *
 * defines metric data
 */
struct cerebro_metric
{
  u_int32_t type;
  u_int32_t len;
  void *value;
};

#define CEREBRO_METRIC_HEADER_LEN  (sizeof(u_int32_t) + sizeof(u_int32_t))

#endif /* _CEREBRO_METRIC_H */
