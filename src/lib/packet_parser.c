/*
 * Author: Kazuya Suzuki
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
 * @file packet_parser.c
 * Source file containing functions for handling packets. This file contains functions for 
 * parsing a packet to find its type (whether IPv4, or ARP) and peform checksum if required.
 */


#include <assert.h>
#include <stdint.h>
#include "packet_info.h"
#include "log.h"
#include "wrapper.h"


/**
 */
static void *
parse_ether( void *ptr, buffer *buf ) {
  assert( ptr != NULL );
  assert( buf != NULL );

  packet_info *packet_info0 = ( packet_info * )buf->user_data;

  // Ethernet header
  ether_header_t *ether_header = ( ether_header_t * )ptr;
  memcpy( packet_info0->eth_macsa, ether_header->macsa, ETH_ADDRLEN );
  memcpy( packet_info0->eth_macda, ether_header->macda, ETH_ADDRLEN );
  packet_info0->eth_type = ntohs( ether_header->type );

  debug( "Parsing an Ethernet frame "
         "( source mac = %02x:%02x:%02x:%02x:%02x:%02x, "
         "destination mac = %02x:%02x:%02x:%02x:%02x:%02x ).",
         packet_info0->eth_macsa[ 0 ], packet_info0->eth_macsa[ 1 ], 
         packet_info0->eth_macsa[ 2 ], packet_info0->eth_macsa[ 3 ], 
         packet_info0->eth_macsa[ 4 ], packet_info0->eth_macsa[ 5 ],
         packet_info0->eth_macda[ 0 ], packet_info0->eth_macda[ 1 ], 
         packet_info0->eth_macda[ 2 ], packet_info0->eth_macda[ 3 ], 
         packet_info0->eth_macda[ 4 ], packet_info0->eth_macda[ 5 ] );

  if ( packet_info0->eth_macsa[ 0 ] & 0x01 ) {
    debug( "Source MAC address is multicast or broadcast." );
  }
  ptr = ( void * )( ether_header + 1 ); 

  // vlan tag 
  if ( packet_info0->eth_type == ETH_ETHTYPE_TPID ) {
    vlantag_header_t *vlantag_header = ( vlantag_header_t * )ptr;

    packet_info0->vlan_tci = ntohs( vlantag_header->tci );
    packet_info0->vlan_tpid = packet_info0->eth_type;
    // is correct?
    packet_info0->vlan_prio = ( uint8_t )( ( packet_info0->vlan_tci >> 13 ) & 7 ); 
    packet_info0->vlan_cfi = ( uint8_t )( ( packet_info0->vlan_tci >> 12 ) & 1 );
    packet_info0->vlan_vid = packet_info0->vlan_tci & 0x0FFF;

    // Rewrite eth_type 
    packet_info0->eth_type = ntohs( vlantag_header->type ); 

    packet_info0->format |= ETH_8021Q;
    
    ptr = ( void * )( vlantag_header + 1 );
  }
  
  // snap header.
  if ( packet_info0->eth_type <= ETH_MTU ) {
    snap_header_t *snap_header = ( snap_header_t * )ptr;

    memcpy( packet_info0->snap_llc, snap_header->llc, SNAP_LLC_LENGTH );
    memcpy( packet_info0->snap_oui, snap_header->oui, SNAP_OUI_LENGTH );
    packet_info0->snap_type = ntohs( snap_header->type );

    packet_info0->format |= ETH_8023_SNAP;

    ptr = ( void * )( snap_header + 1 );
  } else {
    packet_info0->format |= ETH_DIX;
  }
  
  return ptr;
}


/**
 */
static void *
parse_arp( void *ptr, buffer *buf ) {
  assert( ptr != NULL );
  assert( buf != NULL );

  packet_info *packet_info0 = ( packet_info * )buf->user_data;

  // Ethernet header
  arp_header_t *arp_header = ( arp_header_t * )ptr;
  packet_info0->arp_ar_hrd = ntohs( arp_header->ar_hrd );
  packet_info0->arp_ar_pro = ntohs( arp_header->ar_pro );
  packet_info0->arp_ar_hln = arp_header->ar_hln;
  packet_info0->arp_ar_pln = arp_header->ar_pln;
  packet_info0->arp_ar_op = ntohs( arp_header->ar_op );
  memcpy( packet_info0->arp_sha, arp_header->sha, ETH_ADDRLEN );
  packet_info0->arp_spa = ntohl( arp_header->sip );
  memcpy( packet_info0->arp_tha, arp_header->tha, ETH_ADDRLEN );
  packet_info0->arp_tpa = ntohl( arp_header->tip );

  packet_info0->format |= NW_ARP;
  
  ptr = ( void * )( arp_header + 1 );

  return ptr;
};


/**
 */
