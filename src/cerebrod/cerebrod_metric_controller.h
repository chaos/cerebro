/*****************************************************************************\
 *  $Id: cerebrod_metric_controller.h,v 1.1 2005-07-11 20:35:34 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_METRIC_CONTROLLER_H
#define _CEREBROD_METRIC_CONTROLLER_H

/*
 * cerebrod_metric_controller
 *
 * Runs the cerebrod metric controller thread
 *
 * Passed no argument
 *
 * Executed in detached state, no return value.
 */
void *cerebrod_metric_controller(void *);

#endif /* _CEREBROD_METRIC_CONTROLLER_H */
