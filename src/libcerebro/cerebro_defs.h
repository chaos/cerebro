/*****************************************************************************\
 *  $Id: cerebro_defs.h,v 1.1 2005-03-25 19:44:05 achu Exp $
\*****************************************************************************/
 
#ifndef _CEREBRO_DEFS_H
#define _CEREBRO_DEFS_H

/* Do not use MAXHOSTNAMELEN defined (typically) in sys/param.h.  We
 * do not want the possibility that other machines may have a
 * different length definition for MAXHOSTNAMELEN.
 */
#define CEREBRO_MAXHOSTNAMELEN 64

#endif /* _CEREBRO_DEFS_H */
