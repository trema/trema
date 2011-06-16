/*
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


#include <linux/limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "wrapper.h"


// If this is being built for a unit test.
#ifdef UNIT_TESTING

/* Redirect free to test_free() so cmockery can check for memory
 * leaks. */
#ifdef free
#undef free
#endif // free
#define free( ptr ) _test_free( ptr, __FILE__, __LINE__ )
extern void _test_free( void *const ptr, const char *file, const int line );

#ifdef vasprintf
#undef vasprintf
#endif // vasprintf
#define vasprintf mock_vasprintf
extern int mock_vasprintf( char **strp, const char *fmt, va_list ap );

#else // UNIT_TESTING

#include "utility.h"


void *
xmalloc( size_t size ) {
  void *ret = malloc( size );

  if ( !ret ) {
    die( "Out of memory, malloc failed" );
  }
  memset( ret, 0xA5, size );
  return ret;
}


void *
xcalloc( size_t nmemb, size_t size ) {
  void *ret = calloc( nmemb, size );

  if ( !ret ) {
    die( "Out of memory, calloc failed" );
  }
  return ret;
}

#endif // UNIT_TESTING


void
xfree( void *ptr ) {
  free( ptr );
}


char *
xstrdup( const char *s ) {
  size_t len = strlen( s ) + 1;
  char *ret = xmalloc( len );

  memcpy( ret, s, len );
  return ret;
}


char *
xasprintf( const char *format, ... ) {
  char *str;

  va_list args;
  va_start( args, format );
  if ( vasprintf( &str, format, args ) < 0 ) {
    die( "Out of memory, vasprintf failed" );
  }
  va_end( args );

  return str;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
