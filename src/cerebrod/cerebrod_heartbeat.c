/*****************************************************************************\
 *  $Id: cerebrod_heartbeat.c,v 1.7 2005-01-24 16:57:01 achu Exp $
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
#include "cerebrod_cache.h"
#include "cerebrod_config.h"
#include "cerebrod.h"
#include "error.h"
#include "wrappers.h"

extern struct cerebrod_config conf;
#ifndef NDEBUG
pthread_mutex_t debug_output_mutex;
#endif /* NDEBUG */

void
cerebrod_heartbeat_construct(struct cerebrod_heartbeat *hb)
{
  assert(hb);

  hb->version = CEREBROD_VERSION;
  cerebrod_get_hostname(hb->hostname, MAXHOSTNAMELEN);
  hb->starttime = cerebrod_get_starttime();
  hb->boottime = cerebrod_get_boottime();
}

void
cerebrod_heartbeat_dump(struct cerebrod_heartbeat *hb)
{
#ifndef NDEBUG
  assert(hb);

  if (conf.debug)
    {
      int ret;

      ret = Pthread_mutex_trylock(&debug_output_mutex);
      if (ret != EBUSY)
	err_exit("cerebrod_heartbeat_dump: debug_output_mutex not locked");
      
      fprintf(stderr, "**************************************\n");
      fprintf(stderr, "* Cerebrod Heartbeat:\n");     
      fprintf(stderr, "* -------------------\n");
      fprintf(stderr, "* version: %d\n", hb->version);
      fprintf(stderr, "* hostname: \"%s\"\n", hb->hostname);
      fprintf(stderr, "* starttime: %u\n", hb->starttime);
      fprintf(stderr, "* boottime: %u\n", hb->boottime);
      fprintf(stderr, "**************************************\n");
    }
#endif /* NDEBUG */
}

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
cerebrod_heartbeat_marshall(struct cerebrod_heartbeat *hb, 
			    char *buffer, int len) 
{
  int c = 0;

  assert(hb && buffer && len > 0);
  assert(len >= CEREBROD_HEARTBEAT_LEN);

  memset(buffer, '\0', len);
  c += _marshall_int32(hb->version, buffer + c);
  c += _marshall_buffer(hb->hostname, sizeof(hb->hostname), buffer + c);
  c += _marshall_uint32(hb->starttime, buffer + c);
  c += _marshall_uint32(hb->boottime, buffer + c);

  return c;
}

int 
cerebrod_heartbeat_unmarshall(struct cerebrod_heartbeat *hb, 
			      char *buffer, int len)
{
  int c = 0;

  assert(hb && buffer && len > 0);

  if (CEREBROD_HEARTBEAT_LEN > len)
    {
      err_debug("cerebrod_heartbeat_ummarshall: received buffer length "
		"too small: need %d, len %d", CEREBROD_HEARTBEAT_LEN, 
		len);
      return -1;
    }
  
  if (CEREBROD_HEARTBEAT_LEN != len)
    {
      err_debug("cerebrod_heartbeat_marshall: received buffer length "
		"unexpected size: expect %d, len %d", CEREBROD_HEARTBEAT_LEN,
		len);
      return -1;
    }

  c += _unmarshall_int32(&(hb->version), buffer + c);
  c += _unmarshall_buffer(hb->hostname, sizeof(hb->hostname), buffer + c);
  c += _unmarshall_uint32(&(hb->starttime), buffer + c);
  c += _unmarshall_uint32(&(hb->boottime), buffer + c);
  
  return 0;
}
