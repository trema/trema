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
#include <netinet/ip.h>
#include "packet_info.h"
#include "log.h"
#include "wrapper.h"


/**
 * Parses an Ethernet header in buf->data and places in buf->user_data.
 */
static void 
parse_ether( buffer *buf ) {
  assert( buf != NULL );

  packet_info *packet_info0 = buf->user_data;
  void *ptr = packet_info0->l2_header;
  assert( ptr != NULL );

  // Check the length of remained buffer
  size_t length = buf->length - 
    ( size_t ) ( ( char * ) ptr - ( char * ) buf->data );
  if ( length < sizeof( ether_header_t ) ) {
    return;
  }

  // Ethernet header
  ether_header_t *ether_header = ptr;
  memcpy( packet_info0->eth_macsa, ether_header->macsa, ETH_ADDRLEN );
  memcpy( packet_info0->eth_macda, ether_header->macda, ETH_ADDRLEN );
  packet_info0->eth_type = ntohs( ether_header->type );

  ptr = ( void * ) ( ether_header + 1 ); 

  // vlan tag 
  if ( packet_info0->eth_type == ETH_ETHTYPE_TPID ) {
    // Check the length of remained buffer
    size_t length = buf->length -
      ( size_t ) ( ( char * ) ptr - ( char * ) buf->data );
    if ( length < sizeof( vlantag_header_t ) ) {
      return;
    }
    vlantag_header_t *vlantag_header = ptr;

    packet_info0->vlan_tci = ntohs( vlantag_header->tci );
    packet_info0->vlan_tpid = packet_info0->eth_type;
    packet_info0->vlan_prio = TCI_GET_PRIO( packet_info0->vlan_tci );
    packet_info0->vlan_cfi = TCI_GET_CFI( packet_info0->vlan_tci );
    packet_info0->vlan_vid = TCI_GET_VID( packet_info0->vlan_tci );

    // Rewrite eth_type 
    packet_info0->eth_type = ntohs( vlantag_header->type ); 

    packet_info0->format |= ETH_8021Q;
    
    ptr = ( void * ) ( vlantag_header + 1 );
  }
  
  // Skip nested vlan headers.
  while (  packet_info0->eth_type == ETH_ETHTYPE_TPID ) { 
    // Check the length of remained buffer
    size_t length = buf->length -
      ( size_t ) ( ( char * ) ptr - ( char * ) buf->data );
    if ( length < sizeof( vlantag_header_t ) ) {
      return;
    }
    vlantag_header_t *vlantag_header = ptr;

    // Rewrite eth_type 
    packet_info0->eth_type = ntohs( vlantag_header->type ); 
    ptr = ( void * ) ( vlantag_header + 1 );
  }
  
  // snap header.
  if ( packet_info0->eth_type <= ETH_MTU ) {
    // Check the length of remained buffer 
    size_t length = buf->length -
      ( size_t ) ( ( char * ) ptr - ( char * ) buf->data );
    if ( length < sizeof( snap_header_t ) ) {
      return;
    }
    snap_header_t *snap_header = ptr;

    memcpy( packet_info0->snap_llc, snap_header->llc, SNAP_LLC_LENGTH );
    memcpy( packet_info0->snap_oui, snap_header->oui, SNAP_OUI_LENGTH );
    packet_info0->snap_type = ntohs( snap_header->type );

    packet_info0->format |= ETH_8023_SNAP;

    ptr = ( void * ) ( snap_header + 1 );
  } 
  else {
    packet_info0->format |= ETH_DIX;
  }

  packet_info0->l3_header = ptr;

  return;
}


/**
 * Parses an ARP header in buf->data and places in buf->user_data.
 */
static void 
parse_arp( buffer *buf ) {
  assert( buf != NULL );

  packet_info *packet_info0 = buf->user_data;
  void *ptr = packet_info0->l3_header;
  assert( ptr != NULL );

  // Check the length of remained buffer
  size_t length = buf->length -
    ( size_t ) ( ( char * ) ptr - ( char * ) buf->data );
  if ( length < sizeof( arp_header_t ) ) {
    return;
  }

  // Ethernet header
  arp_header_t *arp_header = ptr;
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
  
  return;
};


/**
 * Parses an IPv4 header in buf->data and places in buf->user_data.
 */
static void 
parse_ipv4( buffer *buf ) {
  assert( buf != NULL );

  packet_info *packet_info0 = buf->user_data;
  void *ptr = packet_info0->l3_header;
  assert( ptr != NULL );

  // Check the length of remained buffer for an ipv4 header without options.
  size_t length = buf->length -
    ( size_t ) ( ( char * ) ptr - ( char * ) buf->data );
  if ( length < sizeof( ipv4_header_t ) ) {
    return;
  }

  // Check the length of remained buffer for an ipv4 header with options.
  ipv4_header_t *ipv4_header = ptr;
  if ( ipv4_header->ihl < 5 ) {
    return;
  }
  if ( length < ( size_t ) ipv4_header->ihl * 4 ) {
    return;
  }

  // Parses IPv4 header
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

  packet_info0->l4_header = ( char * ) packet_info0->l3_header +
    packet_info0->ipv4_ihl * 4;

  return;
}


