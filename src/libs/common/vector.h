/*****************************************************************************
 *  $Id: vector.h,v 1.1.2.1 2006-10-30 00:58:34 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2005 The Regents of the University of California.
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
 *  with Genders; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 *****************************************************************************/


#ifndef VECTOR_H
#define VECTOR_H

/* Notes:
 * 
 * API emulates Chris Dunlap's (dunlap6 at llnl dot gov) List library.
 * Lots of code was copied.
 * 
 *  If NDEBUG is not defined, internal debug code will be enabled.  This is
 *  intended for development use only and production code should define NDEBUG.
 *
 *  If WITH_LSD_FATAL_ERROR_FUNC is defined, the linker will expect to
 *  find an external lsd_fatal_error(file,line,mesg) function.  By default,
 *  lsd_fatal_error(file,line,mesg) is a macro definition that outputs an
 *  error message to stderr.  This macro may be redefined to invoke another
 *  routine instead.
 *
 *  If WITH_LSD_NOMEM_ERROR_FUNC is defined, the linker will expect to
 *  find an external lsd_nomem_error(file,line,mesg) function.  By default,
 *  lsd_nomem_error(file,line,mesg) is a macro definition that returns NULL.
 *  This macro may be redefined to invoke another routine instead.
 *
 *  If WITH_PTHREADS is defined, these routines will be thread-safe.
 */

/***************************
 * Data Types              *
 ***************************/

/*
 *  Vector opaque data type
 */
typedef struct vector * Vector;

/*
 *  Vector Iterator opaque data type
 */
typedef struct vectorIterator * VectorIterator;

/*
 *  Function prototype to deallocate all data stored in a vector.
 */
typedef void (*VectorDelF) (void *x);

/*
 *  Function prototype for matching items in a vector.
 *  Returns non-zero if (x==key); o/w returns zero.
 */
typedef int (*VectorFindF) (void *x, void *key);

/*
 *  Function prototype for operating on each item in a vector.
 *  Returns less-than-zero on error.
 */
typedef int (*VectorForF) (void *x, void *arg, unsigned int index);

/***************************
 * General Functions       *
 ***************************/

/*
 * vector_create
 *
 * Create an empty vector.
 *
 * Returns Vector on success, calls lsd_nomem_error on error.
 */
Vector vector_create (VectorDelF f);

/*
 * vector_destroy
 *
 * Destroy the specified vector.
 *
 */
void vector_destroy (Vector v);

/*
 * vector_is_empty
 *
 * Returns 1 if vector is empty, 0 if not
 */
int vector_is_empty (Vector v);

/*
 * vector_length
 *
 * Note: Vector length is 1 larger than the largest index.
 *
 * Returns current vector length
 */
int vector_length (Vector v);

/*
 * vector_count
 *
 * Returns count of items stored in the vector
 */
int vector_count (Vector v);

/***************************
 * Vector Manipulation     *
 ***************************/

/* 
 * vector_append
 *
 * Append an item to the end of the vector, growing the vector
 * by a length of one.
 *
 * Returns data pointer on success, calls lsd_nomem_error on error
 */
void *vector_append (Vector v, void *x);

/* 
 * vector_set
 *
 * Set an item to a specific index location.  
 *
 * Returns data pointer on success, calls lsd_nomem_error on error
 */
void *vector_set(Vector v, void *x, unsigned int index);

/* 
 * vector_clear
 *
 * Remove all data in the vector.
 *
 * Returns number of data elements cleared on success
 */
int vector_clear(Vector v);

/***************************
 * Vector Access           *
 ***************************/

/* 
 * vector_get
 *
 * Return data stored at an index, NULL if item not set
 */
void *vector_get(Vector v, unsigned int index);

/* 
 * vector_find_first
 *
 * Returns the index to the first item that the specified
 * find function finds, -1 if not found
 */
int vector_find_first (Vector v, VectorFindF f, void *key);

/* 
 * vector_for_each
 *
 * Operate on each data element of the vector that is set.
 *
 * Returns number of data elements operated on
 */
int vector_for_each (Vector v, VectorForF f, void *arg);

/* 
 * vector_to_array
 *
 * Copy vector pointers to the supplied array.
 *
 * Returns number of data elements copied, -1 on invalid input
 */
int vector_to_array (Vector v, void *p[], unsigned int len);

/*****************************
 * Vector Iterator Functions *
 *****************************/

/* 
 * vector_iterator_create
 *
 * Create a vector iterator
 *
 * Returns iterator on success, NULL on error
 */
VectorIterator vector_iterator_create (Vector v);

/* 
 * vector_iterator_reset
 *
 * Resets the iterator
 */
void vector_iterator_reset (VectorIterator i);

/* 
 * vector_iterator_destroy
 *
 * Destroy the vector iteratory
 */
void vector_iterator_destroy (VectorIterator i);

/* 
 * vector_next
 *
 * Returns a pointer to the next item's data, NULL at the end of the
 * vector.
 *
 * Note: Iterates only on items set within the vector
 *
 * Example: i=vector_iterator_create(i); while ((x=vector_next(i))) {...}
 */
void *vector_next (VectorIterator i);

#endif /* !VECTOR_H */
