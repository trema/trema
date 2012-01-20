/*
<<<<<<< HEAD
 * Author: Kazuya Suzuki, Naoyoshi Tada
=======
 * Author: Kazuya Suzuki
>>>>>>> 798f20ee867e0db64216dfa469b4fa9c8a7a3afb
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



#include <assert.h>
#include "checks.h"
#include "packet_info.h"
#include "wrapper.h"
#include "trema.h"


<<<<<<< HEAD
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
=======
void
free_packet_info( buffer *buf ) {
  die_if_NULL( buf );
  die_if_NULL( buf->user_data );
>>>>>>> 798f20ee867e0db64216dfa469b4fa9c8a7a3afb

  xfree( buf->user_data );
  buf->user_data = NULL;
  buf->user_data_free_function = NULL;
}


void
calloc_packet_info( buffer *buf ) {
<<<<<<< HEAD
  assert( buf != NULL );
=======
  die_if_NULL( buf );
>>>>>>> 798f20ee867e0db64216dfa469b4fa9c8a7a3afb

  void *user_data = xcalloc( 1, sizeof( packet_info ) );
  assert( user_data != NULL );

  memset( user_data, 0, sizeof( packet_info ) );

  buf->user_data = user_data;
  buf->user_data_free_function = free_packet_info;
<<<<<<< HEAD
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
=======
>>>>>>> 798f20ee867e0db64216dfa469b4fa9c8a7a3afb
}


packet_info
get_packet_info( const buffer *frame ) {
  die_if_NULL( frame );

  packet_info info;
  
  if ( frame->user_data != NULL ) {
    info = *( packet_info * ) frame->user_data;
  } 
  else {
    memset( &info, 0, sizeof( info ) );
  }
  
  return info;
}

<<<<<<< HEAD
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
bool 
packet_type_eth_dix( const buffer *frame ) {
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
bool 
packet_type_eth_vtag( const buffer *frame ) {
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
bool 
packet_type_eth_raw( const buffer *frame ) {
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
bool 
packet_type_eth_llc( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

=======

static bool
if_packet_type( const buffer *frame, const uint32_t type ) {
  die_if_NULL( frame );
  packet_info packet_info = get_packet_info( frame );
  return ( ( packet_info.format & type ) == type );
}


bool
packet_type_eth_dix( const buffer *frame ) {
  die_if_NULL( frame );
  return if_packet_type( frame, ETH_DIX );
}


bool
packet_type_eth_vtag( const buffer *frame ) {
  die_if_NULL( frame );
  return if_packet_type( frame, ETH_8021Q );
}


bool
packet_type_eth_raw( const buffer *frame ) {
  die_if_NULL( frame );
  return if_packet_type( frame, ETH_8023_RAW );
}


bool
packet_type_eth_llc( const buffer *frame ) {
  die_if_NULL( frame );
>>>>>>> 798f20ee867e0db64216dfa469b4fa9c8a7a3afb
  return if_packet_type( frame, ETH_8023_LLC );
}


<<<<<<< HEAD
/**
 * Checks whether packet type is 802.3 snap or not.
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is 802.3 snap
 */
bool 
packet_type_eth_snap( const buffer *frame ) {
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
bool 
packet_type_eth_vtag_dix( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, ETH_DIX ) &
           if_packet_type( frame, ETH_8021Q ) );
}


/**
 * Checks whether packet type is 802.3 raw with vtag
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is 802.3 raw with vtag
 */
bool 
packet_type_eth_vtag_raw( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, ETH_8023_RAW ) &
           if_packet_type( frame, ETH_8021Q ) );
}


/**
 * Checks whether packet type is 802.3 llc with vtag
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is 802.3 llc with vtag
 */
bool 
packet_type_eth_vtag_llc( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, ETH_8023_LLC ) &
           if_packet_type( frame, ETH_8021Q ) );
}


/**
 * Checks whether packet type is 802.3 snap with vtag
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is 802.3 snap with vtag
 */
