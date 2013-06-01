/*
 * Queue implementation.
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


#ifndef QUEUE_H
#define QUEUE_H


#include "trema.h"


typedef struct queue_element {
  buffer *data;
  struct queue_element *next;
} queue_element;

typedef struct queue {
  queue_element *head;
  queue_element *divider;
  queue_element *tail;
  volatile int length;
} queue;


queue *create_queue( void );
bool delete_queue( queue *queue );
bool enqueue( queue *queue, buffer *data );
buffer *dequeue( queue *queue );
buffer *peek( queue *queue );
bool sort_queue( queue *queue, bool compare( const buffer *x, const buffer *y ) );
void foreach_queue( queue *queue, void function( buffer *data ) );


#endif // QUEUE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
