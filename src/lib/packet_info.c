/*
 * Author: Naoyoshi Tada
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

/**
 * @file packet_info.c
 * Source file containing functions for handling packet header information. The
 * packets can be of any of the following type : Ethernet packet, ARP packet,
 * IPv4 packet, TCP packet, UDP packet, ICMP packet.
 */

#include <assert.h>
#include "packet_info.h"
#include "wrapper.h"


/**
 * Releases the memory allocated to structure of type packet_header_info which
 * contains packet header information. Pointer to this structure is stored in
 * user_data element of buffer type structure.
 * @param buf Pointer to buffer type structure
 * @return None
 */
void
free_packet_info( buffer *buf ) {
  assert( buf != NULL );
  assert( buf->user_data != NULL );

  xfree( buf->user_data );
  buf->user_data = NULL;
  buf->user_data_free_function = NULL;
}


/**
 * Allocates memory to structure of type packet_header_info which contains
 * packet header information and initializes its elements to either NULL or 0.
 * Also, user_data element of buffer type structure is initialized to pointer
 * to this structure.
 * @param buf Pointer to buffer type structure
 * @return None
 */
void
alloc_packet_info( buffer *buf ) {
  assert( buf != NULL );

  packet_info *packet_info = xcalloc( 1, sizeof( packet_info ) );
  assert( packet_info != NULL );

  memset( packet_info, 0, sizeof( packet_info ) );

  buf->user_data = packet_info;
  buf->user_data_free_function = free_packet_info;
}


/**
 * This function is deprecated.
 *
 * Releases the memory allocated to structure of type buffer and also to
 * structure of type packet_header_info, pointer to which is contained in
 * user_data element of this buffer type structure.
 * @param buf Pointer to buffer type structure
 * @return None
 */
void
free_packet( buffer *buf ) {
  assert( buf != NULL );

  free_packet_info( buf );
  free_buffer( buf );
}

/**
 *
 */
packet_info 
get_packet_info( const buffer *frame ) {
  assert( frame != NULL );
  
  return *( packet_info *)frame->user_data;
}

/**
 *
 */



/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
