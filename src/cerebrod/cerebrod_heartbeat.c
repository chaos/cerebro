/*****************************************************************************\
 *  $Id: cerebrod_heartbeat.c,v 1.3 2004-12-27 16:48:27 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <netinet/in.h>
#include <assert.h>
#include <errno.h>

#include "cerebrod_heartbeat.h"
#include "error.h"

static int
_marshall_int32(int32_t val, char *buffer)
{
  int32_t temp;
  assert(buffer);
  temp = htonl(val);
  memcpy(buffer, (void *)&temp, sizeof(temp));
  return sizeof(temp);
}

static int
_marshall_uint32(u_int32_t val, char *buffer)
{
  u_int32_t temp;
  assert(buffer);
  temp = htonl(val);
  memcpy(buffer, (void *)&temp, sizeof(temp));
  return sizeof(temp);
}

static int
_marshall_buffer(char *buf, int buflen, char *buffer)
{
  assert(buf && buflen > 0 && buffer);
  memcpy(buffer, buf, buflen);
  return buflen;
}

static int
_unmarshall_int32(int32_t *val, char *buffer)
{
  int32_t temp;
  assert(val && buffer);
  memcpy((void *)&temp, buffer, sizeof(temp));
  *val = ntohl(temp);
  return sizeof(temp);
}

static int
_unmarshall_uint32(u_int32_t *val, char *buffer)
{
  u_int32_t temp;
  assert(val && buffer);
  memcpy((void *)&temp, buffer, sizeof(temp));
  *val = ntohl(temp);
  return sizeof(temp);
}

static int
_unmarshall_buffer(char *buf, int buflen, char *buffer)
{
  assert(buf && buflen > 0 && buffer);
  memcpy(buf, buffer, buflen);
  return buflen;
}

int 
cerebrod_heartbeat_marshall(struct cerebrod_heartbeat_t *cb, 
			    char *buffer, int len) 
{
  int c = 0;

  assert(cb && buffer && len > 0);

  if (CEREBROD_HEARTBEAT_LEN < len)
    err_exit("cerebrod_heartbeat_marshall: internal buffer length "
	     "too small: expect %d, len %d", CEREBROD_HEARTBEAT_LEN, 
	     len);
   
  c += _marshall_int32(cb->version, buffer + c);
  c += _marshall_buffer(cb->hostname, sizeof(cb->hostname), buffer + c);
  c += _marshall_uint32(cb->boottime, buffer + c);

  return c;
}

int 
cerebrod_heartbeat_unmarshall(struct cerebrod_heartbeat_t *cb, 
			      char *buffer, int len)
{
  int c = 0;

  assert(cb && buffer && len > 0);

  if (CEREBROD_HEARTBEAT_LEN > len)
    err_exit("cerebrod_heartbeat_ummarshall: received buffer length "
	     "too small: expect %d, len %d", CEREBROD_HEARTBEAT_LEN, 
	     len);
  if (CEREBROD_HEARTBEAT_LEN != len)
    err_debug("cerebrod_heartbeat_marshall: received buffer length "
	      "unexpected size: expect %d, len %d", CEREBROD_HEARTBEAT_LEN,
	      len);

  c += _unmarshall_int32(&(cb->version), buffer + c);
  c += _unmarshall_buffer(cb->hostname, sizeof(cb->hostname), buffer + c);
  c += _unmarshall_uint32(&(cb->boottime), buffer + c);
  
  return 0;
}
