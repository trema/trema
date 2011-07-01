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


#include <assert.h>
#include <stdint.h>
#include "packet_info.h"
#include "log.h"
#include "wrapper.h"


uint16_t
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


bool
parse_packet( buffer *buf ) {
  assert( buf != NULL );
  assert( buf->data != NULL );

  alloc_packet( buf );
  packet_info( buf )->l2_data.l2 = ( char * ) buf->data - ETH_PREPADLEN;

  if ( !parse_ether( buf ) ) {
    warn( "Failed to parse ethernet header." );
    return false;
  }

  switch ( packet_info( buf )->ethtype ) {
    case ETH_ETHTYPE_ARP:
      if ( !valid_arp_packet( buf ) ) {
        warn( "Failed to parse ARP header." );
        return false;
      }
      break;
    case ETH_ETHTYPE_IPV4:
      if ( !parse_ipv4( buf ) ) {
        warn( "Failed to parse IPv4 header." );
        return false;
      }
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
