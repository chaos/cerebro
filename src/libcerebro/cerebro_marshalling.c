/*****************************************************************************\
 *  $Id: cerebro_marshalling.c,v 1.5 2005-05-04 00:02:46 achu Exp $
\*****************************************************************************/
 
#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */
 
#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>

int
cerebro_marshall_int8(int8_t val, char *buffer, int bufferlen)
{
  if (!buffer)
    {
      cerebro_err_debug_lib("%s(%s:%d): buffer null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (bufferlen < sizeof(int8_t))
    {
      cerebro_err_debug_lib("%s(%s:%d): bufferlen invalid",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  memcpy(buffer, (void *)&val, sizeof(int8_t));
  return sizeof(int8_t);
}

int
cerebro_marshall_int32(int32_t val, char *buffer, int bufferlen)
{
  int32_t temp;

  if (!buffer)
    {
      cerebro_err_debug_lib("%s(%s:%d): buffer null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (bufferlen < sizeof(int32_t))
    {
      cerebro_err_debug_lib("%s(%s:%d): bufferlen invalid",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  temp = htonl(val);
  memcpy(buffer, (void *)&temp, sizeof(temp));
  return sizeof(temp);
}

int
cerebro_marshall_uint8(u_int8_t val, char *buffer, int bufferlen)
{
  if (!buffer)
    {
      cerebro_err_debug_lib("%s(%s:%d): buffer null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (bufferlen < sizeof(u_int8_t))
    {
      cerebro_err_debug_lib("%s(%s:%d): bufferlen invalid",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  memcpy(buffer, (void *)&val, sizeof(u_int8_t));
  return sizeof(u_int8_t);
}

int
cerebro_marshall_uint32(u_int32_t val, char *buffer, int bufferlen)
{
  u_int32_t temp;

  if (!buffer)
    {
      cerebro_err_debug_lib("%s(%s:%d): buffer null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (bufferlen < sizeof(u_int32_t))
    {
      cerebro_err_debug_lib("%s(%s:%d): bufferlen invalid",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  temp = htonl(val);
  memcpy(buffer, (void *)&temp, sizeof(temp));
  return sizeof(temp);
}

int
cerebro_marshall_buffer(char *buf, int buflen, char *buffer, int bufferlen)
{
  if (!buf)
    {
      cerebro_err_debug_lib("%s(%s:%d): buf null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (buflen <= 0)
    {
      cerebro_err_debug_lib("%s(%s:%d): buflen invalid",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!buffer)
    {
      cerebro_err_debug_lib("%s(%s:%d): buffer null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (bufferlen < buflen)
    {
      cerebro_err_debug_lib("%s(%s:%d): bufferlen invalid",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  memcpy(buffer, buf, buflen);
  return buflen;
}

int
cerebro_unmarshall_int8(int8_t *val, char *buffer, int bufferlen)
{
  if (!val)
    {
      cerebro_err_debug_lib("%s(%s:%d): val null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!buffer)
    {
      cerebro_err_debug_lib("%s(%s:%d): buffer null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (bufferlen >= sizeof(int8_t))
    {
      memcpy((void *)val, buffer, sizeof(int8_t));
      return sizeof(int8_t);
    }
  return 0;
}

int
cerebro_unmarshall_int32(int32_t *val, char *buffer, int bufferlen)
{
  int32_t temp;

  if (!val)
    {
      cerebro_err_debug_lib("%s(%s:%d): val null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!buffer)
    {
      cerebro_err_debug_lib("%s(%s:%d): buffer null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (bufferlen >= sizeof(temp))
    {
      memcpy((void *)&temp, buffer, sizeof(temp));
      *val = ntohl(temp);
      return sizeof(temp);
    }
  return 0;
}

int
cerebro_unmarshall_uint8(u_int8_t *val, char *buffer, int bufferlen)
{
  if (!val)
    {
      cerebro_err_debug_lib("%s(%s:%d): val null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!buffer)
    {
      cerebro_err_debug_lib("%s(%s:%d): buffer null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (bufferlen >= sizeof(u_int8_t))
    {
      memcpy((void *)val, buffer, sizeof(u_int8_t));
      return sizeof(u_int8_t);
    }
  return 0;
}

int
cerebro_unmarshall_uint32(u_int32_t *val, char *buffer, int bufferlen)
{
  u_int32_t temp;

  if (!val)
    {
      cerebro_err_debug_lib("%s(%s:%d): val null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!buffer)
    {
      cerebro_err_debug_lib("%s(%s:%d): buffer null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (bufferlen >= sizeof(temp))
    {
      memcpy((void *)&temp, buffer, sizeof(temp));
      *val = ntohl(temp);
      return sizeof(temp);
    }
  return 0;
}

int
cerebro_unmarshall_buffer(char *buf, int buflen, char *buffer, int bufferlen)
{
  if (!buf)
    {
      cerebro_err_debug_lib("%s(%s:%d): buf null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (!buffer)
    {
      cerebro_err_debug_lib("%s(%s:%d): buffer null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }


  if (buflen <= 0)
    {
      cerebro_err_debug_lib("%s(%s:%d): buflen invalid",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (bufferlen >= buflen)
    {
      memcpy(buf, buffer, buflen);
      return buflen;
    }
  return 0;
}

