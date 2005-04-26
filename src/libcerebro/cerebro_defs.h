/*****************************************************************************\
 *  $Id: cerebro_defs.h,v 1.5 2005-04-26 17:04:29 achu Exp $
\*****************************************************************************/
 
#ifndef _CEREBRO_DEFS_H
#define _CEREBRO_DEFS_H

/* Do not use MAXHOSTNAMELEN defined (typically) in sys/param.h.  We
 * do not want the possibility that other machines may have a
 * different length definition for MAXHOSTNAMELEN.  We define our own
 * maximum nodename length.
 */
#define CEREBRO_MAXNODENAMELEN 64

#define CEREBRO_PACKET_BUFLEN  4096

#endif /* _CEREBRO_DEFS_H */
