/*****************************************************************************\
 *  $Id: cerebro_marshalling.c,v 1.10 2005-05-05 16:12:57 achu Exp $
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

#include "cerebro/cerebro_error.h"
#include "cerebro/cerebro_marshalling.h"

int
cerebro_marshall_int8(int8_t val, char *buf, unsigned int buflen)
{
  if (!buf)
    {
      cerebro_err_debug_lib("%s(%s:%d): buf null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (buflen < sizeof(int8_t))
    {
      cerebro_err_debug_lib("%s(%s:%d): buflen invalid",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  memcpy(buf, (void *)&val, sizeof(int8_t));
  return sizeof(int8_t);
}

int
cerebro_marshall_int32(int32_t val, char *buf, unsigned int buflen)
{
  int32_t temp;

  if (!buf)
    {
      cerebro_err_debug_lib("%s(%s:%d): buf null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (buflen < sizeof(int32_t))
    {
      cerebro_err_debug_lib("%s(%s:%d): buflen invalid",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  temp = htonl(val);
  memcpy(buf, (void *)&temp, sizeof(int32_t));
  return sizeof(temp);
}

int
cerebro_marshall_uint8(u_int8_t val, char *buf, unsigned int buflen)
{
  if (!buf)
    {
      cerebro_err_debug_lib("%s(%s:%d): buf null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (buflen < sizeof(u_int8_t))
    {
      cerebro_err_debug_lib("%s(%s:%d): buflen invalid",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  memcpy(buf, (void *)&val, sizeof(u_int8_t));
  return sizeof(u_int8_t);
}

int
cerebro_marshall_uint32(u_int32_t val, char *buf, unsigned int buflen)
{
  u_int32_t temp;

  if (!buf)
    {
      cerebro_err_debug_lib("%s(%s:%d): buf null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (buflen < sizeof(u_int32_t))
    {
      cerebro_err_debug_lib("%s(%s:%d): buflen invalid",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  temp = htonl(val);
  memcpy(buf, (void *)&temp, sizeof(u_int32_t));
  return sizeof(u_int32_t);
}

int
cerebro_marshall_buffer(const char *val, 
			unsigned int vallen, 
			char *buf, 
			unsigned int buflen)
{
  if (!val)
    {
      cerebro_err_debug_lib("%s(%s:%d): val null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!vallen)
    {
      cerebro_err_debug_lib("%s(%s:%d): vallen invalid",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!buf)
    {
      cerebro_err_debug_lib("%s(%s:%d): buf null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (buflen < vallen)
    {
      cerebro_err_debug_lib("%s(%s:%d): buflen invalid",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  memcpy(buf, val, vallen);
  return vallen;
}

int
cerebro_unmarshall_int8(int8_t *val, 
			const char *buf, 
			unsigned int buflen)
{
  if (!val)
    {
      cerebro_err_debug_lib("%s(%s:%d): val null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!buf)
    {
      cerebro_err_debug_lib("%s(%s:%d): buf null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (buflen >= sizeof(int8_t))
    {
      memcpy((void *)val, buf, sizeof(int8_t));
      return sizeof(int8_t);
    }
  return 0;
}

int
cerebro_unmarshall_int32(int32_t *val, 
			 const char *buf, 
			 unsigned int buflen)
{
  int32_t temp;

  if (!val)
    {
      cerebro_err_debug_lib("%s(%s:%d): val null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!buf)
    {
      cerebro_err_debug_lib("%s(%s:%d): buf null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (buflen >= sizeof(int32_t))
    {
      memcpy((void *)&temp, buf, sizeof(int32_t));
      *val = ntohl(temp);
      return sizeof(int32_t);
    }
  return 0;
}

int
cerebro_unmarshall_uint8(u_int8_t *val, 
			 const char *buf, 
			 unsigned int buflen)
{
  if (!val)
    {
      cerebro_err_debug_lib("%s(%s:%d): val null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!buf)
    {
      cerebro_err_debug_lib("%s(%s:%d): buf null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (buflen >= sizeof(u_int8_t))
    {
      memcpy((void *)val, buf, sizeof(u_int8_t));
      return sizeof(u_int8_t);
    }
  return 0;
}

int
cerebro_unmarshall_uint32(u_int32_t *val, 
			  const char *buf, 
			  unsigned int buflen)
{
  u_int32_t temp;

  if (!val)
    {
      cerebro_err_debug_lib("%s(%s:%d): val null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!buf)
    {
      cerebro_err_debug_lib("%s(%s:%d): buf null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (buflen >= sizeof(u_int32_t))
    {
      memcpy((void *)&temp, buf, sizeof(u_int32_t));
      *val = ntohl(temp);
      return sizeof(u_int32_t);
    }
  return 0;
}

int
cerebro_unmarshall_buffer(char *val, 
			  unsigned int vallen, 
			  const char *buf, 
			  unsigned int buflen)
{
  if (!val)
    {
      cerebro_err_debug_lib("%s(%s:%d): val null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!vallen)
    {
      cerebro_err_debug_lib("%s(%s:%d): vallen invalid",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!buf)
    {
      cerebro_err_debug_lib("%s(%s:%d): buf null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (buflen >= vallen)
    {
      memcpy(val, buf, vallen);
      return vallen;
    }
  return 0;
}

