/*
 * Author: Shin-ya Zenke
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
 * @brief Buffer management implementation
 *
 * File containing functions for buffer management, including allocation,
 * deallocation.
 * @code
 * // Allocates buffer type structure and assigns it length bytes of space
 * body_h = alloc_buffer_with_length( body_length );
 * ...
 * // Adds free space in front of already allocated buffer
 * data = append_front_buffer( buffer, header_length );
 * ...
 * // Duplicates buffer
 * body = duplicate_buffer( data );
 * ...
 * // Removes some space from the top part of an already allocated buffer
 * remove_front_buffer( body, offsetof( struct ofp_error_msg, data ) );
 * ...
 * // Releases the allocated buffer
 * free_buffer( body );
 * @endcode
 */

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#include "checks.h"
#include "utility.h"
#include "wrapper.h"


/**
 * Defines an internal structure that is being used within the buffer.c file for handling
 * buffer allocation and management. Buffer being used by external functions (through buffer type) 
 * are assigned to a member element and support for locking is provided on it through this type. 
 * Design of this type is such to embed the externally visible buffer into a management layer. For
 * applications, this type is never directly accessed.
 * @see buffer
 */ 
typedef struct private_buffer {
  buffer public; /*!<Externally visible buffer is embedded to this member*/
  size_t real_length; /*!<True length of allocated buffer */
  void *top; /*!<Pointer to the head of user data area. only valid if public.data is allocated.*/
  pthread_mutex_t *mutex; /*!<mutual exclusion support for buffer access/modification*/
} private_buffer;


/**
 * Finds and returns the length of buffer which has already been consumed.
 * @param pbuf Pointer to private buffer structure which holds the buffer structure, which in turn points to allocated data
 * @return size_t Length of the consumed part of the buffer
 */
static size_t
front_length_of( const private_buffer *pbuf ) {
  assert( pbuf != NULL );

  return ( size_t ) ( ( char * ) pbuf->public.data - ( char * ) pbuf->top );
}


/**
 * Checks if the buffer has already been allocated enough space to work in.
 * @param pbuf Private buffer type structure containing the allocated entry and length allocated
 * @param length Addition length of data to be accommodated in this buffer
 * @return bool True if allocated length of buffer is sufficient to accommodate required length, else False
 */
static bool
already_allocated( private_buffer *pbuf, size_t length ) {
  assert( pbuf != NULL );

  size_t required_length = ( size_t ) front_length_of( pbuf ) + pbuf->public.length + length;

  return ( pbuf->real_length >= required_length );
}


/**
 * Allocates fresh buffer space.
 * @param pbuf Private buffer type structure representing this new allocation
 * @param length Absolute length of the expected buffer
 * @return private_buffer Pointer to the updated pbuf argument with allocation done
 */
static private_buffer *
alloc_new_data( private_buffer *pbuf, size_t length ) {
  assert( pbuf != NULL );

  pbuf->public.data = xmalloc( length );
  pbuf->public.length = length;
  pbuf->top = pbuf->public.data;
  pbuf->real_length = length;

  return pbuf;
}


/**
 * Allocates an empty private_buffer type, and initializes its members to 0 or
 * NULL (as per the case) before returning.
 * @param None
 * @return private_buffer Pointer to the newly allocated private_buffer type structure, members initialized to 0/NULL
 */
static private_buffer *
alloc_private_buffer() {
  private_buffer *new_buf = xcalloc( 1, sizeof( private_buffer ) );

  new_buf->public.data = NULL;
  new_buf->public.length = 0;
  new_buf->public.user_data = NULL;
  new_buf->public.user_data_free_function = NULL;
  new_buf->top = NULL;
  new_buf->real_length = 0;

  pthread_mutexattr_t attr;
  pthread_mutexattr_init( &attr );
  pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE_NP );
  new_buf->mutex = xmalloc( sizeof( pthread_mutex_t ) );
  pthread_mutex_init( new_buf->mutex, &attr );

  return new_buf;
}


/**
 * Adds/Appends more data space at the front side of an already allocated
 * buffer. It is wrapped around by append_front_buffer.
 * @param pbuf Pointer to private_buffer type structure which represents the old buffer to be appended
 * @param length Addition length of data to be appended at front of the present buffer
 * @return private_buffer Pointer to the updated private_buffer type structure, containing front appended buffer
 * @see append_front_buffer
 * @see append_back
 */
