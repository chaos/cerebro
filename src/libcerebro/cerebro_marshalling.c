/*****************************************************************************\
 *  $Id: cerebro_marshalling.c,v 1.1 2005-03-27 08:10:24 achu Exp $
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
cerebro_marshall_int32(int32_t val, char *buffer, int bufferlen)
{
  int32_t temp;

  if (!buffer || bufferlen < sizeof(temp))
    {
      errno = EINVAL;
      return -1;
    }

  temp = htonl(val);
  memcpy(buffer, (void *)&temp, sizeof(temp));
  return sizeof(temp);
}

int
cerebro_marshall_uint32(u_int32_t val, char *buffer, int bufferlen)
{
  u_int32_t temp;

  if (!buffer || bufferlen < sizeof(temp))
    {
      errno = EINVAL;
      return -1;
    }

  temp = htonl(val);
  memcpy(buffer, (void *)&temp, sizeof(temp));
  return sizeof(temp);
}

int
cerebro_marshall_buffer(char *buf, int buflen, char *buffer, int bufferlen)
{
  if (!buf || buflen <= 0 || !buffer || bufferlen < buflen)
    {
      errno = EINVAL;
      return -1;
    }

  memcpy(buffer, buf, buflen);
  return buflen;
}

int
cerebro_unmarshall_int32(int32_t *val, char *buffer, int bufferlen)
{
  int32_t temp;

  if (!val || !buffer || bufferlen < sizeof(temp))
    {
      errno = EINVAL;
      return -1;
    }

  memcpy((void *)&temp, buffer, sizeof(temp));
  *val = ntohl(temp);
  return sizeof(temp);
}

int
cerebro_unmarshall_uint32(u_int32_t *val, char *buffer, int bufferlen)
{
  u_int32_t temp;

  if (!val || !buffer || bufferlen < sizeof(temp))
    {
      errno = EINVAL;
      return -1;
    }

  memcpy((void *)&temp, buffer, sizeof(temp));
  *val = ntohl(temp);
  return sizeof(temp);
}

int
cerebro_unmarshall_buffer(char *buf, int buflen, char *buffer, int bufferlen)
{
  if (!buf || buflen <= 0 || !buffer || bufferlen < buflen)
    {
      errno = EINVAL;
      return -1;
    }

  memcpy(buf, buffer, buflen);
  return buflen;
}

