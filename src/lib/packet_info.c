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


#include <assert.h>
#include "packet_info.h"
#include "wrapper.h"
#include "trema.h"


#define check_null( data )                              \
  {                                                     \
    if ( data == NULL )                                 \
      die( "illegal argument to %s", __func__ );        \
  }


void
free_packet_info( buffer *buf ) {
  assert( buf != NULL );
  assert( buf->user_data != NULL );

  xfree( buf->user_data );
  buf->user_data = NULL;
  buf->user_data_free_function = NULL;
}


void
calloc_packet_info( buffer *buf ) {
  assert( buf != NULL );

  void *user_data = xcalloc( 1, sizeof( packet_info ) );
  assert( user_data != NULL );

  memset( user_data, 0, sizeof( packet_info ) );

  buf->user_data = user_data;
  buf->user_data_free_function = free_packet_info;
}


packet_info 
get_packet_info( const buffer *frame ) {
  assert( frame != NULL );

  packet_info info;
  
  if ( frame->user_data != NULL ) {
    info = *( packet_info * ) frame->user_data;
  } else {
    memset( &info, 0, sizeof( info ) );
  }
  
  return info;
}


static bool
if_packet_type( const uint32_t format, const uint32_t type ) {
  return ( ( format & type ) == type );
}


bool 
packet_type_eth_dix( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return if_packet_type( packet_info.format, ETH_DIX );
}


bool 
packet_type_eth_vtag( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return if_packet_type( packet_info.format, ETH_8021Q );
}


bool 
packet_type_eth_raw( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return if_packet_type( packet_info.format, ETH_8023_RAW );
}


bool 
packet_type_eth_llc( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return if_packet_type( packet_info.format, ETH_8023_LLC );
}


bool 
packet_type_eth_snap( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return if_packet_type( packet_info.format, ETH_8023_SNAP );
}


bool 
packet_type_eth_vtag_dix( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return ( if_packet_type( packet_info.format, ETH_DIX ) &
           if_packet_type( packet_info.format, ETH_8021Q ) );
}


bool 
packet_type_eth_vtag_raw( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return ( if_packet_type( packet_info.format, ETH_8023_RAW ) &
           if_packet_type( packet_info.format, ETH_8021Q ) );
}


bool 
packet_type_eth_vtag_llc( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return ( if_packet_type( packet_info.format, ETH_8023_LLC ) &
           if_packet_type( packet_info.format, ETH_8021Q ) );
}


bool 
packet_type_eth_vtag_snap( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return ( if_packet_type( packet_info.format, ETH_8023_SNAP ) &
           if_packet_type( packet_info.format, ETH_8021Q ) );
}


bool 
packet_type_eth_arp( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return if_packet_type( packet_info.format, NW_ARP );
}


bool 
packet_type_eth_ipv4( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return if_packet_type( packet_info.format, NW_IPV4 );
}


bool 
packet_type_eth_ipv4_icmpv4( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return ( if_packet_type( packet_info.format, NW_IPV4 ) &
           if_packet_type( packet_info.format, NW_ICMPV4 ) );
}


bool 
packet_type_eth_ipv4_tcp( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return ( if_packet_type( packet_info.format, NW_IPV4 ) &
           if_packet_type( packet_info.format, TP_TCP ) );
}


bool 
packet_type_eth_ipv4_udp( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return ( if_packet_type( packet_info.format, NW_IPV4 ) &
           if_packet_type( packet_info.format, TP_UDP ) );
}


bool 
packet_type_eth_vtag_arp( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return ( if_packet_type( packet_info.format, NW_ARP ) &
           if_packet_type( packet_info.format, ETH_8021Q ) );
}


bool 
packet_type_eth_vtag_ipv4( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return ( if_packet_type( packet_info.format, NW_IPV4 ) &
           if_packet_type( packet_info.format, ETH_8021Q ) );
}


bool 
packet_type_eth_vtag_ipv4_icmpv4( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return ( if_packet_type( packet_info.format, NW_IPV4 ) &
           if_packet_type( packet_info.format, NW_ICMPV4 ) &
           if_packet_type( packet_info.format, ETH_8021Q ) );
}


