/*****************************************************************************\
 *  $Id: cerebrod_metric.h,v 1.2 2005-05-19 22:21:10 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_METRIC_H
#define _CEREBROD_METRIC_H

/*
 * cerebrod_metric
 *
 * Runs the cerebrod metric server thread
 *
 * Passed no argument
 *
 * Executed in detached state, no return value.
 */
void *cerebrod_metric(void *);

#endif /* _CEREBROD_METRIC_H */
