/*
 * Author: Yasuhito Takamiya <yasuhito@gmail.com>
 *
 * Copyright (C) 2008-2012 NEC Corporation
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


char *
xasprintf( const char *format, ... ) {
  const char error[] = "Out of memory, xasprintf failed";
  va_list args;
  va_start( args, format );
  char *str;
  if ( trema_vasprintf( &str, format, args ) < 0 ) {
    die( error );
  }
  char *result = _xstrdup( str, error );
  free( str );
  va_end( args );
  return result;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
