/*****************************************************************************\
 *  $Id: cerebrod_speaker.h,v 1.3 2005-03-20 21:24:58 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_SPEAKER_H
#define _CEREBROD_SPEAKER_H

/* 
 * cerebrod_speaker
 *
 * Runs the cerebrod speaker thread
 *
 * Passed no argument
 * 
 * Executed in detached state, no return value.
 */
void *cerebrod_speaker(void *);

#endif /* _CEREBROD_SPEAKER_H */
