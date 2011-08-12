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
 * @brief Functions for handling ARP type messages
 */

#include <assert.h>
#include "log.h"
#include "packet_info.h"
#include "wrapper.h"


/**
 * This function is to find if a packet contained in passed buffer is a valid
 * ARP Packet or not. It does so by matching the incoming buffer against the ARP
 * Header specifications as defined by RFC 826
 * @param buf Buffer containing packet to verify for being ARP message
 * @return bool True if buffer contains valid ARP message, else False
 */
bool
valid_arp_packet( const buffer *buf ) {
  assert( buf != NULL );
  assert( packet_info( buf )->l3_data.arp != NULL );

  debug( "Validating an ARP packet." );

  arp_header_t *arp = packet_info( buf )->l3_data.arp;
  size_t frame_len = buf->length - ( size_t ) ( ( char * ) ( packet_info( buf )->l3_data.l3 ) - ( ( char * ) buf->data ) );
  if ( frame_len < sizeof( arp_header_t ) ) {
    debug( "Frame length is shorter than the minimum length of ARP header ( length = %u ).", frame_len );
    return false;
  }
  if ( ntohs( arp->ar_hrd ) != ARPHRD_ETHER ) {
    debug( "Unsupported hardware type ( type = %#x ).", ntohs( arp->ar_hrd ) );
    return false;
  }
  if ( ntohs( arp->ar_pro ) != ETH_ETHTYPE_IPV4 ) {
    debug( "Unsupported protocol type ( type = %#x ).", ntohs( arp->ar_pro ) );
    return false;
  }
  if ( arp->ar_hln != ETH_ADDRLEN ) {
    debug( "Invalid hardware length ( length = %u ).", arp->ar_hln );
    return false;
  }
  if ( arp->ar_pln != IPV4_ADDRLEN ) {
    debug( "Invalid protocol length ( length = %u ).", arp->ar_pln );
    return false;
  }
  switch ( ntohs( arp->ar_op ) ) {
  case ARPOP_REQUEST:
  case ARPOP_REPLY:
    break;
  default:
    debug( "Undefined operation ( operation = %#x ).", ntohs( arp->ar_op ) );
    return false;
  }

  debug( "Validation completed successfully." );

  return true;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
