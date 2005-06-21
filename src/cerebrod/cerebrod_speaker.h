/*****************************************************************************\
 *  $Id: cerebrod_speaker.h,v 1.5 2005-06-21 20:29:10 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_SPEAKER_H
#define _CEREBROD_SPEAKER_H

/*
 * struct cerebrod_metric_module
 *
 * contains cerebrod metric information
 */
struct cerebrod_metric_module
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
