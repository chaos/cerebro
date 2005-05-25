/*****************************************************************************\
 *  $Id: cerebro_marshalling.c,v 1.13 2005-05-25 20:39:35 achu Exp $
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

#include "cerebro_marshalling.h"
#include "cerebro/cerebro_error.h"

int
_cerebro_marshall_int8(int8_t val, char *buf, unsigned int buflen)
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
_cerebro_marshall_int32(int32_t val, char *buf, unsigned int buflen)
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
_cerebro_marshall_unsigned_int8(u_int8_t val, char *buf, unsigned int buflen)
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
_cerebro_marshall_unsigned_int32(u_int32_t val, char *buf, unsigned int buflen)
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
_cerebro_marshall_float(float val, char *buf, unsigned int buflen)
{
  u_int32_t temp, *ptr;
  
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

  if (sizeof(float) != sizeof(u_int32_t))
    {
      cerebro_err_debug_lib("%s(%s:%d): float size error: "
                            "float=%d u_int32_t=%d",
			    __FILE__, __FUNCTION__, __LINE__,
                            sizeof(float), sizeof(u_int32_t));
      return -1;
    }

  ptr = (u_int32_t *)&val;
  temp = htonl(*ptr);
  memcpy(buf, (void *)&temp, sizeof(u_int32_t));
  return sizeof(u_int32_t);
}
                                                                                    
int 
_cerebro_marshall_double(double val, char *buf, unsigned int buflen)
{
  u_int32_t word0, word1, *ptr;
  
  if (!buf)
    {
      cerebro_err_debug_lib("%s(%s:%d): buf null",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (buflen < (2*sizeof(u_int32_t)))
    {
      cerebro_err_debug_lib("%s(%s:%d): buflen invalid",
			    __FILE__, __FUNCTION__, __LINE__);
      return -1;
    }

  if (sizeof(double) != (2*sizeof(u_int32_t)))
    {
      cerebro_err_debug_lib("%s(%s:%d): float size error: "
                            "float=%d u_int32_t=%d",
			    __FILE__, __FUNCTION__, __LINE__,
                            sizeof(float), sizeof(u_int32_t));
      return -1;
    }

  ptr = (u_int32_t *)&val;
#ifdef WORDS_BIGENDIAN
  word0 = htonl(ptr[0]);
  word1 = htonl(ptr[1]);
#else  /* !WORDS_BIGENDIAN */
  word0 = htonl(ptr[1]);
  word1 = htonl(ptr[0]);
#endif /* !WORDS_BIGENDIAN */
  memcpy(buf, (void *)&word0, sizeof(u_int32_t));
  memcpy(buf+sizeof(u_int32_t), (void *)&word1, sizeof(u_int32_t));
  return (2*sizeof(u_int32_t));
}


int
_cerebro_marshall_buffer(const char *val, 
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
_cerebro_unmarshall_int8(int8_t *val, 
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
_cerebro_unmarshall_int32(int32_t *val, 
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

  if (buflen >= sizeof(int32_t))
    {
      int32_t temp;
      memcpy((void *)&temp, buf, sizeof(int32_t));
      *val = ntohl(temp);
      return sizeof(int32_t);
    }
  return 0;
}

int
_cerebro_unmarshall_unsigned_int8(u_int8_t *val, 
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
_cerebro_unmarshall_unsigned_int32(u_int32_t *val, 
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

  if (buflen >= sizeof(u_int32_t))
    {
      u_int32_t temp;
      memcpy((void *)&temp, buf, sizeof(u_int32_t));
      *val = ntohl(temp);
      return sizeof(u_int32_t);
    }
  return 0;
}

int 
_cerebro_unmarshall_float(float *val,
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

  if (sizeof(float) != sizeof(u_int32_t))
    {
      cerebro_err_debug_lib("%s(%s:%d): float size error: "
                            "float=%d u_int32_t=%d",
			    __FILE__, __FUNCTION__, __LINE__,
                            sizeof(float), sizeof(u_int32_t));
      return -1;
    }

  if (buflen >= sizeof(u_int32_t))
    {
      u_int32_t temp;
      u_int32_t *ptr = (u_int32_t *)val;

      memcpy((void *)&temp, buf, sizeof(u_int32_t));
      *ptr = ntohl(temp);
      return sizeof(u_int32_t);
    }
  return 0;
}

int 
_cerebro_unmarshall_double(double *val,
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

  if (sizeof(double) != (2*sizeof(u_int32_t)))
    {
      cerebro_err_debug_lib("%s(%s:%d): float size error: "
                            "float=%d u_int32_t=%d",
			    __FILE__, __FUNCTION__, __LINE__,
                            sizeof(float), sizeof(u_int32_t));
      return -1;
    }

  if (buflen >= (2*sizeof(u_int32_t)))
    {
      u_int32_t word0, word1;
      u_int32_t *ptr = (u_int32_t *)val;

      memcpy((void *)&word0, buf, sizeof(u_int32_t));
      memcpy((void *)&word1, buf+sizeof(u_int32_t), sizeof(u_int32_t));
#ifdef WORDS_BIGENDIAN
      ptr[0] = ntohl(word0);
      ptr[1] = ntohl(word1);
#else  /* !WORDS_BIGENDIAN */
      ptr[0] = ntohl(word1);
      ptr[1] = ntohl(word0);
#endif /* !WORDS_BIGENDIAN */
      return (2*sizeof(u_int32_t));
    }
  return 0;
}

int
_cerebro_unmarshall_buffer(char *val, 
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

