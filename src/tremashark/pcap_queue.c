/*
 * tremashark: A bridge for printing Trema IPC messages on Wireshark
 * 
 * Author: Yasunori Nakazawa
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


#include <stdio.h>
#include <assert.h>
#include "linked_list.h"
#include "pcap_queue.h"
#include "wrapper.h"


#define QUEUE_LIMIT 4096


static list_element *packet_queue = NULL;
static uint16_t amount_of_packet = 0;


void
create_queue() {
  assert( packet_queue == NULL );
  assert( amount_of_packet == 0 );

  create_list( &packet_queue );
  return;
}


bool
delete_queue() {
  buffer *p;

  while ( amount_of_packet > 0 ) {
    pop_pcap_packet( &p );
    delete_pcap_packet( p );
  }

  delete_list( packet_queue );
  packet_queue = NULL;
  amount_of_packet = 0;

  return true;
}


buffer*
create_pcap_packet( void* pcap_header, size_t pcap_len,
               void* dump_header, size_t dump_len,
               void* data, size_t data_len ) {
  size_t length = pcap_len + dump_len + data_len;
  assert( length != 0 );
  assert( pcap_header != NULL && dump_header != NULL );

  buffer *buf = alloc_buffer_with_length( length );
  assert( buf != NULL );

  if ( pcap_header != NULL && pcap_len > 0 ) {
    void *d = append_back_buffer( buf, pcap_len );
    assert( d != NULL );
    memcpy( d, pcap_header, pcap_len );
  }

  if ( dump_header != NULL && dump_len > 0 ) {
    void *d = append_back_buffer( buf, dump_len );
    assert( d != NULL );
    memcpy( d, dump_header, dump_len );
  }

  if ( data != NULL && data_len > 0 ) {
    void *d = append_back_buffer( buf, data_len );
    assert( d != NULL );
    memcpy( d, data, data_len );
  }

  return buf;
}


bool
delete_pcap_packet( buffer *packet ) {
  assert( packet != NULL );
  
  free_buffer( packet );
  return true;
}


queue_return
push_pcap_packet( buffer *packet ) {
  if ( amount_of_packet >= QUEUE_LIMIT ) {
    return QUEUE_FULL;
  }

  append_to_tail( &packet_queue, packet );
  amount_of_packet++;

  return QUEUE_SUCCESS;
}


queue_return
push_pcap_packet_in_front( buffer *packet ) {
  if ( amount_of_packet >= QUEUE_LIMIT ) {
    return QUEUE_FULL;
  }

  insert_in_front( &packet_queue, packet );
  amount_of_packet++;
  
  return QUEUE_SUCCESS;
}


queue_return
pop_pcap_packet( buffer **packet ) {
  assert( packet != NULL );

  if ( packet_queue == NULL ) {
    return QUEUE_EMPTY;
  }

  *packet = packet_queue->data;
  delete_element( &packet_queue, *packet );
  amount_of_packet--;

  return QUEUE_SUCCESS;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
