/*****************************************************************************\
 *  $Id: cerebrod_heartbeat.c,v 1.1 2004-11-08 19:07:51 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <assert.h>
#include <errno.h>

#include "cerebrod_heartbeat.h"
#include "error.h"

/*
struct cerebro_heartbeat_t
  {
    int32_t version;
    char hostname[MAXHOSTNAMELEN];
    u_int32_t boottime;
  };
*/

int 
cerebrod_heartbeat_marshall(struct cerebro_heartbeat_t *cb, 
			    char *buffer, int len) 
{
  int32_t temp;
  int ret_len = 0;

  if ((ret_len + sizeof(temp)) > len)
    err_exit("cerebrod_heartbeat_marshall: internal buffer length "
	     "too short: %d", len);
  temp = htonl(cb->version);
  memcpy(buffer + ret_len, (void *)&temp, sizeof(temp));
  ret_len += sizeof(temp);

  if ((ret_len + sizeof(temp)) > len)
    err_exit("cerebrod_heartbeat_marshall: internal buffer length "
	     "too short: %d", len);
  memcpy(buffer + ret_len, cb->hostname, MAXHOSTNAMELEN);
  ret_len += MAXHOSTNAMELEN;

  if ((ret_len + sizeof(temp)) > len)
    err_exit("cerebrod_heartbeat_marshall: internal buffer length "
	     "too short: %d", len);
  temp = htonl(cb->boottime);
  memcpy(buffer + ret_len, (void *)&temp, sizeof(temp));
  ret_len += sizeof(temp);

  return ret_len;
}

int 
cerebrod_heartbeat_unmarshall(struct cerebro_heartbeat_t *cb, 
			      char *buffer, int len)
{
}
