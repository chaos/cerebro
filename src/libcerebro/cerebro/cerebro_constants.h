/*****************************************************************************\
 *  $Id: cerebro_constants.h,v 1.1 2005-05-05 16:12:57 achu Exp $
\*****************************************************************************/
 
#ifndef _CEREBRO_CONSTANTS_H
#define _CEREBRO_CONSTANTS_H

/* Do not use MAXHOSTNAMELEN defined (typically) in sys/param.h.  We
 * do not want the possibility that other machines may have a
 * different length definition for MAXHOSTNAMELEN.  We define our own
 * maximum nodename length.
 */
#define CEREBRO_MAXNODENAMELEN         64 

#define CEREBRO_MAXPATHLEN             512

#define CEREBRO_PACKET_BUFLEN          4096

#define CEREBRO_MAXIPADDRLEN           64

#define CEREBRO_MAXNETWORKINTERFACELEN 128

#endif /* _CEREBRO_CONSTANTS_H */
