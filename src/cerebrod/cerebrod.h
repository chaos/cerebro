/*****************************************************************************\
 *  $Id: cerebrod.h,v 1.5 2005-03-20 20:10:14 achu Exp $
\*****************************************************************************/

#ifndef _CEREBROD_H
#define _CEREBROD_H

/* Do not use MAXHOSTNAMELEN defined (typically) in sys/param.h.  We
 * do not want the possibility that other machines may have a
 * different length definition for MAXHOSTNAMELEN.
 */
#define CEREBROD_MAXHOSTNAMELEN   64

#define CEREBROD_PARSE_BUFLEN     4096
#define CEREBROD_PACKET_BUFLEN    1024
#define CEREBROD_STRING_BUFLEN    1024

#define CEREBROD_PROTOCOL_VERSION 1

#endif /* _CEREBROD_H */
