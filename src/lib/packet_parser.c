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
 * @file packet_parser.c
 * Source file containing functions for handling packets. This file contains functions for 
 * parsing a packet to find its type (whether IPv4, or ARP) and peform checksum if required.
 */

#include <assert.h>
#include <stdint.h>
#include "packet_info.h"
#include "log.h"
#include "wrapper.h"

static bool parse_ether( buffer *buf );
static bool parse_arp( buffer *buf );
static bool parse_ipv4( buffer *buf );
static bool parse_icmp( buffer *buf );
static bool parse_udp( buffer *buf );
static bool parse_tcp( buffer *buf );

#if 0
/**
 * Calculates checksum. Following code snippet explains how 
 * @code
 *   //Calling get_checksum to calculate checksum over ipv4 header, which is stored in a buf pointer
 *   if ( get_checksum( ( uint16_t * ) packet_info( buf )->l3_data.ipv4, ( uint32_t ) hdr_len ) != 0 ){
 *       //Registering "checksum verification error" if calculated checksum is not equal to 0
 *       debug( "Corrupted IPv4 header ( checksum verification error )." );
 *   }
 * @endcode
 * @param pos Pointer of type uint16_t
 * @param size Variable of type uint32_t
 * @return uint16_t Checksum
 */
static uint16_t
get_checksum( uint16_t *pos, uint32_t size ) {
  assert( pos != NULL );

  uint32_t csum = 0;
  for (; 2 <= size; pos++, size -= 2 ) {
    csum += *pos;
  }
  if ( size == 1 ) {
    csum += *( unsigned char * ) pos;
  }
  while ( csum & 0xffff0000 ) {
    csum = ( csum & 0x0000ffff ) + ( csum >> 16 );
  }

  return ( uint16_t ) ~csum;
}
#endif 

/**
 * Validates packet header information contained in structure of type packet_header_info.
 * @param buf Pointer to buffer type structure, user_data element of which points to structure of type packet_header_info
 * @return bool True if packet has valid header, else False
 */
bool
parse_packet( buffer *buf ) {
  assert( buf != NULL );
  assert( buf->data != NULL );

  alloc_packet( buf );
  if ( buf->user_data != NULL ) {
    return false;
  }

  packet_info *pkt_info = ( packet_info * )buf->user_data;

  // parse L2 information.
  if ( !parse_ether( buf ) ) {
    warn( "Failed to parse ethernet header." );
    return false;
  }

  // parse L3 information.
  switch ( pkt_info->eth_type ) {
  case ETH_ETHTYPE_ARP:
    if ( !parse_arp( buf ) ) {
      warn( "Failed to parse an arp packet." );
      return false;
    }
    break;

  case ETH_ETHTYPE_IPV4:
    if ( !parse_ipv4( buf ) ) {
      warn( "Failed to parse IPv4 header." );
      return false;
    }
    
    // parse L4 information.
    switch ( pkt_info->ipv4_protocol ) {
    case IPPROTO_ICMP:
      if ( !parse_icmp( buf ) ) {
        return false;
      }
        
    case IPPROTO_TCP:
      if ( !parse_tcp( buf ) ) {
        return false;
      }

    case IPPROTO_UDP:
      if ( !parse_udp( buf ) ) {
        return false;        
      }
      
    default:
      warn( "" );
      break;
    }
      
    break; // exit from parsing ipv4 header.

    default:
      warn( "" );
      break;
  }

  return true;
}


/**
 * This function is to find if Ethernet header contained in passed buffer is
 * valid Ethernet header or not.
 * @param buf Buffer containing header to verify for being valid Ethernet header
 * @return bool True if buffer contains valid Ethernet header, else False
 */
