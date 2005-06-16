/*****************************************************************************\
 *  $Id: cerebrod_speaker.h,v 1.4 2005-06-16 23:50:28 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_SPEAKER_H
#define _CEREBROD_SPEAKER_H

/*
 * struct cerebrod_metric
 *
 * contains cerebrod metric information
 */
struct cerebrod_metric
{
  char *metric_name;
  int index;
};

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