static private_buffer *
append_front( private_buffer *pbuf, size_t length ) {
  assert( pbuf != NULL );

  void *new_data = xmalloc( front_length_of( pbuf ) + pbuf->public.length + length );
  memcpy( ( char * ) new_data + front_length_of( pbuf ) + length, pbuf->public.data, pbuf->public.length );
  xfree( pbuf->top );

  pbuf->public.data = ( char * ) new_data + front_length_of( pbuf );
  pbuf->real_length = sizeof( new_data );
  pbuf->top = new_data;

  return pbuf;
}


/**
 * Adds/Appends more data space at the back end of an already allocated buffer.
 * It is wrapped around by append_back_buffer.
 * @param pbuf Pointer to private_buffer type structure which represents the old buffer to be appended
 * @param length Addition length of data to be appended at back of the present buffer
 * @return private_buffer Pointer to the updated private_buffer type structure, containing back appended buffer
 * @see append_back_buffer
 * @see append_front
 */
static private_buffer *
append_back( private_buffer *pbuf, size_t length ) {
  assert( pbuf != NULL );

  void *new_data = xmalloc( front_length_of( pbuf ) + pbuf->public.length + length );
  memcpy( ( char * ) new_data + front_length_of( pbuf ), pbuf->public.data, pbuf->public.length );
  xfree( pbuf->top );

  pbuf->public.data = ( char * ) new_data + front_length_of( pbuf );
  pbuf->real_length = sizeof( new_data );
  pbuf->top = new_data;

  return pbuf;
}


/**
 * Allocates an empty buffer type structure which can be used for representing
 * allocated area.
 * @param None
 * @return buffer Pointer to buffer type, which is embedded into private_buffer type
 */
buffer *
alloc_buffer() {
  return ( buffer * ) alloc_private_buffer();
}


/**
 * Allocates buffer type structure and assigns it length bytes of space. It
 * initializes all internal data members.
 * @param length Length of allocated buffer requested
 * @return buffer Pointer to buffer type which holds the allocated area, and its members appropriately initialized
 */
buffer *
alloc_buffer_with_length( size_t length ) {
  assert( length != 0 );

  private_buffer *new_buf = alloc_private_buffer();
  new_buf->public.data = xmalloc( length );
  new_buf->top = new_buf->public.data;
  new_buf->real_length = length;

  return ( buffer * ) new_buf;
}


/**
 * Releases the allocated buffer as well as the private_buffer structure which
 * was allocated to represent this buffer.
 * @param buf Pointer to buffer type, which is internally mapped to private_buffer type
 * @return None
 */
void
free_buffer( buffer *buf ) {
  assert( buf != NULL );

  if ( buf->user_data != NULL && buf->user_data_free_function != NULL ) {
    ( *buf->user_data_free_function )( buf );
    assert( buf->user_data == NULL );
    assert( buf->user_data_free_function == NULL );
  }
  pthread_mutex_lock( ( ( private_buffer * ) buf )->mutex );
  private_buffer *delete_me = ( private_buffer * ) buf;
  if ( delete_me->top != NULL ) {
    xfree( delete_me->top );
  }
  pthread_mutex_unlock( delete_me->mutex );
  pthread_mutex_destroy( delete_me->mutex );
  xfree( delete_me->mutex );
  xfree( delete_me );
}


/**
 * Adds free space in front of already allocated buffer.
 * @param buf Pointer to buffer type to which extra space has to be allocated
 * @param length Length of the extra buffer to be appended
 * @return void* Pointer to allocated space
 * @see append_front
 */
void *
append_front_buffer( buffer *buf, size_t length ) {
  assert( buf != NULL );
  assert( length != 0 );

  pthread_mutex_lock( ( ( private_buffer * ) buf )->mutex );

  private_buffer *pbuf = ( private_buffer * ) buf;

  if ( pbuf->top == NULL ) {
    alloc_new_data( pbuf, length );
    pthread_mutex_unlock( pbuf->mutex );
    return pbuf->public.data;
  }

  buffer *b = &( pbuf->public );
  if ( already_allocated( pbuf, length ) ) {
    memmove( ( char * ) b->data + length, b->data, b->length );
    memset( b->data, 0, length );
  } else {
    append_front( pbuf, length );
  }
  b->length += length;

  pthread_mutex_unlock( pbuf->mutex );

  return b->data;
}


