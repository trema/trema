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
#include <pcap.h>
#include "pcap_private.h"
#include "pcap_queue.h"
#include "queue.h"
#include "trema.h"


static int QUEUE_LIMIT = 65536;
static queue *packet_queue = NULL;


bool
create_pcap_queue( void ) {
  assert( packet_queue == NULL );

  packet_queue = create_queue();
  if ( packet_queue == NULL ) {
    return false;
  }

  return true;
}


bool
delete_pcap_queue( void ) {
  assert( packet_queue != NULL );

  bool ret = delete_queue( packet_queue );
  if ( !ret ) {
    return false;
  }

  return true;
}


buffer *
create_pcap_packet( void *pcap_header, size_t pcap_len, void *dump_header, size_t dump_len, void *data, size_t data_len ) {
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


queue_status
enqueue_pcap_packet( buffer *packet ) {
  if ( packet_queue->length >= QUEUE_LIMIT ) {
    return QUEUE_FULL;
  }

  enqueue( packet_queue, packet );

  return QUEUE_SUCCESS;
}


queue_status
peek_pcap_packet( buffer **packet ) {
  assert( packet != NULL );

  if ( packet_queue == NULL ) {
    return QUEUE_EMPTY;
  }

  *packet = peek( packet_queue );
  if ( *packet == NULL ) {
    return QUEUE_EMPTY;
  }

  return QUEUE_SUCCESS;
}


queue_status
dequeue_pcap_packet( buffer **packet ) {
  assert( packet != NULL );

  if ( packet_queue == NULL ) {
    return QUEUE_EMPTY;
  }

  *packet = dequeue( packet_queue );
  if ( *packet == NULL ) {
    return QUEUE_EMPTY;
  }

  return QUEUE_SUCCESS;
}


static bool
compare_timestamp( const buffer *x, const buffer *y ) {
  struct pcap_pkthdr_private *px = x->data;
  struct pcap_pkthdr_private *py = y->data;

  if ( ( px->ts.tv_sec > py->ts.tv_sec ) ||
       ( px->ts.tv_sec == py->ts.tv_sec && px->ts.tv_usec > py->ts.tv_usec ) ) {
    return true;
  }

  return false;
}


bool
sort_pcap_queue( void ) {
  assert( packet_queue != NULL );

  return sort_queue( packet_queue, compare_timestamp );
}


void
set_max_pcap_queue_length( int length ) {
  assert( length >= 0 );

  QUEUE_LIMIT = length;
}


int
get_pcap_queue_length( void ) {
  assert( packet_queue != NULL );

  return packet_queue->length;
}


void
foreach_pcap_queue( void function( buffer *data ) ) {
  assert( packet_queue != NULL );

  foreach_queue( packet_queue, function );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
