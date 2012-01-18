/*
 * Queue implementation
 *
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


#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H


#include "trema.h"


typedef struct {
  list_element *head;
  list_element *tail;
  int length;
} message_queue;


message_queue *create_message_queue( void );
bool delete_message_queue( message_queue *queue );
bool enqueue_message( message_queue *queue, buffer *message );
buffer *dequeue_message( message_queue *queue );
buffer *peek_message( message_queue *queue );


#endif // MESSAGE_QUEUE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
