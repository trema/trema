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

/**
 * @file
 *
 * @brief Wrappers for basic library functions
 *
 * @code
 * // Memory allocation using malloc
 * new_entry = xmalloc( sizeof( match_entry ) );
 * // Memory allocation using calloc
 * packet_header_info *header_info = xcalloc( 1, sizeof( packet_header_info ) );
 * // Free allocated memory
 * xfree( trema_name );
 * // Duplicate string in memory
 * trema_name = xstrdup( name );
 * @endcode
 */

#include <linux/limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "trema_wrapper.h"
#include "utility.h"
#include "wrapper.h"


/**
 * Allocates a block of buffer. It is wrapped around by xmalloc.
 * @param size Bytes of buffer to be allocated
 * @param error_message Message to be displayed if error occurs
 * @return void* Pointer to allocated block of memory
 * @see xmalloc
 */
static void *
_trema_malloc( size_t size, const char *error_message ) {
  void *ret = trema_malloc( size );
  if ( !ret ) {
    die( error_message );
  }
  return ret;
}


/**
 * Allocates a buffer and initializes it.
 * @param size Bytes of memory to be allocated
 * @return void* Pointer to allocated block of memory
 */
void *
xmalloc( size_t size ) {
  void *ret = _trema_malloc( size, "Out of memory, xmalloc failed" );
  memset( ret, 0xA5, size );
  return ret;
}


/**
 * Allocates, and initializes a buffer to 0. Extension of trema_calloc
 * @param nmemb Number of memory blocks to be allocated
 * @param size Size of each memory block
 * @return void* Pointer to allocated block of memory
 */
void *
xcalloc( size_t nmemb, size_t size ) {
  void *ret = trema_calloc( nmemb, size );
  if ( !ret ) {
    die( "Out of memory, xcalloc failed" );
  }
  return ret;
}


/**
 * Frees an allocated buffer.
 * @param ptr Pointer to the buffer which is to be freed
 * @return None
 */
void
xfree( void *ptr ) {
  trema_free( ptr );
}


/**
 * Allocates and duplicates a string into memory. It is wrapped around by xstrdup.
 * @param s Pointer to constant string
 * @param error_message Message to be displayed if error occurs
 * @return char* Pointer to duplicated string
 * @see xstrdup
 */
static char *
_xstrdup( const char *s, const char *error_message ) {
  size_t len = strlen( s ) + 1;
  char *ret = _trema_malloc( len, error_message );
  memcpy( ret, s, len );
  return ret;
}


/**
 * Allocates and duplicates a string into memory. If sufficient memory is not
 * available, exits with an error.
 * @param s Pointer to constant string
 * @return char* Pointer to duplicated string
 */
char *
xstrdup( const char *s ) {
  return _xstrdup( s, "Out of memory, xstrdup failed" );
}


/**
 * Allocates a string large enough to hold the output including the terminating
 * null byte. If sufficient memory is not available, exits with an error.
 * @param format Pointer to constant string
 * @param ... Variable argument list
 * @return char* Pointer to string
 */
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
