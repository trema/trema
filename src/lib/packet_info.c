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
#include "checks.h"
#include "packet_info.h"
#include "wrapper.h"


void
free_packet_info( buffer *buf ) {
  die_if_NULL( buf );
  die_if_NULL( buf->user_data );

  xfree( buf->user_data );
  buf->user_data = NULL;
  buf->user_data_free_function = NULL;
}


void
calloc_packet_info( buffer *buf ) {
  die_if_NULL( buf );

  void *user_data = xcalloc( 1, sizeof( packet_info ) );
  assert( user_data != NULL );

  memset( user_data, 0, sizeof( packet_info ) );

  buf->user_data = user_data;
  buf->user_data_free_function = free_packet_info;
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
  return if_packet_type( frame, ETH_8023_LLC );
}


bool
packet_type_ether( const buffer *frame ) {
  die_if_NULL( frame );
  return ( if_packet_type( frame, ETH_DIX ) |
           if_packet_type( frame, ETH_8023_RAW ) |
           if_packet_type( frame, ETH_8023_LLC ) |
           if_packet_type( frame, ETH_8023_SNAP ) );
}


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
packet_type_rarp( const buffer *frame ) {
  die_if_NULL( frame );
  return if_packet_type( frame, NW_RARP );
}


bool
packet_type_ipv4( const buffer *frame ) {
  die_if_NULL( frame );
  return if_packet_type( frame, NW_IPV4 );
}


bool
packet_type_ipv6( const buffer *frame ) {
  die_if_NULL( frame );
  return if_packet_type( frame, NW_IPV6 );
}


bool
packet_type_lldp( const buffer *frame ) {
  die_if_NULL( frame );
  return if_packet_type( frame, NW_LLDP );
}


bool
packet_type_icmpv4( const buffer *frame ) {
  die_if_NULL( frame );
  return if_packet_type( frame, NW_ICMPV4 );
}


bool
packet_type_igmp( const buffer *frame ) {
  die_if_NULL( frame );
  return if_packet_type( frame, NW_IGMP );
}


bool
packet_type_ipv4_tcp( const buffer *frame ) {
  die_if_NULL( frame );
  return if_packet_type( frame, NW_IPV4 | TP_TCP );
}


bool
packet_type_ipv6_tcp( const buffer *frame ) {
  die_if_NULL( frame );
  return if_packet_type( frame, NW_IPV6 | TP_TCP );
}


bool
packet_type_ipv4_udp( const buffer *frame ) {
  die_if_NULL( frame );
  return if_packet_type( frame, NW_IPV4 | TP_UDP );
}


bool
packet_type_ipv6_udp( const buffer *frame ) {
  die_if_NULL( frame );
  return if_packet_type( frame, NW_IPV6 | TP_UDP );
}


bool
packet_type_ipv4_etherip( const buffer *frame ) {
  die_if_NULL( frame );
  return if_packet_type( frame, NW_IPV4 | TP_ETHERIP );
}


static bool
if_arp_opcode( const buffer *frame, const uint32_t opcode ) {
  die_if_NULL( frame );
  packet_info packet_info = get_packet_info( frame );
  return ( packet_info.arp_ar_op == opcode );
}


bool
packet_type_arp_request( const buffer *frame ) {
  die_if_NULL( frame );
  return ( if_packet_type( frame, NW_ARP ) &
           if_arp_opcode( frame, ARP_OP_REQUEST ) );
}


bool
packet_type_arp_reply( const buffer *frame ) {
  die_if_NULL( frame );
  return ( if_packet_type( frame, NW_ARP ) &
           if_arp_opcode( frame, ARP_OP_REPLY ) );
}


bool
packet_type_rarp_request( const buffer *frame ) {
  die_if_NULL( frame );
  return ( if_packet_type( frame, NW_RARP ) &
           if_arp_opcode( frame, ARP_OP_RREQUEST ) );
}


bool
packet_type_rarp_reply( const buffer *frame ) {
  die_if_NULL( frame );
  return ( if_packet_type( frame, NW_RARP ) &
           if_arp_opcode( frame, ARP_OP_RREPLY ) );
}


static bool
if_icmpv4_type( const buffer *frame, const uint32_t type ) {
  die_if_NULL( frame );
  packet_info packet_info = get_packet_info( frame );
  return ( packet_info.icmpv4_type == type );
}


bool
packet_type_icmpv4_echo_reply( const buffer *frame ) {
  die_if_NULL( frame );
  return ( if_packet_type( frame, NW_ICMPV4 ) &
           if_icmpv4_type( frame, ICMP_TYPE_ECHOREP ) );
}


bool
packet_type_icmpv4_dst_unreach( const buffer *frame ) {
  die_if_NULL( frame );
  return ( if_packet_type( frame, NW_ICMPV4 ) &
           if_icmpv4_type( frame, ICMP_TYPE_UNREACH ) );
}


bool
packet_type_icmpv4_redirect( const buffer *frame ) {
  die_if_NULL( frame );
  return ( if_packet_type( frame, NW_ICMPV4 ) &
           if_icmpv4_type( frame, ICMP_TYPE_REDIRECT ) );
}


bool
packet_type_icmpv4_echo_request( const buffer *frame ) {
  die_if_NULL( frame );
  return ( if_packet_type( frame, NW_ICMPV4 ) &
           if_icmpv4_type( frame, ICMP_TYPE_ECHOREQ ) );
}


static bool
if_igmp_type( const buffer *frame, const uint32_t type ) {
  die_if_NULL( frame );
  packet_info packet_info = get_packet_info( frame );
  return ( packet_info.igmp_type == type );
}


bool
packet_type_igmp_membership_query( const buffer *frame ) {
  die_if_NULL( frame );
  return ( if_packet_type( frame, NW_IGMP ) &
           if_igmp_type( frame, IGMP_TYPE_MEMBERSHIP_QUERY ) );
}


bool
packet_type_igmp_v1_membership_report( const buffer *frame ) {
  die_if_NULL( frame );
  return ( if_packet_type( frame, NW_IGMP ) &
           if_igmp_type( frame, IGMP_TYPE_V1_MEMBERSHIP_REPORT ) );
}

bool
packet_type_igmp_v2_membership_report( const buffer *frame ) {
  die_if_NULL( frame );
  return ( if_packet_type( frame, NW_IGMP ) &
           if_igmp_type( frame, IGMP_TYPE_V2_MEMBERSHIP_REPORT ) );
}


bool
packet_type_igmp_v2_leave_group( const buffer *frame ) {
  die_if_NULL( frame );
  return ( if_packet_type( frame, NW_IGMP ) &
           if_igmp_type( frame, IGMP_TYPE_V2_LEAVE_GROUP ) );
}


bool
packet_type_igmp_v3_membership_report( const buffer *frame ) {
  die_if_NULL( frame );
  return ( if_packet_type( frame, NW_IGMP ) &
           if_igmp_type( frame, IGMP_TYPE_V3_MEMBERSHIP_REPORT ) );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