bool 
packet_type_eth_vtag_snap( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, ETH_8023_SNAP ) &
           if_packet_type( frame, ETH_8021Q ) );
}


/**
 * Checks whether packet type is ARP
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is arp
 */
bool 
packet_type_eth_arp( const buffer *frame ) {
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
bool 
packet_type_eth_ipv4( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, NW_IPV4 ) );
}


/**
 * Checks whether packet type is IPv4 and ICMPv4
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is ipv4 and icmpv4.
 */
bool 
packet_type_eth_ipv4_icmpv4( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, NW_IPV4 ) &
           if_packet_type( frame, NW_ICMPV4 ) );
}


/**
 * Checks whether packet type is IPv4 and TCP
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is ipv4 and tcp.
 */
bool 
packet_type_eth_ipv4_tcp( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, NW_IPV4 ) &
           if_packet_type( frame, TP_TCP ) );
}


/**
 * Checks whether packet type is IPv4 and UDP
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is ipv4 and udp.
 */
bool 
packet_type_eth_ipv4_udp( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, NW_IPV4 ) &
           if_packet_type( frame, TP_UDP ) );
}


/**
 * Checks whether packet type is ARP with 802.1q tag.
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is arp with 802.1q tag.
 */
bool 
packet_type_eth_vtag_arp( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, NW_ARP ) &
           if_packet_type( frame, ETH_8021Q ) );
}


/**
 * Checks whether packet type is IPv4 with 802.1q tag.
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is ipv4 with 802.1q tag.
 */
bool 
packet_type_eth_vtag_ipv4( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, NW_IPV4 ) &
           if_packet_type( frame, ETH_8021Q ) );
}


/**
 * Checks whether packet type is IPv4 and ICMPv4 with 802.1q tag
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is ipv4 and icmpv4 with 802.1q tag.
 */
bool 
packet_type_eth_vtag_ipv4_icmpv4( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, NW_IPV4 ) &
           if_packet_type( frame, NW_ICMPV4 ) &
           if_packet_type( frame, ETH_8021Q ) );
}


/**
 * Checks whether packet type is IPv4 and TCP with 802.1q tag
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is ipv4 and tcp with 802.1q tag.
 */
bool 
packet_type_eth_vtag_ipv4_tcp( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, NW_IPV4 ) &
           if_packet_type( frame, TP_TCP ) &
           if_packet_type( frame, ETH_8021Q ) );
}


/**
 * Checks whether packet type is IPv4 and UDP with 802.1q tag
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is ipv4 and udp with 802.1q tag.
 */
bool 
packet_type_eth_vtag_ipv4_udp( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, NW_IPV4 ) &
           if_packet_type( frame, TP_UDP ) &
           if_packet_type( frame, ETH_8021Q ) );
}


/**
 * Checks whether packet type is ARP with snap header
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is ARP with snap header.
 */
bool 
packet_type_eth_snap_arp( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, NW_ARP ) &
           if_packet_type( frame, ETH_8023_SNAP ) );
}


/**
 * Checks whether packet type is IPv4 with snap header
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is ipv4 with snap header.
 */
bool packet_type_eth_snap_ipv4( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, NW_IPV4 ) &
           if_packet_type( frame, ETH_8023_SNAP ) );
}


/**
 * Checks whether packet type is IPv4 and ICMPv4 with snap header
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is ipv4 and icmpv4 with snap header.
 */
bool 
packet_type_eth_snap_ipv4_icmpv4( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, NW_IPV4 ) &
           if_packet_type( frame, NW_ICMPV4 ) &
=======
bool
packet_type_ether( const buffer *frame ) {
  die_if_NULL( frame );
  return ( if_packet_type( frame, ETH_DIX ) |
           if_packet_type( frame, ETH_8023_RAW ) |
           if_packet_type( frame, ETH_8023_LLC ) |
>>>>>>> 798f20ee867e0db64216dfa469b4fa9c8a7a3afb
           if_packet_type( frame, ETH_8023_SNAP ) );
}


<<<<<<< HEAD
/**
 * Checks whether packet type is IPv4 and TCP with snap header
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is ipv4 and tcp  with snap header.
 */
