/*
 * Wrappers that simplifies the handling of memory allocation errors.
 *
 * Copyright (C) 2008-2013 NEC Corporation
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


#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include "utility.h"


// If this is being built for a unit test.
#ifdef UNIT_TESTING

// Redirect assert to mock_assert() so assertions can be caught by cmockery.
#ifdef assert
#undef assert
#endif // assert
#define assert( expression )                                                \
  mock_assert( ( int ) ( expression ), #expression, __FILE__, __LINE__ );
extern void mock_assert( const int result, const char *const expression, const char *const file, const int line );

#endif // UNIT_TESTING


// Undef Ruby's xmalloc()
#ifdef xmalloc
#undef xmalloc
#endif
void *xmalloc( size_t size );

// Undef Ruby's xcalloc()
#ifdef xcalloc
#undef xcalloc
#endif
void *xcalloc( size_t nmemb, size_t size );

// Undef Ruby's xfree()
#ifdef xfree
#undef xfree
#endif
void xfree( void *ptr );

char *xstrdup( const char *s );
char *xvasprintf( const char *format, va_list args );
char *xasprintf( const char *format, ... );


#endif // WRAPPER_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
