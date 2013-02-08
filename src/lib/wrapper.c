/*
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


#include <linux/limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "trema_wrapper.h"
#include "utility.h"
#include "wrapper.h"


static void *
_trema_malloc( size_t size, const char *error_message ) {
  void *ret = trema_malloc( size );
  if ( !ret ) {
    die( error_message );
  }
  return ret;
}


void *
xmalloc( size_t size ) {
  void *ret = _trema_malloc( size, "Out of memory, xmalloc failed" );
  memset( ret, 0xA5, size );
  return ret;
}


void *
xcalloc( size_t nmemb, size_t size ) {
  void *ret = trema_calloc( nmemb, size );
  if ( !ret ) {
    die( "Out of memory, xcalloc failed" );
  }
  return ret;
}


void
xfree( void *ptr ) {
  trema_free( ptr );
}


static char *
_xstrdup( const char *s, const char *error_message ) {
  size_t len = strlen( s ) + 1;
  char *ret = _trema_malloc( len, error_message );
  memcpy( ret, s, len );
  return ret;
}


char *
xstrdup( const char *s ) {
  return _xstrdup( s, "Out of memory, xstrdup failed" );
}


static char *
_xvasprintf( const char *format, va_list args, const char *error_message ) {
  va_list args_copy;
  va_copy( args_copy, args );
  int n = vsnprintf( NULL, 0, format, args_copy );
  va_end( args_copy );
  if ( n <= 0 ) {
    return NULL;
  }
  size_t size = ( size_t ) ( n + 1 );
  char *str = _trema_malloc( size, error_message );
  va_copy( args_copy, args );
  n = vsnprintf( str, size, format, args_copy );
  va_end( args_copy );
  if ( n <= 0 || ( size_t ) n > size ) {
    xfree( str );
    return NULL;
  }
  return str;
}


char *
xvasprintf( const char *format, va_list args ) {
  return _xvasprintf( format, args, "Out of memory, xvasprintf failed" );
}


char *
xasprintf( const char *format, ... ) {
  const char error[] = "Out of memory, xasprintf failed";
  va_list args;
  va_start( args, format );
  char *str = _xvasprintf( format, args, error );
  va_end( args );
  return str;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
