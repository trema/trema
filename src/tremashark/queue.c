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


#include <assert.h>
#include "queue.h"


queue *
create_queue( void ) {
  queue *new_queue = xmalloc( sizeof( queue ) );
  new_queue->head = xmalloc( sizeof( queue_element ) );
  new_queue->head->data = NULL;
  new_queue->head->next = NULL;
  new_queue->divider = new_queue->tail = new_queue->head;
  new_queue->length = 0;

  return new_queue;
}


bool
delete_queue( queue *queue ) {
  assert( queue != NULL );

  while ( queue->head != NULL ) {
    queue_element *e = queue->head;
    if ( queue->head->data != NULL ) {
      free_buffer( queue->head->data );
    }
    queue->head = queue->head->next;
    xfree( e );
  }
  xfree( queue );

  return true;
}


static void
collect_garbage( queue *queue ) {
  while ( queue->head != queue->divider ) {
    queue_element *e = queue->head;
    queue->head = queue->head->next;
    xfree( e );
  }
}


bool
enqueue( queue *queue, buffer *data ) {
  assert( queue != NULL );
  assert( data != NULL );
  assert( data->length > 0 );

  queue_element *new_tail = xmalloc( sizeof( queue_element ) );
  new_tail->data = data;
  new_tail->next = NULL;

  queue->tail->next = new_tail;
  queue->tail = new_tail;
  __sync_add_and_fetch( &queue->length, 1 ); // this must be an atomic operation for thread safety

  collect_garbage( queue );

  return true;
}


buffer *
dequeue( queue *queue ) {
  assert( queue != NULL );

  if ( queue->divider != queue->tail ) {
    queue_element *next = queue->divider->next;
    buffer *data = next->data;
    next->data = NULL; // data must be freed by caller
    queue->divider = next;
    __sync_sub_and_fetch( &queue->length, 1 ); // this must be an atomic operation for thread safety

    return data;
  }

  return NULL;
}


buffer *
peek( queue *queue ) {
  assert( queue != NULL );

  if ( queue->divider != queue->tail ) {
    return queue->divider->next->data;
  }

  return NULL;
}


bool
sort_queue( queue *queue, bool compare( const buffer *x, const buffer *y ) ) {
  assert( queue != NULL );

  collect_garbage( queue );

  const int length = queue->length;
  queue_element *elements[ length ];

  int i = 0;
  queue_element *element = queue->head;
  while ( element->next != NULL ) {
    elements[ i ] = element->next;
    element = element->next;
    i++;
  }

  int j = 0;
  buffer *data;
  for ( int i = 1; i < length; i++ ) {
    data = elements[ i ]->data;
    if ( compare( elements[ i - 1 ]->data, data ) ) {
      j = i;
      do {
        elements[ j ]->data = elements[ j - 1 ]->data;
        j--;
      }
      while ( j > 0 && compare( elements[ j - 1 ]->data, data ) );
      elements[ j ]->data = data;
    }
  }

  return true;
}


void
foreach_queue( queue *queue, void function( buffer *data ) ) {
  assert( queue != NULL );

  queue_element *e = queue->divider;
  while ( e != queue->tail ) {
    function( e->next->data );
    e = e->next;
  }
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
