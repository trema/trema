/*
 * Author: Jari Sundell
 *
 * Copyright (C) 2011 axsh Ltd.
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
#include <string.h>
#include <stdint.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include "packet_info.h"
#include "wrapper.h"


static int
write_arp( packet_info *info, void *buffer, int buffer_length ) {
  arp_header_t *arp_header = buffer;

  if ( buffer_length >= ( int ) sizeof( arp_header_t ) ) {
    arp_header->ar_hrd = htons( info->arp_ar_hrd );
    arp_header->ar_pro = htons( info->arp_ar_pro );
    arp_header->ar_hln = info->arp_ar_hln;
    arp_header->ar_pln = info->arp_ar_pln;
    arp_header->ar_op = htons( info->arp_ar_op );
    memcpy( arp_header->sha, info->arp_sha, ETH_ADDRLEN );
    arp_header->sip = htonl( info->arp_spa );
    memcpy( arp_header->tha, info->arp_tha, ETH_ADDRLEN );
    arp_header->tip = htonl( info->arp_tpa );
  }

  return sizeof( arp_header_t );
}


int
write_packet( packet_info *info, char *buffer, int buffer_length ) {
  struct ether_header *ether_header = ( void * ) buffer;

  if ( ( char * ) ( ether_header + 1 ) <= buffer + buffer_length ) {
    memcpy( ether_header->ether_shost, info->eth_macsa, ETH_ADDRLEN );
    memcpy( ether_header->ether_dhost, info->eth_macda, ETH_ADDRLEN );
    ether_header->ether_type = htons( info->eth_type );
  }

  char *ptr = ( char * ) ( ether_header + 1 );
  
  // More stuff, like vlan header, etc...

  switch ( info->format & NW_MASK ) {
  case NW_ARP:
    ptr += write_arp( info, buffer, buffer_length - ( int ) ( ptr - buffer ) );
    break;

  default:
    // Protocol unsupported at this moment.
    return -1;
  }

  if ( ptr - buffer >= 0 && ptr - buffer < ETH_MINIMUM_LENGTH ) {
    memset( ptr, 0, ( size_t ) ( ETH_MINIMUM_LENGTH - ( ptr - buffer ) ) );
    ptr = buffer + ETH_MINIMUM_LENGTH;
  }

  return ( int ) ( ptr - buffer );
}
