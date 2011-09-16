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
 * @file
 *
 * @brief Packet Header Information handling implementation
 *
 * File containing functions for handling packet header information. The
 * packets can be of any of the following type : Ethernet packet, ARP packet,
 * IPv4 packet, TCP packet, UDP packet, ICMP packet.
 * @code
 * // Allocates memory to structure which contains packet header information
 * alloc_packet( buf );
 * ...
 * // Parse an Ethernet frame stored in eth_frame. Now the frame is
 * // parsed and the results are stored in a newly allocated memory area.
 * parse_packet( eth_frame );
 * // Now you can refer to the header field values like follows.
 * switch ( packet_info( eth_frame )->ethtype ) {
 * case ETH_ETHTYPE_IPV4:
 * ...
 * case ETH_ETHTYPE_ARP:
 * ...
 * // Finally free the buffer.
 * free_buffer( eth_frame );
 * @endcode
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
static void
free_packet_header_info( buffer *buf ) {
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
alloc_packet( buffer *buf ) {
  assert( buf != NULL );

  packet_header_info *header_info = xcalloc( 1, sizeof( packet_header_info ) );
  assert( header_info != NULL );

  header_info->ethtype = 0;
  header_info->nvtags = 0;
  header_info->ipproto = 0;
  header_info->l2_data.l2 = NULL;
  header_info->vtag = NULL;
  header_info->l3_data.l3 = NULL;
  header_info->l4_data.l4 = NULL;
  buf->user_data = header_info;
  buf->user_data_free_function = free_packet_header_info;
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

  free_packet_header_info( buf );
  free_buffer( buf );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
