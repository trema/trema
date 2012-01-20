/*
 * Author: Yasunobu Chiba
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


#include <assert.h>
#include "message_queue.h"


message_queue *
create_message_queue( void ) {
  message_queue *queue = xmalloc( sizeof( message_queue ) );
  create_list( &queue->head );
  queue->tail = NULL;
  queue->length = 0;

  return queue;
}


bool
delete_message_queue( message_queue *queue ) {
  assert( queue != NULL );

  list_element *element = queue->head;
  while( element != NULL ) {
    free_buffer( element->data );
    element = element->next;
  }
  delete_list( queue->head );
  xfree( queue );

  return true;
}


bool
enqueue_message( message_queue *queue, buffer *message ) {
  assert( queue != NULL );
  assert( message != NULL );
  assert( message->length > 0 );

  list_element *new_tail = xmalloc( sizeof( list_element ) );
  new_tail->data = message;
  new_tail->next = NULL;

  queue->length++;

  if ( queue->head == NULL ) {
    queue->head = new_tail;
    queue->tail = new_tail;

    return true;
  }

  queue->tail->next = new_tail;
  queue->tail = new_tail;

  return true;
}


buffer *
dequeue_message( message_queue *queue ) {
  assert( queue != NULL );

  if ( queue->head == NULL ) {
    return NULL;
  }

  list_element *delete_me = queue->head;
  buffer *message = delete_me->data;
  queue->head = queue->head->next;
  xfree( delete_me );

  if ( queue->head == NULL ) {
    queue->tail = NULL;
  }

  queue->length--;

  return message;
}


buffer *
peek_message( message_queue *queue ) {
  assert( queue != NULL );

  if ( queue->head == NULL ) {
    return NULL;
  }

  return queue->head->data;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