bool 
packet_type_eth_snap_ipv4_tcp( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, NW_IPV4 ) &
           if_packet_type( frame, TP_TCP ) &
           if_packet_type( frame, ETH_8023_SNAP ) );
}


/**
 * Checks whether packet type is IPv4 and UDP with snap header
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is ipv4 and udp with snap header.
 */
bool 
packet_type_eth_snap_ipv4_udp( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, NW_IPV4 ) &
           if_packet_type( frame, TP_TCP ) &
           if_packet_type( frame, ETH_8023_SNAP ) );
}


/**
 * Checks whether packet type is ARP with snap and 802.1q tag
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is arp with snap and 802.1q tag
 */
bool 
packet_type_eth_snap_vtag_arp( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, NW_ARP ) &
           if_packet_type( frame, ETH_8021Q ) &
           if_packet_type( frame, ETH_8023_SNAP ) );
}


/**
 * Checks whether packet type is IPv4 with snap and 802.1q tag
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is ipv4 with snap and 802.1q tag
 */
bool 
packet_type_eth_snap_vtag_ipv4( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, NW_IPV4 ) &
           if_packet_type( frame, ETH_8021Q ) &
           if_packet_type( frame, ETH_8023_SNAP ) );
}


/**
 * Checks whether packet type is IPv4 and ICMPv4 with snap and 802.1q tag
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is ipv4 and icmpv4 with snap and 802.1q tag
 */
bool 
packet_type_eth_snap_vtag_ipv4_icmpv4( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, NW_IPV4 ) &
           if_packet_type( frame, NW_ICMPV4 ) &
           if_packet_type( frame, ETH_8021Q ) &
           if_packet_type( frame, ETH_8023_SNAP ) );
}


/**
 * Checks whether packet type is IPv4 and TCP
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is ipv4 and tcp with snap and 802.1q tag
 */
bool 
packet_type_eth_snap_vtag_ipv4_tcp( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, NW_IPV4 ) &
           if_packet_type( frame, TP_TCP ) &
           if_packet_type( frame, ETH_8021Q ) &
           if_packet_type( frame, ETH_8023_SNAP ) );
}


/**
 * Checks whether packet type is IPv4 and UDP with snap and 802.1q tag
 * @param buf Pointer to buffer type structure
 * @return bool true if packet type is ipv4 and udp with snap and 802.1q tag
 */
bool 
packet_type_eth_snap_vtag_ipv4_udp( const buffer *frame ) {
  if ( frame == NULL ) {
    die( "illegal argument to %s", __func__ );
  }
  if ( frame->user_data == NULL ) {
    die( "illegal argument to %s", __func__ );
  }

  return ( if_packet_type( frame, NW_IPV4 ) &
           if_packet_type( frame, TP_UDP ) &
           if_packet_type( frame, ETH_8021Q ) &
           if_packet_type( frame, ETH_8023_SNAP ) );
=======
bool
packet_type_eth_snap( const buffer *frame ) {
  die_if_NULL( frame );
  return if_packet_type( frame, ETH_8023_SNAP );
}


bool
packet_type_arp( const buffer *frame ) {
  die_if_NULL( frame );
  return if_packet_type( frame, NW_ARP );
}


bool
packet_type_ipv4( const buffer *frame ) {
  die_if_NULL( frame );
  return if_packet_type( frame, NW_IPV4 );
}


bool
packet_type_icmpv4( const buffer *frame ) {
  die_if_NULL( frame );
  return if_packet_type( frame, NW_ICMPV4 );
}


bool
packet_type_ipv4_tcp( const buffer *frame ) {
  die_if_NULL( frame );
  return if_packet_type( frame, NW_IPV4 | TP_TCP );
}


bool
packet_type_ipv4_udp( const buffer *frame ) {
  die_if_NULL( frame );
  return if_packet_type( frame, NW_IPV4 | TP_UDP );
>>>>>>> 798f20ee867e0db64216dfa469b4fa9c8a7a3afb
}



/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
