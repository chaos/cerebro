/*****************************************************************************\
 *  $Id: cerebrod_heartbeat.c,v 1.11 2005-03-20 20:21:18 achu Exp $
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
#include "cerebrod_config.h"
#include "cerebrod_data.h"
#include "cerebrod.h"
#include "error.h"
#include "wrappers.h"

extern struct cerebrod_config conf;
#ifndef NDEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* NDEBUG */

/*
 * cerebrod_heartbeat_construct
 *
 * construct a heartbeat packet
 */
void
cerebrod_heartbeat_construct(struct cerebrod_heartbeat *hb)
{
  assert(hb);

  hb->version = CEREBROD_PROTOCOL_VERSION;
  cerebrod_get_hostname(hb->hostname, CEREBROD_MAXHOSTNAMELEN);
  hb->starttime = cerebrod_get_starttime();
  hb->boottime = cerebrod_get_boottime();
}

/*
 * cerebrod_heartbeat_dump
 *
 * dump contents of a heartbeat packet
 */
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

/* 
 * _marshall_int32
 *
 * marshall contents of a 32 bit integer
 *
 * Returns length of data copied into buffer
 */
static int
_marshall_int32(int32_t val, char *buffer)
{
  int32_t temp;
  assert(buffer);
  temp = htonl(val);
  memcpy(buffer, (void *)&temp, sizeof(temp));
  return sizeof(temp);
}

/* 
 * _marshall_uint32
 *
 * marshall contents of an unsigned 32 bit integer
 *
 * Returns length of data copied into buffer
 */
static int
_marshall_uint32(u_int32_t val, char *buffer)
{
  u_int32_t temp;
  assert(buffer);
  temp = htonl(val);
  memcpy(buffer, (void *)&temp, sizeof(temp));
  return sizeof(temp);
}

/* 
 * _marshall_buffer
 *
 * marshall contents of a buffer
 *
 * Returns length of data copied into buffer
 */
static int
_marshall_buffer(char *buf, int buflen, char *buffer)
{
  assert(buf && buflen > 0 && buffer);
  memcpy(buffer, buf, buflen);
  return buflen;
}

/*
 * cerebrod_heartbeat_marshall
 *
 * marshall contents of a heartbeat packet.
 *
 * Returns length of data copied into buffer, -1 on error
 */
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

/* 
 * _unmarshall_int32
 *
 * unmarshall contents of a 32 bit integer
 *
 * Returns length of data read from buffer
 */
static int
_unmarshall_int32(int32_t *val, char *buffer)
{
  int32_t temp;
  assert(val && buffer);
  memcpy((void *)&temp, buffer, sizeof(temp));
  *val = ntohl(temp);
  return sizeof(temp);
}

/* 
 * _unmarshall_uint32
 *
 * unmarshall contents of an unsigned 32 bit integer
 *
 * Returns length of data read from buffer
 */
static int
_unmarshall_uint32(u_int32_t *val, char *buffer)
{
  u_int32_t temp;
  assert(val && buffer);
  memcpy((void *)&temp, buffer, sizeof(temp));
  *val = ntohl(temp);
  return sizeof(temp);
}

/* 
 * _unmarshall_buffer
 *
 * unmarshall contents of a buffer
 *
 * Returns length of data read from buffer
 */
static int
_unmarshall_buffer(char *buf, int buflen, char *buffer)
{
  assert(buf && buflen > 0 && buffer);
  memcpy(buf, buffer, buflen);
  return buflen;
}

/* 
 * cerebrod_heartbeat_unmarshall
 *
 * unmarshall contents of a packet buffer
 *
 * Returns 0 on success, -1 on error 
 */
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