static void *
parse_ipv4( void *ptr, buffer *buf ) {
  assert( ptr != NULL );
  assert( buf != NULL );

  packet_info *packet_info0 = ( packet_info * )buf->user_data;

  // Ethernet header
  ipv4_header_t *ipv4_header = ( ipv4_header_t * )ptr;
  packet_info0->ipv4_version = ipv4_header->version;
  packet_info0->ipv4_ihl = ipv4_header->ihl;
  packet_info0->ipv4_tos = ipv4_header->tos;
  packet_info0->ipv4_tot_len = ntohs( ipv4_header->tot_len );
  packet_info0->ipv4_id = ntohs( ipv4_header->id );
  packet_info0->ipv4_frag_off = ntohs( ipv4_header->frag_off );
  packet_info0->ipv4_ttl = ipv4_header->ttl;
  packet_info0->ipv4_protocol = ipv4_header->protocol;
  packet_info0->ipv4_checksum = ntohs( ipv4_header->check );
  packet_info0->ipv4_saddr = ntohl( ipv4_header->saddr );
  packet_info0->ipv4_daddr = ntohl( ipv4_header->daddr );

  ptr = ( void * )( ipv4_header + 1 );

  return ptr;
}


/**
 */
static void *
parse_icmp( void *ptr, buffer *buf ) {
  assert( ptr != NULL );
  assert( buf != NULL );

  packet_info *packet_info0 = ( packet_info * )buf->user_data;

  // ICMPV4 header
  icmp_header_t *icmp_header = ( icmp_header_t * )ptr;
  packet_info0->icmpv4_type = icmp_header->type;
  packet_info0->icmpv4_code = icmp_header->code;
  packet_info0->icmpv4_checksum = ntohs( icmp_header->csum );

  switch ( packet_info0->icmpv4_type ) {
  case ICMP_TYPE_ECHOREP:
  case ICMP_TYPE_ECHOREQ:
    packet_info0->icmpv4_id = ntohs( icmp_header->icmp_data.echo.ident );
    packet_info0->icmpv4_seq = ntohs( icmp_header->icmp_data.echo.seq );
    break;

  case ICMP_TYPE_REDIRECT:
    packet_info0->icmpv4_gateway = ntohl( icmp_header->icmp_data.gateway );
    break;

  default:
    break;
  }

  packet_info0->format |= NW_ICMPV4;

  ptr = ( void * )( icmp_header + 1 );

  return ptr;
};


/**
 */
static void *
parse_udp( void *ptr, buffer *buf ) {
  assert( ptr != NULL );
  assert( buf != NULL );

  packet_info *packet_info0 = ( packet_info * )buf->user_data;

  // UDP header
  udp_header_t *udp_header = ( udp_header_t * )ptr;
  packet_info0->udp_src_port = ntohs( udp_header->src_port );
  packet_info0->udp_dst_port = ntohs( udp_header->dst_port );
  packet_info0->udp_len = ntohs( udp_header->len );
  packet_info0->udp_checksum = ntohs( udp_header->csum );

  packet_info0->format |= TP_UDP;

  ptr = ( void * )( udp_header + 1 );

  return ptr;
};


/**
 */
static void *
parse_tcp( void *ptr, buffer *buf ) {
  assert( ptr != NULL );
  assert( buf != NULL );

  packet_info *packet_info0 = ( packet_info * )buf->user_data;

  // TCP header
  tcp_header_t *tcp_header = ( tcp_header_t * )ptr;
  packet_info0->tcp_src_port = ntohs( tcp_header->src_port );
  packet_info0->tcp_dst_port = ntohs( tcp_header->dst_port );
  packet_info0->tcp_seq_no = ntohl( tcp_header->seq_no );
  packet_info0->tcp_ack_no = ntohl( tcp_header->ack_no );
  packet_info0->tcp_offset = tcp_header->offset;
  packet_info0->tcp_flags = tcp_header->flags;
  packet_info0->tcp_window = ntohs( tcp_header->window );
  packet_info0->tcp_checksum = ntohs( tcp_header->csum );
  packet_info0->tcp_urgent = ntohs( tcp_header->urgent );

  packet_info0->format |= TP_TCP;

  ptr = ( void * )( tcp_header + 1 );

  return ptr;
};


/**
 * Validates packet header information contained in structure of type packet_header_info.
 * @param buf Pointer to buffer type structure, user_data element of which points to structure of type packet_header_info
 * @return bool True if packet has valid header, else False
 */
bool
parse_packet( buffer *buf ) {
  assert( buf != NULL );
  assert( buf->data != NULL );

  alloc_packet_info( buf );
  if ( buf->user_data != NULL ) {
    return false;
  }

  void *ptr = buf->data;
  packet_info *packet_info0 = ( packet_info * )buf->user_data;

  // parse L2 information.
  ptr = parse_ether( ptr, buf );
  
  // parse L3 information.
  switch ( packet_info0->eth_type ) {
  case ETH_ETHTYPE_ARP:
    ptr = parse_arp( ptr, buf );
    break;

  case ETH_ETHTYPE_IPV4:
    ptr = parse_ipv4( ptr, buf );
    break;

  default:
    warn( "" );
    break;
  }
    
  if ( !( packet_info0->format & NW_IPV4 ) ) {
    return true;
  } 

  // parse L4 information.
  switch ( packet_info0->ipv4_protocol ) {
  case IPPROTO_ICMP:
    parse_icmp( ptr, buf );    
    break;

  case IPPROTO_TCP:
    parse_tcp( ptr, buf );    
    break;
    
  case IPPROTO_UDP:
    parse_udp( ptr, buf );    
    break;
    
  default:
    warn( "" );
    break;
  }
  return true;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
