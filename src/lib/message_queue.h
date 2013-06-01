/*
 * Queue implementation
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


#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H


#include "bool.h"
#include "buffer.h"


typedef struct message_queue_element {
  buffer *data;
  struct message_queue_element *next;
} message_queue_element;


typedef struct {
  message_queue_element *head;
  message_queue_element *divider;
  message_queue_element *tail;
  unsigned int length;
} message_queue;


message_queue *create_message_queue( void );
bool delete_message_queue( message_queue *queue );
bool enqueue_message( message_queue *queue, buffer *message );
buffer *dequeue_message( message_queue *queue );
buffer *peek_message( message_queue *queue );
void foreach_message_queue( message_queue *queue, bool function( buffer *message, void *user_data ), void *user_data );


#endif // MESSAGE_QUEUE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
