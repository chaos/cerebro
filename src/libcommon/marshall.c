/*****************************************************************************\
 *  $Id: marshall.c,v 1.1 2005-06-16 15:45:32 achu Exp $
\*****************************************************************************/
 
#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */
 
#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <errno.h>
#include <netinet/in.h>

#include "marshall.h"

int
marshall_int8(int8_t val, char *buf, unsigned int buflen)
{
  if (!buf)
    {
      errno = EINVAL;
      return -1;
    }

  if (buflen >= sizeof(int8_t))
    {
      memcpy(buf, (void *)&val, sizeof(int8_t));
      return sizeof(int8_t);
    }
  return 0;
}

int
marshall_int32(int32_t val, char *buf, unsigned int buflen)
{
  if (!buf)
    {
      errno = EINVAL;
      return -1;
    }

  if (buflen >= sizeof(int32_t))
    {
      int32_t temp;
      temp = htonl(val);
      memcpy(buf, (void *)&temp, sizeof(int32_t));
      return sizeof(int32_t);
    }
  return 0;
}

int
marshall_u_int8(u_int8_t val, char *buf, unsigned int buflen)
{
  if (!buf)
    {
      errno = EINVAL;
      return -1;
    }

  if (buflen >= sizeof(u_int8_t))
    {
      memcpy(buf, (void *)&val, sizeof(u_int8_t));
      return sizeof(u_int8_t);
    }
  return 0;
}

int
marshall_u_int32(u_int32_t val, char *buf, unsigned int buflen)
{
  if (!buf)
    {
      errno = EINVAL;
      return -1;
    }

  if (buflen >= sizeof(u_int32_t))
    {
      u_int32_t temp;
      temp = htonl(val);
      memcpy(buf, (void *)&temp, sizeof(u_int32_t));
      return sizeof(u_int32_t);
    }
  return 0;
}

int 
marshall_float(float val, char *buf, unsigned int buflen)
{
  if (!buf || sizeof(float) != sizeof(u_int32_t))
    {
      errno = EINVAL;
      return -1;
    }

  if (buflen >= sizeof(u_int32_t))
    {
      u_int32_t temp, *ptr;
      ptr = (u_int32_t *)&val;
      temp = htonl(*ptr);
      memcpy(buf, (void *)&temp, sizeof(u_int32_t));
      return sizeof(u_int32_t);
    }
  return 0;
}
                                                                                    
int 
marshall_double(double val, char *buf, unsigned int buflen)
{
  
  if (!buf || sizeof(double) != (2*sizeof(u_int32_t)))
    {
      errno = EINVAL;
      return -1;
    }

  if (buflen >= (2*sizeof(u_int32_t)))
    {
      u_int32_t word0, word1, *ptr;
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
  return 0;
}


int
marshall_buffer(const char *val, 
                unsigned int vallen, 
                char *buf, 
                unsigned int buflen)
{
  if (!val || !buf)
    {
      errno = EINVAL;
      return -1;
    }

  if (buflen >= vallen)
    {
      memcpy(buf, val, vallen);
      return vallen;
    }
  return 0;
}

int
unmarshall_int8(int8_t *val, const char *buf, unsigned int buflen)
{
  if (!val || !buf)
    {
      errno = EINVAL;
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
unmarshall_int32(int32_t *val, const char *buf, unsigned int buflen)
{
  if (!val || !buf)
    {
      errno = EINVAL;
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
unmarshall_u_int8(u_int8_t *val, const char *buf, unsigned int buflen)
{
  if (!val || !buf)
    {
      errno = EINVAL;
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
unmarshall_u_int32(u_int32_t *val, const char *buf, unsigned int buflen)
{
  if (!val || !buf)
    {
      errno = EINVAL;
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
unmarshall_float(float *val, const char *buf, unsigned int buflen)
{
  if (!val || !buf || sizeof(float) != sizeof(u_int32_t))
    {
      errno = EINVAL;
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
unmarshall_double(double *val, const char *buf, unsigned int buflen)
{
  if (!val || !buf || sizeof(double) != (2*sizeof(u_int32_t)))
    {
      errno = EINVAL;
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
unmarshall_buffer(char *val, 
                  unsigned int vallen, 
                  const char *buf, 
                  unsigned int buflen)
{
  if (!val || !buf)
    {
      errno = EINVAL;
      return -1;
    }

  if (buflen >= vallen)
    {
      memcpy(val, buf, vallen);
      return vallen;
    }
  return 0;
}

