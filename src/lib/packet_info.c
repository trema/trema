/*
 * Author: Kazuya Suzuki, Naoyoshi Tada
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
#include "trema.h"


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
calloc_packet_info( buffer *buf ) {
  assert( buf != NULL );

  void *user_data = xcalloc( 1, sizeof( packet_info ) );
  assert( user_data != NULL );

  memset( user_data, 0, sizeof( packet_info ) );

  buf->user_data = user_data;
  buf->user_data_free_function = free_packet_info;
}


/**
 * Return packet information stored in user_data element of
 * buffer type structure.
 * @param frame Pointer to buffer type structure
 * @return packet_info Packet information stored in frame
 */
packet_info 
get_packet_info( const buffer *frame ) {
  assert( frame != NULL );

  packet_info info;
  
  if ( frame->user_data != NULL ) {
    info = *( packet_info *)frame->user_data;
  } else {
    memset( &info, 0, sizeof( info ) );
  }
  
  return info;
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
 * Checks whether packet type is dix or not.
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is dix
 */
static bool
if_packet_type( const buffer *frame, uint32_t type ) {
  assert( frame != NULL );
  assert( frame->user_data != NULL );

  packet_info *packet_info0 = ( packet_info * )frame->user_data;
  
  return ( ( packet_info0->format & type ) != 0 );
  
}


/**
 * Checks whether packet type is dix or not.
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is dix
*/
bool packet_type_eth_dix( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return if_packet_type( frame, ETH_DIX );
}


/**
 * Checks whether packet has vtag or not.
 * @param buf Pointer to buffer type structure
 * @return bool true if packet has vtag
 */
bool packet_type_eth_vtag( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return if_packet_type( frame, ETH_8021Q );
}


/**
 * Checks whether packet type is 802.3 raw or not.
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is 802.3 raw
 */
bool packet_type_eth_raw( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return if_packet_type( frame, ETH_8023_RAW );
}


/**
 * Checks whether packet type is 802.3 llc or not.
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is 802.3 llc
 */
bool packet_type_eth_llc( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return if_packet_type( frame, ETH_8023_LLC );
}


/**
 * Checks whether packet type is 802.3 snap or not.
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is 802.3 snap
 */
bool packet_type_eth_snap( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return if_packet_type( frame, ETH_8023_SNAP );
}


/**
 * Checks whether packet type is dix with vtag
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is dix with vtag
 */
bool packet_type_eth_vtag_dix( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  if ( if_packet_type( frame, ETH_DIX ) 
       & if_packet_type( frame, ETH_8021Q ) ) 
    return true;
  else 
    return false;
}


/**
 * Checks whether packet type is 802.3 raw with vtag
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is 802.3 raw with vtag
 */
bool packet_type_eth_vtag_raw( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  if ( if_packet_type( frame, ETH_8023_RAW ) 
       & if_packet_type( frame, ETH_8021Q ) ) 
    return true;
  else 
    return false;
}


/**
 * Checks whether packet type is 802.3 llc with vtag
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is 802.3 llc with vtag
 */
bool packet_type_eth_vtag_llc( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  if ( if_packet_type( frame, ETH_8023_LLC ) 
       & if_packet_type( frame, ETH_8021Q ) ) 
    return true;
  else 
    return false;
}


/**
 * Checks whether packet type is 802.3 snap with vtag
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is 802.3 snap with vtag
 */
bool packet_type_eth_vtag_snap( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  if ( if_packet_type( frame, ETH_8023_SNAP ) 
       & if_packet_type( frame, ETH_8021Q ) ) 
    return true;
  else 
    return false;
}


/**
 * Checks whether packet type is ARP
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is arp
 */
bool packet_type_eth_arp( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, NW_ARP ) );
}


/**
 * Checks whether packet type is IPv4
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is ipv4
 */
bool packet_type_eth_ipv4( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, NW_IPV4 ) );
}

#if 0
bool packet_type_eth_ipv4_icmpv4( const buffer *frame );
bool packet_type_eth_ipv4_tcp( const buffer *frame );
bool packet_type_eth_ipv4_udp( const buffer *frame );
bool packet_type_eth_vtag_arp( const buffer *frame );
bool packet_type_eth_vtag_ipv4( const buffer *frame );
bool packet_type_eth_vtag_ipv4_icmpv4( const buffer *frame );
bool packet_type_eth_vtag_ipv4_tcp( const buffer *frame );
bool packet_type_eth_vtag_ipv4_udp( const buffer *frame );
bool packet_type_eth_snap_arp( const buffer *frame );
bool packet_type_eth_snap_ipv4( const buffer *frame );
bool packet_type_eth_snap_ipv4_icmpv4( const buffer *frame );
bool packet_type_eth_snap_ipv4_tcp( const buffer *frame );
bool packet_type_eth_snap_ipv4_udp( const buffer *frame );
bool packet_type_eth_snap_vtag_arp( const buffer *frame );
bool packet_type_eth_snap_vtag_ipv4( const buffer *frame );
bool packet_type_eth_snap_vtag_ipv4_icmpv4( const buffer *frame );
bool packet_type_eth_snap_vtag_ipv4_tcp( const buffer *frame );
bool packet_type_eth_snap_vtag_ipv4_udp( const buffer *frame );
#endif

/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
