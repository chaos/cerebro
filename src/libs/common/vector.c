/*****************************************************************************
 *  $Id: vector.c,v 1.12 2010-02-02 01:01:20 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2018 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2005-2007 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>.
 *  UCRL-CODE-155989 All rights reserved.
 *
 *  This file is part of Cerebro, a collection of cluster monitoring
 *  tools and libraries.  For details, see
 *  <http://www.llnl.gov/linux/cerebro/>.
 *
 *  Cerebro is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  Cerebro is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with Cerebro.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************
 *  Copyright (C) 2001-2002 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Chris Dunlap <cdunlap@llnl.gov>.
 *
 *  This file is from LSD-Tools, the LLNL Software Development Toolbox.
 *
 *  LSD-Tools is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  LSD-Tools is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with LSD-Tools; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 *****************************************************************************/


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#ifdef WITH_PTHREADS
#include <pthread.h>
#endif /* WITH_PTHREADS */
#include <sys/types.h>
#include <errno.h>
#include <assert.h>

#include "vector.h"

/*********************
 *  lsd_fatal_error  *
 *********************/

#ifdef WITH_LSD_FATAL_ERROR_FUNC
#  undef lsd_fatal_error
extern void lsd_fatal_error(char *file, int line, char *mesg);
#else /* !WITH_LSD_FATAL_ERROR_FUNC */
#  ifndef lsd_fatal_error
#    include <errno.h>
#    include <stdio.h>
#    include <string.h>
#    define lsd_fatal_error(file, line, mesg)                                 \
       do {                                                                   \
           fprintf(stderr, "ERROR: [%s:%d] %s: %s\n",                         \
                   file, line, mesg, strerror(errno));                        \
       } while (0)
#  endif /* !lsd_fatal_error */
#endif /* !WITH_LSD_FATAL_ERROR_FUNC */


/*********************
 *  lsd_nomem_error  *
 *********************/

#ifdef WITH_LSD_NOMEM_ERROR_FUNC
#  undef lsd_nomem_error
extern void * lsd_nomem_error(char *file, int line, char *mesg);
#else /* !WITH_LSD_NOMEM_ERROR_FUNC */
#  ifndef lsd_nomem_error
#    define lsd_nomem_error(file, line, mesg) (NULL)
#  endif /* !lsd_nomem_error */
#endif /* !WITH_LSD_NOMEM_ERROR_FUNC */

/***************
 *  Constants  *
 ***************/

#define VECTOR_MAGIC          0x937aabbc
#define VECTOR_ITERATOR_MAGIC 0x9e74abd2

/****************
 *  Data Types  *
 ****************/

struct vector {
  VectorDelF f;                 /* delete function */
  unsigned int count;           /* number of data elements stored */
  unsigned int length;          /* length of current data array */
  void **data;                  /* data array */
#ifdef WITH_PTHREADS
  pthread_mutex_t mutex;
#endif /* WITH_PTHREADS */
#ifndef NDEBUG
  int32_t magic;                /* magic number */
#endif /* !NDEBUG */
};

struct vectorIterator {
  struct vector *v;
  unsigned int current;
#ifndef NDEBUG
  int32_t magic;                /* magic number */
#endif /* !NDEBUG */
};

#ifdef WITH_PTHREADS

#  define vector_mutex_init(mutex)                                            \
     do {                                                                     \
         int e = pthread_mutex_init(mutex, NULL);                             \
         if (e != 0) {                                                        \
             errno = e;                                                       \
             lsd_fatal_error(__FILE__, __LINE__, "vector mutex init");        \
             abort();                                                         \
         }                                                                    \
     } while (0)

#  define vector_mutex_lock(mutex)                                            \
     do {                                                                     \
         int e = pthread_mutex_lock(mutex);                                   \
         if (e != 0) {                                                        \
             errno = e;                                                       \
             lsd_fatal_error(__FILE__, __LINE__, "vector mutex lock");        \
             abort();                                                         \
         }                                                                    \
     } while (0)

#  define vector_mutex_unlock(mutex)                                          \
     do {                                                                     \
         int e = pthread_mutex_unlock(mutex);                                 \
         if (e != 0) {                                                        \
             errno = e;                                                       \
             lsd_fatal_error(__FILE__, __LINE__, "vector mutex unlock");      \
             abort();                                                         \
         }                                                                    \
     } while (0)

#  define vector_mutex_destroy(mutex)                                         \
     do {                                                                     \
         int e = pthread_mutex_destroy(mutex);                                \
         if (e != 0) {                                                        \
             errno = e;                                                       \
             lsd_fatal_error(__FILE__, __LINE__, "vector mutex destroy");     \
             abort();                                                         \
         }                                                                    \
     } while (0)