static bool
parse_ether( buffer *buf ) {
  assert( buf != NULL );
  assert( packet_info( buf )->l2_data.eth != NULL );

  size_t frame_length = buf->length - ( size_t ) ( ( char * ) ( packet_info( buf )->l2_data.l2 ) - ( char * ) buf->data ) - ETH_PREPADLEN;
  ether_header_t *eth = packet_info( buf )->l2_data.eth;
  size_t ether_length = sizeof( ether_header_t ) - ETH_PREPADLEN;
  if ( frame_length < ether_length ) {
    debug( "Frame length is shorter than the length of an Ethernet header ( frame length = %u ).", frame_length );
    return false;
  }

  debug( "Parsing an Ethernet frame "
         "( source mac = %02x:%02x:%02x:%02x:%02x:%02x, "
         "destination mac = %02x:%02x:%02x:%02x:%02x:%02x ).",
         eth->macsa[ 0 ], eth->macsa[ 1 ], eth->macsa[ 2 ],
         eth->macsa[ 3 ], eth->macsa[ 4 ], eth->macsa[ 5 ],
         eth->macda[ 0 ], eth->macda[ 1 ], eth->macda[ 2 ],
         eth->macda[ 3 ], eth->macda[ 4 ], eth->macda[ 5 ] );

  if ( eth->macsa[ 0 ] & 0x01 ) {
    debug( "Source MAC address is multicast or broadcast." );
    return false;
  }

  uint16_t type = ntohs( eth->type );
  vlantag_header_t *vlan_tag = ( void * ) ( eth + 1 );
  uint8_t next_vlan_tags = 0;
  while ( type == ETH_ETHTYPE_TPID ) {
    ether_length += sizeof( vlantag_header_t );
    if ( frame_length < ether_length ) {
      debug( "Too short 802.1Q frame ( frame length = %u ).", frame_length );
      return false;
    }
    type = ntohs( vlan_tag->type );
    vlan_tag++;
    next_vlan_tags++;
    debug( "802.1Q header found ( type = %#x, # of headers = %u ).", type, next_vlan_tags );
  }

  if ( type <= ETH_ETHTYPE_8023 ) {
    snap_header_t *snap = ( snap_header_t * ) vlan_tag;

    // check frame length first
    if ( snap->llc[ 0 ] == 0xaa && snap->llc[ 1 ] == 0xaa ) {
      // LLC header with SNAP
      ether_length += sizeof( snap_header_t );
    }
    else {
      // LLC header without SNAP
      ether_length += offsetof( snap_header_t, oui );
    }
    if ( frame_length < ether_length ) {
      debug( "Too short LLC/SNAP frame ( minimun frame length = %u, frame length = %u ).",
             ether_length, frame_length );
      return false;
    }
    if ( frame_length < ( size_t ) type ) {
      debug( "Frame length is shorter than the length header field value "
             "( frame length = %u, length field value = %u ).", frame_length, type );
      return false;
    }

    // check header field values
    if ( snap->llc[ 0 ] == 0xaa && snap->llc[ 1 ] == 0xaa ) {
      // LLC header with SNAP
      if ( snap->llc[ 2 ] == 0x03 || snap->llc[ 2 ] == 0xf3 || snap->llc[ 2 ] == 0xe3
           || snap->llc[ 2 ] == 0xbf || snap->llc[ 2 ] == 0xaf ) {
        type = ntohs( snap->type );
      }
      else {
        debug( "Unexpected SNAP frame ( length = %u, llc = 0x%02x%02x%02x, oui = 0x%02x%02x%02x, type = %u ).",
               type, snap->llc[ 0 ], snap->llc[ 1 ], snap->llc[ 2 ],
               snap->oui[ 0 ], snap->oui[ 1 ], snap->oui[ 2 ], ntohs( snap->type ) );
        return false;
      }
    }
    else {
      // LLC header without SNAP
      debug( "Unhandled 802.3 frame ( length = %u, llc = 0x%02x%02x%02x ).",
             type, snap->llc[ 0 ], snap->llc[ 1 ], snap->llc[ 2 ] );
      type = ETH_ETHTYPE_UKNOWN;
    }
  }

  packet_info( buf )->nvtags = next_vlan_tags;
  if ( next_vlan_tags > 0 ) {
    packet_info( buf )->vtag = ( void * ) ( eth + 1 );
    debug( "802.1Q header found ( tci = %#x, type = %#x ).",
           ntohs( packet_info( buf )->vtag->tci ), ntohs( packet_info( buf )->vtag->type ) );
  }
  else {
    packet_info( buf )->vtag = NULL;
    debug( "No 802.1Q header found." );
  }

  debug( "Ethernet type = %#x.", type );

  packet_info( buf )->ethtype = type;
  packet_info( buf )->l3_data.l3 = ( char * ) packet_info( buf )->l2_data.l2 + ETH_PREPADLEN + ether_length;

  return true;
}

/**
 */
static bool 
parse_arp( buffer *buf ) {
  assert( buf != NULL );
  return true;
};


/**
 * This function parses the IPv4 packet for being valid IPv4 packet.
 * @param buf Pointer to buffer containing IPv4 packet
 * @return bool True if the buffer contains valid IPv4 packet, else False
 */
static bool
parse_ipv4( buffer *buf ) {
  assert( buf != NULL );
  return true;
}


/**
 */
static bool 
parse_icmp( buffer *buf ) {
  assert( buf != NULL );
  return true;
};


/**
 */
static bool 
parse_udp( buffer *buf ) {
  assert( buf != NULL );
  return true;
};


/**
 */
static bool 
parse_tcp( buffer *buf ) {
  assert( buf != NULL );
  return true;
};


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
