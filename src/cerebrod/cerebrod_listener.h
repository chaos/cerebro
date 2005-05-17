/*****************************************************************************\
 *  $Id: cerebrod_listener.h,v 1.10 2005-05-17 22:40:02 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_LISTENER_H
#define _CEREBROD_LISTENER_H

#define CEREBROD_LISTENER_REINITIALIZE_WAIT 2

/* 
 * cerebrod_listener
 *
 * Runs a cerebrod listening thread
 *
 * Passed no argument
 * 
 * Executed in detached state, no return value.
 */
void *cerebrod_listener(void *);

#endif /* _CEREBROD_LISTENER_H */