bool 
packet_type_eth_vtag_ipv4_tcp( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return ( if_packet_type( packet_info.format, NW_IPV4 ) &
           if_packet_type( packet_info.format, TP_TCP ) &
           if_packet_type( packet_info.format, ETH_8021Q ) );
}


bool 
packet_type_eth_vtag_ipv4_udp( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return ( if_packet_type( packet_info.format, NW_IPV4 ) &
           if_packet_type( packet_info.format, TP_UDP ) &
           if_packet_type( packet_info.format, ETH_8021Q ) );
}


bool 
packet_type_eth_snap_arp( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return ( if_packet_type( packet_info.format, NW_ARP ) &
           if_packet_type( packet_info.format, ETH_8023_SNAP ) );
}


bool 
packet_type_eth_snap_ipv4( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return ( if_packet_type( packet_info.format, NW_IPV4 ) &
           if_packet_type( packet_info.format, ETH_8023_SNAP ) );
}


bool 
packet_type_eth_snap_ipv4_icmpv4( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return ( if_packet_type( packet_info.format, NW_IPV4 ) &
           if_packet_type( packet_info.format, NW_ICMPV4 ) &
           if_packet_type( packet_info.format, ETH_8023_SNAP ) );
}


bool 
packet_type_eth_snap_ipv4_tcp( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return ( if_packet_type( packet_info.format, NW_IPV4 ) &
           if_packet_type( packet_info.format, TP_TCP ) &
           if_packet_type( packet_info.format, ETH_8023_SNAP ) );
}


bool 
packet_type_eth_snap_ipv4_udp( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return ( if_packet_type( packet_info.format, NW_IPV4 ) &
           if_packet_type( packet_info.format, TP_TCP ) &
           if_packet_type( packet_info.format, ETH_8023_SNAP ) );
}


bool 
packet_type_eth_snap_vtag_arp( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return ( if_packet_type( packet_info.format, NW_ARP ) &
           if_packet_type( packet_info.format, ETH_8021Q ) &
           if_packet_type( packet_info.format, ETH_8023_SNAP ) );
}


bool 
packet_type_eth_snap_vtag_ipv4( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return ( if_packet_type( packet_info.format, NW_IPV4 ) &
           if_packet_type( packet_info.format, ETH_8021Q ) &
           if_packet_type( packet_info.format, ETH_8023_SNAP ) );
}


bool 
packet_type_eth_snap_vtag_ipv4_icmpv4( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return ( if_packet_type( packet_info.format, NW_IPV4 ) &
           if_packet_type( packet_info.format, NW_ICMPV4 ) &
           if_packet_type( packet_info.format, ETH_8021Q ) &
           if_packet_type( packet_info.format, ETH_8023_SNAP ) );
}


bool 
packet_type_eth_snap_vtag_ipv4_tcp( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return ( if_packet_type( packet_info.format, NW_IPV4 ) &
           if_packet_type( packet_info.format, TP_TCP ) &
           if_packet_type( packet_info.format, ETH_8021Q ) &
           if_packet_type( packet_info.format, ETH_8023_SNAP ) );
}


bool 
packet_type_eth_snap_vtag_ipv4_udp( const buffer *frame ) {
  check_null( frame );
  check_null( frame->user_data );

  packet_info packet_info = get_packet_info( frame );

  return ( if_packet_type( packet_info.format, NW_IPV4 ) &
           if_packet_type( packet_info.format, TP_UDP ) &
           if_packet_type( packet_info.format, ETH_8021Q ) &
           if_packet_type( packet_info.format, ETH_8023_SNAP ) );
}


bool
packet_type_ether( const buffer *frame ) {
  check_null( frame );
  
  packet_info packet_info = get_packet_info( frame );

  return ( if_packet_type( packet_info.format, ETH_DIX ) |
           if_packet_type( packet_info.format, ETH_8023_RAW ) |
           if_packet_type( packet_info.format, ETH_8023_LLC ) |
           if_packet_type( packet_info.format, ETH_8023_SNAP ) );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
