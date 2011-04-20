/*
 * Wrappers that simplifies the handling of memory allocation errors.
 *
 * Author: Yasuhito Takamiya <yasuhito@gmail.com>
 *
 * Copyright (C) 2008-2011 NEC Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#ifndef WRAPPER_H
#define WRAPPER_H


#include <stddef.h>
#include <string.h>
#include "utility.h"


// If this is being built for a unit test.
#ifdef UNIT_TESTING

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define xmalloc( _size )                                                    \
  ( {                                                                       \
    void *_ret = _test_malloc( ( _size ), __FILE__, __LINE__ );             \
    if ( !_ret ) {                                                          \
      fprintf( stderr, "Out of memory, malloc failed" );                    \
      exit( EXIT_FAILURE );                                                 \
    }                                                                       \
    memset( _ret, 0xA5, ( _size ) );                                        \
    _ret;                                                                   \
  } )
extern void *_test_malloc( const size_t size, const char *file, const int line );

#define xcalloc( _nmemb, _size )                                            \
  ( {                                                                       \
    void *_ret = _test_calloc( ( _nmemb ), ( _size ), __FILE__, __LINE__ ); \
    if ( !_ret ) {                                                          \
      fprintf( stderr, "Out of memory, calloc failed" );                    \
      exit( EXIT_FAILURE );                                                 \
    }                                                                       \
    _ret;                                                                   \
  } )
extern void *_test_calloc( const size_t number_of_elements, const size_t size, const char *file, const int line );

// Redirect assert to mock_assert() so assertions can be caught by cmockery.
#ifdef assert
#undef assert
#endif // assert
#define assert( expression )                                                \
  mock_assert( ( int ) ( expression ), #expression, __FILE__, __LINE__ );
extern void mock_assert( const int result, const char *const expression, const char *const file, const int line );

#else // UNIT_TESTING

#ifdef xmalloc
#undef xmalloc
#endif // xmalloc
void *xmalloc( size_t size );

#ifdef xcalloc
#undef xcalloc
#endif // xcalloc
void *xcalloc( size_t nmemb, size_t size );

#endif // UNIT_TESTING


void xfree( void *ptr );
char *xstrdup( const char *s );


#endif // WRAPPER_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
