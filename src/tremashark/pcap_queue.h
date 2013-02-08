/*
 * Queue implementation for keeping pcap formatted packets.
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


#ifndef PCAP_QUEUE_H
#define PCAP_QUEUE_H


#include "trema.h"


typedef enum {
  QUEUE_SUCCESS,
  QUEUE_FULL,
  QUEUE_EMPTY
} queue_status;


bool create_pcap_queue( void );
bool delete_pcap_queue( void );
buffer *create_pcap_packet( void *pcap_header, size_t pcap_len, void *dump_header, size_t dump_len, void *data, size_t data_len );
bool delete_pcap_packet( buffer *packet );
queue_status enqueue_pcap_packet( buffer *packet );
queue_status peek_pcap_packet( buffer **packet );
queue_status dequeue_pcap_packet( buffer **packet );
bool sort_pcap_queue( void );
void set_max_pcap_queue_length( int length );
int get_pcap_queue_length( void );
void foreach_pcap_queue( void function( buffer *data ) );


#endif // PCAP_QUEUE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
