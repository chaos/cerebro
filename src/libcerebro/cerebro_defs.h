/*****************************************************************************\
 *  $Id: cerebro_defs.h,v 1.3 2005-04-25 23:25:23 achu Exp $
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
 * struct cerebro
 *
 * Used globally as cerebro handle
 */
struct cerebro {
  int32_t magic;
  int32_t errnum;
  int32_t loaded_flags;
};

#define CEREBRO_MAGIC            0xf00f1234

#endif /* _CEREBRO_DEFS_H */