#  ifndef NDEBUG
static int vector_mutex_is_locked (pthread_mutex_t *mutex);
#  endif /* !NDEBUG */

#else /* !WITH_PTHREADS */

#  define vector_mutex_init(mutex)
#  define vector_mutex_lock(mutex)
#  define vector_mutex_unlock(mutex)
#  define vector_mutex_destroy(mutex)
#  define vector_mutex_is_locked(mutex) (1)

#endif /* !WITH_PTHREADS */

#ifndef NDEBUG
#ifdef WITH_PTHREADS
static int
vector_mutex_is_locked (pthread_mutex_t *mutex)
{
  int rv;

  assert(mutex);
  rv = pthread_mutex_trylock(mutex);
  return(rv == EBUSY ? 1 : 0);
}
#endif /* WITH_PTHREADS */
#endif /* !NDEBUG */

static void *
_alloc_data (Vector v, unsigned int length_increase)
{
  void **ptr;
  int i;

  assert(v);
  assert(v->magic == VECTOR_MAGIC);
  assert(vector_mutex_is_locked(&(v->mutex)));
  assert(length_increase);

  if (!(ptr = (void **)realloc(v->data, (v->length + length_increase)*sizeof(void *))))
    {
      errno = ENOMEM;
      return lsd_nomem_error(__FILE__, __LINE__, "data create");
    }

  for (i = v->length; i < (v->length + length_increase); i++)
    ptr[i] = NULL;

  v->data = ptr;
  v->length += length_increase;

  return v->data;
}

static int
_clear_data (Vector v)
{
  int i, rv = 0;

  assert(v);
  assert(v->magic == VECTOR_MAGIC);
  assert(vector_mutex_is_locked(&(v->mutex)));

  if (v->data)
    {
      for (i = 0; i < v->length; i++)
        {
          if (v->f && v->data[i])
            v->f(v->data[i]);
          v->data[i] = NULL;
        }
      free(v->data);
      v->data = NULL;

      rv = v->count;

      v->count = 0;
      v->length = 0;
    }

  return rv;
}

Vector
vector_create (VectorDelF f)
{
  struct vector *v;

  if (!(v = (struct vector *)malloc(sizeof(struct vector))))
    {
      errno = ENOMEM;
      return lsd_nomem_error(__FILE__, __LINE__, "vector create");
    }

  memset(v, '\0', sizeof(struct vector));
  v->f = f;
  v->count = 0;
  v->length = 0;
  v->data = NULL;
  vector_mutex_init(&(v->mutex));
  assert(v->magic = VECTOR_MAGIC); /* set magic via assert abuse */

  return v;
}

void
vector_destroy (Vector v)
{
  assert(v);
  vector_mutex_lock(&v->mutex);
  assert(v->magic == VECTOR_MAGIC);

  (void)_clear_data(v);

  assert(v->magic = ~VECTOR_MAGIC); /* clear magic via assert abuse */
  vector_mutex_unlock(&v->mutex);
  vector_mutex_destroy(&v->mutex);
  free(v);
}

int
vector_is_empty (Vector v)
{
  unsigned int n;

  assert(v);
  vector_mutex_lock(&v->mutex);
  assert(v->magic == VECTOR_MAGIC);
  n = v->count;
  vector_mutex_unlock(&v->mutex);
  return (n == 0);
}

int
vector_length (Vector v)
{
  unsigned int n;

  assert(v);
  vector_mutex_lock(&v->mutex);
  assert(v->magic == VECTOR_MAGIC);
  n = v->length;
  vector_mutex_unlock(&v->mutex);
  return n;
}

int
vector_count (Vector v)
{
  unsigned int n;

  assert(v);
  vector_mutex_lock(&v->mutex);
  assert(v->magic == VECTOR_MAGIC);
  n = v->count;
  vector_mutex_unlock(&v->mutex);
  return n;
}

void *
vector_append (Vector v, void *x)
{
  void *rv = NULL;

  assert(v);
  assert(x);
  vector_mutex_lock(&v->mutex);
  assert(v->magic == VECTOR_MAGIC);

  if (!_alloc_data(v, 1))
    goto cleanup;

  v->data[v->length - 1] = x;
  v->count++;
  rv = x;

 cleanup:
  vector_mutex_unlock(&v->mutex);
  return rv;
}

