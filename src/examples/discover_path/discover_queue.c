/*
 * Author: Hiroyasu OHYAMA
 *
 * Copyright (C) 2012 Univ. of tsukuba
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


#include <pthread.h>
#include "trema.h"

#include "discover_queue.h"
#include "ofnode.h"

static struct discover_queue_head qhead;

void
discover_queue_init() {
  create_list( &qhead.head );

  pthread_mutexattr_t attr;
  pthread_mutexattr_init( &attr );
  pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_ADAPTIVE_NP );

  qhead.mutex = xmalloc( sizeof( pthread_mutex_t ) );
  pthread_mutex_init( qhead.mutex, &attr );
}

void
discover_queue_destroy() {
  pthread_mutex_lock( qhead.mutex );

  list_element *elem = qhead.head;
  while( elem != NULL ) {
    list_element *next = elem->next;

    xfree( elem->data );

    elem = next;
  }
  delete_list( qhead.head );

  pthread_mutex_unlock( qhead.mutex );

  pthread_mutex_destroy( qhead.mutex );
  xfree( qhead.mutex );
}

void
discover_queue_push_switch( uint64_t target, uint64_t from, uint16_t in_port ) {

  discover_queue_entry *entry = xmalloc( sizeof( discover_queue_entry ) );
  entry->target_dpid = target;
  entry->type = TYPE_SWITCH;

  entry->from_dpid = from;
  entry->in_port = in_port;

  pthread_mutex_lock( qhead.mutex );

  insert_in_front( &qhead.head, entry );

  pthread_mutex_unlock( qhead.mutex );
}

void
discover_queue_push_host( uint64_t target, uint8_t *mac_addr, uint32_t nw_addr, uint16_t in_port ) {
  discover_queue_entry *entry = xmalloc( sizeof( discover_queue_entry ) );
  entry->target_dpid = target;
  entry->type = TYPE_HOST;

  entry->nw_addr = nw_addr;
  entry->in_port = in_port;
  memcpy( entry->mac_addr, mac_addr, ETH_ADDRLEN );

  pthread_mutex_lock( qhead.mutex );

  insert_in_front( &qhead.head, entry );

  pthread_mutex_unlock( qhead.mutex );
}

void
discover_queue_push( discover_queue_entry *entry ) {
  pthread_mutex_lock( qhead.mutex );

  if ( entry != NULL ) {
    insert_in_front( &qhead.head, entry );
  }

  pthread_mutex_unlock( qhead.mutex );
}

discover_queue_entry *
discover_queue_pop() {
  discover_queue_entry *entry = NULL;

  pthread_mutex_lock( qhead.mutex );

  list_element *elem = qhead.head;
  if ( elem != NULL ) {
    entry = ( discover_queue_entry * ) elem->data;

    delete_element( &qhead.head, entry );
  }

  pthread_mutex_unlock( qhead.mutex );

  return entry;
}
