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
#include "wrapper.h"
#include "message_queue.h"


message_queue *
create_message_queue( void ) {
  message_queue *new_queue = xmalloc( sizeof( message_queue ) );
  new_queue->head = xmalloc( sizeof( message_queue_element ) );
  new_queue->head->data = NULL;
  new_queue->head->next = NULL;
  new_queue->divider = new_queue->tail = new_queue->head;
  new_queue->length = 0;

  return new_queue;
}


bool
delete_message_queue( message_queue *queue ) {
  if ( queue == NULL ) {
    die( "queue must not be NULL" );
  }

  while ( queue->head != NULL ) {
    message_queue_element *element = queue->head;
    if ( queue->head->data != NULL ) {
      free_buffer( element->data );
    }
    queue->head = queue->head->next;
    xfree( element );
  }
  xfree( queue );

  return true;
}


static void
collect_garbage( message_queue *queue ) {
  while ( queue->head != queue->divider ) {
    message_queue_element *element = queue->head;
    queue->head = queue->head->next;
    xfree( element );
  }
}


bool
enqueue_message( message_queue *queue, buffer *message ) {
  if ( queue == NULL ) {
    die( "queues must not be NULL" );
  }
  if ( message == NULL ) {
    die( "message must not be NULL" );
  }

  message_queue_element *new_tail = xmalloc( sizeof( message_queue_element ) );
  new_tail->data = message;
  new_tail->next = NULL;

  queue->tail->next = new_tail;
  queue->tail = new_tail;
  queue->length++;

  collect_garbage( queue );

  return true;
}


buffer *
dequeue_message( message_queue *queue ) {
  if ( queue == NULL ) {
    die( "queue must not be NULL" );
  }
  if ( queue->divider == queue->tail ) {
    return NULL;
  }

  message_queue_element *next = queue->divider->next;
  buffer *message = next->data;
  next->data = NULL; // data must be freed by caller
  queue->divider = next;
  queue->length--;

  return message;
}


buffer *
peek_message( message_queue *queue ) {
  if ( queue == NULL ) {
    die( "queue must not be NULL" );
  }

  if ( queue->divider == queue->tail ) {
    return NULL;
  }

  return queue->divider->next->data;
}


void foreach_message_queue( message_queue *queue, bool function( buffer *message, void *user_data ), void *user_data ) {
  if ( queue->divider == queue->tail ) {
    return;
  }
  message_queue_element *element;
  for ( element = queue->divider->next; element != NULL; element = element->next ) {
    buffer *message = element->data;
    assert( message != NULL );
    if ( !function( message, user_data ) ) {
      break;
    }
  }
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