/**
 * Removes some space from the top part of an already allocated buffer.
 * @param buf Pointer to buffer type which holds the allocated buffers its length
 * @param length Length of space to remove from top of the allocated buffer
 * @return void* Updated pointer to allocated data after removing top length bytes
 */
void *
remove_front_buffer( buffer *buf, size_t length ) {
  assert( buf != NULL );
  assert( length != 0 );

  pthread_mutex_lock( ( ( private_buffer * ) buf )->mutex );

  private_buffer *pbuf = ( private_buffer * ) buf;
  assert( pbuf->public.length >= length );

  pbuf->public.data = ( char * ) pbuf->public.data + length;
  pbuf->public.length -= length;

  pthread_mutex_unlock( pbuf->mutex );

  return pbuf->public.data;
}


/**
 * Adds free space at the back of already allocated buffer.
 * @param buf Pointer to buffer type to which extra space has to be allocated
 * @param length Length of the extra buffer to be appended
 * @return void* Pointer to allocated buffer which appended space
 * @see append_back
 */
void *
append_back_buffer( buffer *buf, size_t length ) {
  assert( buf != NULL );
  assert( length != 0 );

  pthread_mutex_lock( ( ( private_buffer * ) buf )->mutex );

  private_buffer *pbuf = ( private_buffer * ) buf;

  if ( pbuf->real_length == 0 ) {
    alloc_new_data( pbuf, length );
    pthread_mutex_unlock( pbuf->mutex );
    return ( char * ) pbuf->public.data;
  }
 
  if ( !already_allocated( pbuf, length ) ) {
    append_back( pbuf, length );
  }

  void *appended = ( char * ) pbuf->public.data + pbuf->public.length;
  pbuf->public.length += length;

  pthread_mutex_unlock( pbuf->mutex );

  return appended;
}


/**
 * Makes exact replica of the buffer type passed as argument, includes copying
 * of the data and initializing the buffer type members.
 * @param buf Pointer to buffer type which holds the allocated space and other members to manage this space
 * @return buffer* Pointer to freshly allocated buffer type which is replica of argument passed
 */
buffer *
duplicate_buffer( const buffer *buf ) {
  assert( buf != NULL );

  pthread_mutex_lock( ( ( const private_buffer * ) buf )->mutex );

  private_buffer *new_buffer = alloc_private_buffer();
  const private_buffer *old_buffer = ( const private_buffer * ) buf;

  if ( old_buffer->real_length == 0 ) {
    pthread_mutex_unlock( old_buffer->mutex );
    return ( buffer * ) new_buffer;
  }

  alloc_new_data( new_buffer, old_buffer->real_length );
  memcpy( new_buffer->top, old_buffer->top, old_buffer->real_length );

  new_buffer->public.length = old_buffer->public.length;
  new_buffer->public.user_data = old_buffer->public.user_data;
  new_buffer->public.user_data_free_function = NULL;
  new_buffer->public.data = ( char * ) ( new_buffer->public.data ) + front_length_of( old_buffer );

  pthread_mutex_unlock( old_buffer->mutex );

  return ( buffer * ) new_buffer;
}


/**
 * Provides pluggable method for printing/dumping buffer onto a I/O stream
 * (like terminal). It can accept as argument a function pointer which defines
 * the method for handling the I/O stream.
 * @param buf Pointer to buffer type which needs to be printed/dumped
 * @param dump_function Function pointer to a custom I/O stream writing routine
 * @return None
 */
void
dump_buffer( const buffer *buf, void dump_function( const char *format, ... ) ) {
  assert( dump_function != NULL );

  pthread_mutex_lock( ( ( const private_buffer * ) buf )->mutex );

  char *hex = xmalloc( sizeof( char ) * ( buf->length * 2 + 1 ) );
  char *datap = buf->data;
  char *hexp = hex;
  for ( unsigned int i = 0; i < buf->length; i++, datap++, hexp += 2 ) {
    snprintf( hexp, 3, "%02x", *datap );
  }
  ( *dump_function )( "%s", hex );

  xfree( hex );

  pthread_mutex_unlock( ( ( const private_buffer * ) buf )->mutex );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
