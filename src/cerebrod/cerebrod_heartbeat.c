/*****************************************************************************\
 *  $Id: cerebrod_heartbeat.c,v 1.17 2005-03-30 05:41:45 achu Exp $
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

#include "cerebro_defs.h"
#include "cerebro_marshalling.h"
#include "cerebrod_heartbeat_protocol.h"

#include "cerebrod_heartbeat.h"
#include "cerebrod_config.h"
#include "cerebrod_data.h"
#include "cerebrod_error.h"
#include "wrappers.h"

extern struct cerebrod_config conf;
#ifndef NDEBUG
extern pthread_mutex_t debug_output_mutex;
#endif /* NDEBUG */

void
cerebrod_heartbeat_construct(struct cerebrod_heartbeat *hb)
{
  assert(hb);

  hb->version = CEREBROD_HEARTBEAT_PROTOCOL_VERSION;
  cerebrod_get_hostname(hb->hostname, CEREBRO_MAXHOSTNAMELEN);
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
	{
	  fprintf(stderr, "cerebrod_heartbeat_dump: "
		  "debug_output_mutex not locked");
	  exit(1);
	}
      
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

int 
cerebrod_heartbeat_marshall(struct cerebrod_heartbeat *hb, 
			    char *buffer, int bufferlen) 
{
  int ret, c = 0;

  assert(hb && buffer && bufferlen > 0);
  assert(bufferlen >= CEREBROD_HEARTBEAT_LEN);

  memset(buffer, '\0', bufferlen);
  if ((ret = cerebro_marshall_int32(hb->version, 
				    buffer + c, 
				    bufferlen - c)) < 0)
    cerebrod_err_exit("%s(%s:%d): cerebro_marshall_int32: %s",
		      __FILE__, __FUNCTION__, __LINE__,
		      strerror(errno));
  c += ret;

  if ((ret = cerebro_marshall_buffer(hb->hostname, 
                                     sizeof(hb->hostname), 
                                     buffer + c, 
                                     bufferlen - c)) < 0)
    cerebrod_err_exit("%s(%s:%d): cerebro_marshall_buffer: %s",
		      __FILE__, __FUNCTION__, __LINE__,
		      strerror(errno));
  c += ret;

  if ((ret = cerebro_marshall_uint32(hb->starttime, 
				     buffer + c, 
				     bufferlen - c)) < 0)
    cerebrod_err_exit("%s(%s:%d): cerebro_marshall_uint32: %s",
		      __FILE__, __FUNCTION__, __LINE__,
		      strerror(errno));
  c += ret;

  if ((ret = cerebro_marshall_uint32(hb->boottime, 
				     buffer + c, 
				     bufferlen - c)) < 0)
    cerebrod_err_exit("%s(%s:%d): cerebro_marshall_uint32: %s",
		      __FILE__, __FUNCTION__, __LINE__,
		      strerror(errno));
  c += ret;

  return c;
}

int 
cerebrod_heartbeat_unmarshall(struct cerebrod_heartbeat *hb, 
			      char *buffer, 
			      int bufferlen)
{
  int ret, c = 0;

  assert(hb && buffer && bufferlen >= 0);

  if (CEREBROD_HEARTBEAT_LEN > bufferlen)
    {
      cerebrod_err_debug("%s(%s:%d): received buffer length "
                         "too small: need %d, bufferlen %d", 
			 __FILE__, __FUNCTION__, __LINE__,
			 CEREBROD_HEARTBEAT_LEN, bufferlen);
      return -1;
    }
  
  if (CEREBROD_HEARTBEAT_LEN != bufferlen)
    {
      cerebrod_err_debug("%s(%s:%d): received buffer length "
                         "unexpected size: expect %d, bufferlen %d", 
			 __FILE__, __FUNCTION__, __LINE__,
			 CEREBROD_HEARTBEAT_LEN, bufferlen);
      return -1;
    }
  
  if ((ret = cerebro_unmarshall_int32(&(hb->version), 
				      buffer + c, 
				      bufferlen - c)) < 0)
      cerebrod_err_exit("%s(%s:%d): cerebro_unmarshall_int32: %s",
			__FILE__, __FUNCTION__, __LINE__,
			strerror(errno));
  c += ret;

  if ((ret = cerebro_unmarshall_buffer(hb->hostname, 
                                       sizeof(hb->hostname), 
                                       buffer + c, 
                                       bufferlen - c)) < 0)
    cerebrod_err_exit("%s(%s:%d): cerebro_unmarshall_buffer: %s",
		      __FILE__, __FUNCTION__, __LINE__,
		      strerror(errno));
  c += ret;

  if ((ret = cerebro_unmarshall_uint32(&(hb->starttime), 
				       buffer + c, 
				       bufferlen - c)) < 0)
    cerebrod_err_exit("%s(%s:%d): cerebro_unmarshall_uint32: %s",
		      __FILE__, __FUNCTION__, __LINE__,
		      strerror(errno));
  c += ret;

  if ((ret = cerebro_unmarshall_uint32(&(hb->boottime), 
				       buffer + c, 
				       bufferlen - c)) < 0)
    cerebrod_err_exit("%s(%s:%d): cerebro_unmarshall_uint32: %s",
		      __FILE__, __FUNCTION__, __LINE__,
		      strerror(errno));
  c += ret;
  
  return 0;
}