/**
 * Parses an ICMP header in buf->data and places in buf->user_data.
 */
static void 
parse_icmp( buffer *buf ) {
  assert( buf != NULL );

  packet_info *packet_info0 = buf->user_data;
  void *ptr = packet_info0->l4_header;
  assert( ptr != NULL );

  // Check the length of remained buffer
  size_t length = buf->length -
    ( size_t ) ( ( char * ) ptr - ( char * ) buf->data );
  if ( length < sizeof( icmp_header_t ) ) {
    return;
  }

  // ICMPV4 header
  icmp_header_t *icmp_header = ptr;
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

  return;
};


/**
 * Parses a UDP header in buf->data and places in buf->user_data.
 */
static void
parse_udp( buffer *buf ) {
  assert( buf != NULL );

  packet_info *packet_info0 = buf->user_data;
  void *ptr = packet_info0->l4_header;
  assert( ptr != NULL );

  // Check the length of remained buffer
  size_t length = buf->length -
    ( size_t ) ( ( char * ) ptr - ( char * ) buf->data );
  if ( length < sizeof( udp_header_t ) ) {
    return;
  }

  // UDP header
  udp_header_t *udp_header = ptr;
  packet_info0->udp_src_port = ntohs( udp_header->src_port );
  packet_info0->udp_dst_port = ntohs( udp_header->dst_port );
  packet_info0->udp_len = ntohs( udp_header->len );
  packet_info0->udp_checksum = ntohs( udp_header->csum );

  packet_info0->l4_payload = ( char * )packet_info0->l4_header +
    sizeof( udp_header_t );

  packet_info0->format |= TP_UDP;
  
  return;
};


/**
 * Parses a TCP header in buf->data and places in buf->user_data.
 */
static void
parse_tcp( buffer *buf ) {
  assert( buf != NULL );

  packet_info *packet_info0 = buf->user_data;
  void *ptr = packet_info0->l4_header;
  assert( ptr != NULL );

  // Check the length of remained buffer for a tcp header without options
  size_t length = buf->length -
    ( size_t ) ( ( char * ) ptr - ( char * ) buf->data );
  if ( length < sizeof( tcp_header_t ) ) {
    return;
  }

  // Check the length of remained buffer for a tcp header with options
  tcp_header_t *tcp_header = ptr;
  if ( tcp_header->offset < 5 ) { 
    return;
  }
  if ( length < ( size_t ) tcp_header->offset * 4 ) {
    return;
  }
  
  // TCP header
  packet_info0->tcp_src_port = ntohs( tcp_header->src_port );
  packet_info0->tcp_dst_port = ntohs( tcp_header->dst_port );
  packet_info0->tcp_seq_no = ntohl( tcp_header->seq_no );
  packet_info0->tcp_ack_no = ntohl( tcp_header->ack_no );
  packet_info0->tcp_offset = tcp_header->offset;
  packet_info0->tcp_flags = tcp_header->flags;
  packet_info0->tcp_window = ntohs( tcp_header->window );
  packet_info0->tcp_checksum = ntohs( tcp_header->csum );
  packet_info0->tcp_urgent = ntohs( tcp_header->urgent );

  packet_info0->l4_payload = ( char * ) packet_info0->l4_header + 
    packet_info0->tcp_offset * 4;

  packet_info0->format |= TP_TCP;

  return;
};


/**
 * Parses a packet in buf->data and place in buf->user_data.
 * @param buf Pointer to buffer type structure, user_data element of which points to structure of type packet_info
 * @return bool True on success, else False
 */
bool
parse_packet( buffer *buf ) {
  assert( buf != NULL );
  assert( buf->data != NULL );

  calloc_packet_info( buf );
  if ( buf->user_data == NULL ) {
    error( "Can't alloc memory for packet_info." );
    return false;
  }

  packet_info *packet_info0 = buf->user_data;
  packet_info0->l2_header = buf->data;

  // Parse a L2 header.
  parse_ether( buf );
  
  // Parse a L3 header.
  switch ( packet_info0->eth_type ) {
    case ETH_ETHTYPE_ARP:
      parse_arp( buf );
      break;
      
    case ETH_ETHTYPE_IPV4:
      parse_ipv4( buf );
      break;
      
    default:
      return true;
  }
    
  if ( !( packet_info0->format & NW_IPV4 ) ) {
    // Unknown L3 type
    return true;
  } 
  else if ( ( packet_info0->ipv4_frag_off & IP_OFFMASK ) != 0 ) {
    // The ipv4 packet is fragmented.
    return true;
  }

  // Parse a L4 header.
  switch ( packet_info0->ipv4_protocol ) {
    case IPPROTO_ICMP:
      parse_icmp( buf );
      break;
      
    case IPPROTO_TCP:
      parse_tcp( buf );
      break;
      
    case IPPROTO_UDP:
      parse_udp( buf );
      break;
      
    default:
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
