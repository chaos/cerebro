/*****************************************************************************\
 *  $Id: cerebro_defs.h,v 1.8 2005-05-02 17:50:34 achu Exp $
\*****************************************************************************/
 
#ifndef _CEREBRO_DEFS_H
#define _CEREBRO_DEFS_H

/* Do not use MAXHOSTNAMELEN defined (typically) in sys/param.h.  We
 * do not want the possibility that other machines may have a
 * different length definition for MAXHOSTNAMELEN.  We define our own
 * maximum nodename length.
 */
#define CEREBRO_MAXNODENAMELEN      64

#define CEREBRO_MAXPATHLEN          512

#define CEREBRO_PACKET_BUFLEN       4096

#define CEREBRO_IPADDRSTRLEN        64

#define CEREBRO_MAXNETWORKINTERFACE 128

#endif /* _CEREBRO_DEFS_H */
