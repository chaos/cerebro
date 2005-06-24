/*****************************************************************************\
 *  $Id: cerebro_constants.h,v 1.2 2005-06-24 20:42:28 achu Exp $
\*****************************************************************************/
 
#ifndef _CEREBRO_CONSTANTS_H
#define _CEREBRO_CONSTANTS_H

/* Do not use MAXHOSTNAMELEN defined in sys/param.h.  We do not want
 * the possibility that other machines may have a different length
 * definition for MAXHOSTNAMELEN.  We define our own maximum nodename
 * length.
 */
#define CEREBRO_MAX_NODENAME_LEN           64 
#define CEREBRO_MAX_HOSTNAME_LEN           CEREBRO_MAX_NODENAME_LEN

#define CEREBRO_MAX_PATH_LEN               512

#define CEREBRO_MAX_PACKET_LEN             4096

#define CEREBRO_MAX_IPADDR_LEN             64

#define CEREBRO_MAX_NETWORK_INTERFACE_LEN  128

#define CEREBRO_MAX_METRIC_NAME_LEN        32

#endif /* _CEREBRO_CONSTANTS_H */
