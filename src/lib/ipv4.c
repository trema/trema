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
 * @brief Functions for handling IPv4 type packets
 * @code
 * // Parses Ethernet header
 * if ( !parse_ipv4( buf ) ) {
 *     warn( "Failed to parse IpV4 header." );
 *     return false;
 * ....
 * @endcode
 */

#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "log.h"
#include "packet_info.h"
#include "packet_parser.h"
#include "wrapper.h"


#ifdef UNIT_TESTING

#ifdef debug
#undef debug
#endif
#define debug mock_debug
void mock_debug( const char *format, ... );

#endif // UNIT_TESTING


/**
 * Validates if the packet header is valid IPv4 header. This is wrapped around
 * by parse_ipv4 function.
 * @param buf Pointer to buffer containing packet header to verify for being ipv4 header
 * @param packet_len Length of packet passed in the buffer
 * @return bool True if the packet header is a valid IPv4 header, else False
 * @see bool parse_ipv4( buffer *buf )
 */
static bool
valid_ipv4_packet_header( buffer *buf, uint32_t packet_len ) {
  if ( ( size_t ) packet_len < sizeof( ipv4_header_t ) ) {
    debug( "Too short IPv4 packet ( length = %u ).", packet_len );
    return false;
  }

  if ( packet_info( buf )->l3_data.ipv4->version != IPVERSION ) {
    debug( "Unsupported IP version ( version = %#x ).",
           packet_info( buf )->l3_data.ipv4->version );
    return false;
  }

  size_t hdr_len = ( size_t ) packet_info( buf )->l3_data.ipv4->ihl * 4;
  if ( hdr_len < sizeof( ipv4_header_t ) ) {
    debug( "Too short IPv4 header length field value ( length = %u ).",
           hdr_len );
    return false;
  }

  if ( verify_checksum( ( uint16_t * ) packet_info( buf )->l3_data.ipv4, ( uint32_t ) hdr_len ) != 0 ) {
    debug( "Corrupted IPv4 header ( checksum verification error )." );
    return false;
  }

  return true;
}


/**
 * Extracts fragment offset and IP packet length of IPv4 packet and validates
 * if packet's length is valid. This is wrapped around by parse_ipv4 function.
 * @param buf Pointer to buffer containing IPv4 packet header
 * @param packet_len Length of packet passed in the buffer
 * @return bool True if the packet length is valid IPv4 packet length, else False
 * @see bool parse_ipv4( buffer *buf )
 */
static bool
valid_ipv4_packet_more_fragments( buffer *buf, uint32_t packet_len ) {
  uint16_t ip_len = ntohs( packet_info( buf )->l3_data.ipv4->tot_len );
  uint32_t frag_offset = ntohs( packet_info( buf )->l3_data.ipv4->frag_off );

  frag_offset = ( frag_offset & IP_OFFMASK ) << 3;

  if ( ( frag_offset > 0 ) && ( ip_len == ( uint16_t ) sizeof( ipv4_header_t ) ) ) {
    debug( "Too short IPv4 fragment ( fragment offset = %u ).", frag_offset );
    return false;
  }

  if ( ( frag_offset + ip_len ) > IP_MAXPACKET ) {
    debug( "Too large IPv4 packet ( total length = %u, fragment offset = %u ).",
           ip_len, frag_offset );
    return false;
  }
  if ( ip_len > packet_len ) {
    debug( "Packet length is shorter than the total length header field value "
           "( total length = %u, packet length = %u ).", ip_len, packet_len );
    return false;
  }

  return true;
}


/**
 * Extracts source and destination address of IPv4 packet and validates them.
 * This is wrapped around by parse_ipv4 function.
 * @param buf Pointer to buffer containing IPv4 packet header
 * @return bool True if source and destination address of IPv4 packet header are valid, else False
 * @see bool parse_ipv4( buffer *buf )
 */
static bool
valid_ipv4_packet_ip_address( buffer *buf ) {
  char addr[ 16 ];
  uint32_t ip_sa = ntohl( packet_info( buf )->l3_data.ipv4->saddr );
  struct in_addr in;

  if ( IPV4_IS_CLASSDE( ip_sa ) || IPV4_IS_LOOPBACK( ip_sa ) || IPV4_IS_LIMITEDBC( ip_sa ) ) {
    in.s_addr = packet_info( buf )->l3_data.ipv4->saddr;
    memset( addr, 0, sizeof( addr ) );
    inet_ntop( AF_INET, &in, addr, sizeof( addr ) );
    debug( "Invalid source IPv4 address ( source address = %s ).", addr );
    return false;
  }

  uint32_t ip_da = ntohl( packet_info( buf )->l3_data.ipv4->daddr );
  if ( IPV4_IS_LOOPBACK( ip_da ) ) {
    in.s_addr = packet_info( buf )->l3_data.ipv4->daddr;
    memset( addr, 0, sizeof( addr ) );
    inet_ntop( AF_INET, &in, addr, sizeof( addr ) );
    debug( "Destination IPv4 address must not be a loopback address ( destination address = %s ).", addr );
    return false;
  }
  if ( ip_sa == ip_da ) {
    in.s_addr = packet_info( buf )->l3_data.ipv4->saddr;
    memset( addr, 0, sizeof( addr ) );
    inet_ntop( AF_INET, &in, addr, sizeof( addr ) );
    debug( "Destination address and source address must not be the same ( address = %s ).", addr );
    return false;
  }

  return true;
}


/**
 * Parses the IPv4 packet for being valid IPv4 packet.
 * @param buf Pointer to buffer containing IPv4 packet
 * @return bool True if the buffer contains valid IPv4 packet, else False
 */
bool
parse_ipv4( buffer *buf ) {
  assert( buf != NULL );
  assert( packet_info( buf )->l3_data.ipv4 != NULL );

  uint32_t packet_len = ( uint32_t ) buf->length - ( uint32_t ) ( ( char * ) ( packet_info( buf )->l3_data.l3 ) -  ( char * ) ( buf->data ) );

  if ( !valid_ipv4_packet_header( buf, packet_len ) ) {
    debug( "Invalid IPv4 header." );
    return false;
  }

  if ( !valid_ipv4_packet_more_fragments( buf, packet_len ) ) {
    debug( "Invalid fragmented IPv4 packet." );
    return false;
  }

  if ( !valid_ipv4_packet_ip_address( buf ) ) {
    debug( "IPv4 packet with an invalid address." );
    return false;
  }

  uint32_t hdr_len = ( uint32_t ) packet_info( buf )->l3_data.ipv4->ihl * 4;
  switch ( packet_info( buf )->l3_data.ipv4->protocol ) {
  case IPPROTO_AH:
    if ( ( hdr_len + 8 ) > packet_len ) {
      debug( "Too short IPv4 packet with an authentication header ( length = %u ).", packet_len );
      return false;
    }
    packet_info( buf )->ipproto = *( uint8_t * ) ( ( char * ) buf->data + hdr_len );
    hdr_len += *( uint8_t * ) ( ( char * ) buf->data + hdr_len + 1 );
    break;
  default:
    packet_info( buf )->ipproto = packet_info( buf )->l3_data.ipv4->protocol;
    break;
  }

  packet_info( buf )->l4_data.l4 = ( char * ) packet_info( buf )->l3_data.l3 + hdr_len;

  return true;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
