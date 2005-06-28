/*****************************************************************************\
 *  $Id: cerebrod_listener.h,v 1.11 2005-06-28 00:32:12 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_LISTENER_H
#define _CEREBROD_LISTENER_H

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
