/*****************************************************************************\
 *  $Id: cerebro_defs.h,v 1.4 2005-04-26 00:09:13 achu Exp $
\*****************************************************************************/
 
#ifndef _CEREBRO_DEFS_H
#define _CEREBRO_DEFS_H

#include <sys/types.h>

/* Do not use MAXHOSTNAMELEN defined (typically) in sys/param.h.  We
 * do not want the possibility that other machines may have a
 * different length definition for MAXHOSTNAMELEN.  We define our own
 * maximum nodename length.
 */
#define CEREBRO_MAXNODENAMELEN 64

/* 
 * CEREBRO_MAGIC
 *
 * Magic number for cerebro handle
 */
#define CEREBRO_MAGIC 0xF00F1234

/* 
 * Cerebro loaded state flags
 *
 * CEREBRO_UPDOWN_DATA_LOADED - Indicates updown data has been loaded
 */
#define CEREBRO_UPDOWN_DATA_LOADED   0x00000001

/* 
 * struct cerebro
 *
 * Used globally as cerebro handle
 */
struct cerebro {
  int32_t magic;
  int32_t errnum;
  int32_t loaded_state;
  void *updown_data;
};

#endif /* _CEREBRO_DEFS_H */
