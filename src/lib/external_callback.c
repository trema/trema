/*
 * Copyright (C) 2013 NEC Corporation
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


#include <assert.h>
#include "external_callback.h"
#include "log.h"
#include "wrapper.h"

#ifdef UNIT_TESTING

#ifdef xcalloc
#undef xcalloc
#endif
#define xcalloc mock_xcalloc
extern void *mock_xcalloc( size_t nmemb, size_t size );

#ifdef xfree
#undef xfree
#endif
#define xfree mock_xfree
extern void mock_xfree( void *ptr );

#ifdef debug
#undef debug
#endif
#define debug mock_debug
extern void mock_debug( const char *format, ... );

#define static

#endif // UNIT_TESTING


typedef struct {
  external_callback_t *buffer;
  unsigned int size;
  unsigned int write_position;
  unsigned int read_position;
} ring_buffer;

static const unsigned int MAX_EXTERNAL_CALLBACK = 16;
static ring_buffer *external_callbacks = NULL;


static void
_init_external_callback() {
  assert( external_callbacks == NULL );

  external_callbacks = xcalloc( 1, sizeof( ring_buffer ) );
  external_callbacks->buffer = xcalloc( MAX_EXTERNAL_CALLBACK, sizeof( external_callback_t * ) );
  external_callbacks->size = MAX_EXTERNAL_CALLBACK;
}
void ( *init_external_callback )() = _init_external_callback;


static void
_finalize_external_callback() {
  assert( external_callbacks != NULL );
  assert( external_callbacks->buffer != NULL );

  xfree( external_callbacks->buffer );
  xfree( external_callbacks );
  external_callbacks = NULL;
}
void ( *finalize_external_callback )() = _finalize_external_callback;


static bool
_push_external_callback( external_callback_t callback ) {
  if ( callback == NULL ) {
    return false;
  }
  if ( external_callbacks == NULL ) {
    return false;
  }
  assert( external_callbacks->buffer != NULL );

  debug( "push_external_callback: %p", callback );
  unsigned int write_pos = __sync_fetch_and_add( &external_callbacks->write_position, 1 ) % external_callbacks->size;
  assert( __sync_bool_compare_and_swap( &( external_callbacks->buffer[ write_pos ] ), NULL, callback ) );

  return true;
}
bool ( *push_external_callback )( external_callback_t callback ) = _push_external_callback;


static void
_run_external_callback() {
  assert( external_callbacks != NULL );
  assert( external_callbacks->buffer != NULL );

  while ( external_callbacks->write_position != external_callbacks->read_position ) {
    unsigned int read_pos = external_callbacks->read_position % external_callbacks->size;
    external_callback_t callback = external_callbacks->buffer[ read_pos ];
    assert( callback != NULL );
    external_callbacks->buffer[ read_pos ] = NULL;
    external_callbacks->read_position++;
    debug( "run_external_callback: %p ( write_position = %u, read_position = %u )", callback,
           external_callbacks->write_position, external_callbacks->read_position );
    callback();
  }
}
void ( *run_external_callback )() = _run_external_callback;


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