void *
vector_set (Vector v, void *x, unsigned int index)
{
  void *rv = NULL;

  assert(v);
  assert(index >= 0);
  vector_mutex_lock(&v->mutex);
  assert(v->magic == VECTOR_MAGIC);

  if (index < v->length)
    {
      if (v->data[index])
        {
          if (v->f)
            {
              v->f(v->data[index]);
              v->data[index] = NULL;
            }
          v->count--;
        }
      v->data[index] = x;
      v->count++;
      rv = x;
    }
  else
    {
      if (!_alloc_data(v, (index - v->length + 1)))
        goto cleanup;

      v->data[v->length - 1] = x;
      v->count++;
      rv = x;
    }

 cleanup:
  vector_mutex_unlock(&v->mutex);
  return rv;
}

int
vector_clear (Vector v)
{
  int rv = -1;

  assert(v);
  vector_mutex_lock(&v->mutex);
  assert(v->magic == VECTOR_MAGIC);

  rv = _clear_data(v);

  vector_mutex_unlock(&v->mutex);
  return rv;
}

void *
vector_get (Vector v, unsigned int index)
{
  void *rv = NULL;

  assert(v);
  vector_mutex_lock(&v->mutex);
  assert(v->magic == VECTOR_MAGIC);

  if (index >= v->length)
    {
      errno = EINVAL;
      goto cleanup;
    }
  rv = v->data[index];

 cleanup:
  vector_mutex_unlock(&v->mutex);
  return rv;
}

int
vector_find_first (Vector v, VectorFindF f, void *key)
{
  int i, rv = -1;

  assert(v);
  assert(f);
  assert(key);
  vector_mutex_lock(&v->mutex);
  assert(v->magic == VECTOR_MAGIC);

  for (i = 0; i < v->length; i++)
    {
      if (v->data[i])
        {
          if (f(v->data[i], key))
            {
              rv = i;
              break;
            }
        }
    }

  vector_mutex_unlock(&v->mutex);
  return(rv);
}

int
vector_for_each (Vector v, VectorForF f, void *arg)
{
  int i, rv = 0;

  assert(v);
  assert(f);
  vector_mutex_lock(&v->mutex);
  assert(v->magic == VECTOR_MAGIC);

  for (i = 0; i < v->length; i++)
    {
      if (v->data[i])
        {
          if (f(v->data[i], arg, i) < 0)
            {
              rv = -1;
              break;
            }
          rv++;
        }
    }

  vector_mutex_unlock(&v->mutex);
  return(rv);
}

int
vector_to_array (Vector v, void *p[], unsigned int len)
{
  int i, rv = -1;

  assert(v);
  assert(p);
  assert(len);
  vector_mutex_lock(&v->mutex);
  assert(v->magic == VECTOR_MAGIC);

  if (len < v->length)
    {
      errno = EINVAL;
      goto cleanup;
    }

  rv = 0;
  for (i = 0; i < v->length; i++)
    {
      p[i] = v->data[i];
      rv++;
    }

 cleanup:
  vector_mutex_unlock(&v->mutex);
  return(rv);
}

VectorIterator
vector_iterator_create (Vector v)
{
  struct vectorIterator *i;

  assert(v);
  vector_mutex_lock(&v->mutex);
  assert(v->magic == VECTOR_MAGIC);

  if (!(i = (struct vectorIterator *)malloc(sizeof(struct vectorIterator))))
    {
      errno = ENOMEM;
      return lsd_nomem_error(__FILE__, __LINE__, "vector iterator create");
    }
  memset(i, '\0', sizeof(struct vectorIterator));
  i->v = v;
  i->current = 0;
  assert(i->magic = VECTOR_ITERATOR_MAGIC); /* set magic via assert abuse */
  vector_mutex_unlock(&v->mutex);
  return(i);
}

void
vector_iterator_reset (VectorIterator i)
{
  assert(i);
  assert(i->magic == VECTOR_ITERATOR_MAGIC);
  vector_mutex_lock(&i->vector->mutex);
  assert(i->v->magic == VECTOR_MAGIC);
  i->current = 0;
  vector_mutex_unlock(&i->vector->mutex);
  return;
}


void
vector_iterator_destroy (VectorIterator i)
{
  assert(i);
  assert(i->magic == VECTOR_ITERATOR_MAGIC);
  assert(i->magic = ~VECTOR_MAGIC);     /* clear magic via assert abuse */
  free(i);
  return;
}


void *
vector_next (VectorIterator i)
{
  void *rv = NULL;

  assert(i);
  assert(i->magic == VECTOR_ITERATOR_MAGIC);
  vector_mutex_lock(&i->vector->mutex);
  assert(i->v->magic == VECTOR_MAGIC);
  if (i->current < i->v->length)
    {
      /* Skip any elements that are NULL */
      while ((!(i->v->data[i->current]))
             && i->current < i->v->length)
        i->current++;

      if (i->current < i->v->length)
        rv = i->v->data[i->current];
    }
  vector_mutex_unlock(&i->vector->mutex);
  return rv;
}


