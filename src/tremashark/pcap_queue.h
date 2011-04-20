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


#ifndef TREMASHARK_QUEUE_H
#define TREMASHARK_QUEUE_H


#include <stdint.h>
#include "buffer.h"

typedef enum queue_return {
  QUEUE_SUCCESS = 0,
  QUEUE_FULL,
  QUEUE_EMPTY
} queue_return;


void create_queue();
bool delete_queue();
buffer* create_pcap_packet( void* pcap_header, size_t pcap_len,
                       void* dump_header, size_t dump_len,
                       void* data, size_t data_len );
bool delete_pcap_packet( buffer *packet );
queue_return push_pcap_packet( buffer *packet );
queue_return push_pcap_packet_in_front( buffer *packet );
queue_return pop_pcap_packet( buffer **packet );


#endif // TREMASHARK_QUEUE_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
