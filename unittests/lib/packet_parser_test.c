/*
 * Unit tests for packet_parser functions and macros.
 *
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
#include <string.h>
#include "checks.h"
#include "cmockery_trema.h"
#include "packet_info.h"
#include "packet_parser.h"
#include "wrapper.h"


const char macda[] = {
    ( char ) 0xff, ( char ) 0xff, ( char ) 0xff, 
    ( char ) 0xff, ( char ) 0xff, ( char ) 0xff
};
const char macsa[] = {
    ( char ) 0x00, ( char ) 0xd0, ( char ) 0x09, 
    ( char ) 0x20, ( char ) 0x09, ( char ) 0xF7
};
const char sntp_data[] = {
    ( char ) 0xaa, ( char ) 0xaa, ( char ) 0x03, ( char ) 0x00, 
    ( char ) 0x00, ( char ) 0x00, ( char ) 0x08, ( char ) 0x00
};


/*
 * add padding data to arp/ipv4 header size forcibly to be able to adapt
 * itself to the minimum size of the ethernet packet (= 46 byte).
 */
const unsigned int ipv4_padding_size = 46 - sizeof( ipv4_header_t );
const unsigned int arp_padding_size = 46 - sizeof( arp_header_t );


/********************************************************************************
 * Setup and teardown function.
 ********************************************************************************/

static buffer *
setup_dummy_ether_packet( size_t length, uint16_t type ) {
  buffer *buf = alloc_buffer_with_length( length );
  alloc_packet( buf );
  append_back_buffer( buf, length );
  packet_info( buf )->l2_data.eth = buf->data;
  packet_info( buf )->ethtype = type;
  ether_header_t *ether = packet_info( buf )->l2_data.eth;
  ether->type = htons( type );

  memcpy( ( char * ) ether->macda, macda, ETH_ADDRLEN );
  memcpy( ( char * ) ether->macsa, macsa, ETH_ADDRLEN );

  packet_info( buf )->l3_data.l3 = ( char * ) packet_info( buf )->l2_data.l2 + sizeof( ether_header_t );
  vlantag_header_t *vtag = ( vlantag_header_t * ) ( ( void * ) ( ether + 1 ) );
  packet_info( buf )->vtag = vtag;

  return buf;
}


static buffer *
setup_dummy_ether_arp_packet() {
  buffer *arp_buffer = setup_dummy_ether_packet( sizeof( ether_header_t ) + sizeof( arp_header_t ) + arp_padding_size, ETH_ETHTYPE_ARP );

  arp_header_t *arp = packet_info( arp_buffer )->l3_data.arp;
  arp->ar_hrd = htons( ARPHRD_ETHER );
  arp->ar_pro = htons( ETH_ETHTYPE_IPV4 );
  arp->ar_hln = ETH_ADDRLEN;
  arp->ar_pln = IPV4_ADDRLEN;
  arp->ar_op = htons( ARPOP_REPLY );

  xfree( arp_buffer->user_data );
  arp_buffer->user_data = NULL;

  remove_front_buffer( arp_buffer, ETH_PREPADLEN );

  return arp_buffer;
}


static buffer *
setup_dummy_ether_ipv4_packet() {
  buffer *ipv4_buffer = setup_dummy_ether_packet( sizeof( ether_header_t ) + sizeof( ipv4_header_t ) + ipv4_padding_size, ETH_ETHTYPE_IPV4 );

  ipv4_header_t *ipv4 = packet_info( ipv4_buffer )->l3_data.ipv4;
  ipv4->version = IPVERSION;
  ipv4->ihl = sizeof( ipv4_header_t ) / 4;
  ipv4->tot_len = htons( sizeof( ipv4_header_t ) );
  ipv4->ttl = 0;
  ipv4->check = 0;
  ipv4->protocol = IPPROTO_UDP;
  ipv4->saddr = htonl( 0xC0A80067 );
  ipv4->daddr = htonl( 0xC0A80036 );
  ipv4->frag_off = htons( 0 );
  ipv4->check = get_checksum( ( uint16_t * ) packet_info( ipv4_buffer )->l3_data.ipv4, sizeof( ipv4_header_t ) );

  xfree( ipv4_buffer->user_data );
  ipv4_buffer->user_data = NULL;

  remove_front_buffer( ipv4_buffer, ETH_PREPADLEN );

  return ipv4_buffer;
}


/********************************************************************************
 * ether arp Tests.
 ********************************************************************************/

static void
test_parse_packet_ether_arp_succeeds() {
  buffer *arp_buffer = setup_dummy_ether_arp_packet( );

  assert_true( parse_packet( arp_buffer ) );

  packet_info packet_info0 = ( packet_info * )buf->user_data;

  free_buffer( arp_buffer );
}


/********************************************************************************
 * ether ipv4 Tests.
 ********************************************************************************/

static void
test_parse_packet_ether_ipv4_succeeds() {
  buffer *ipv4_buffer = setup_dummy_ether_ipv4_packet( );

  assert_true( parse_packet( ipv4_buffer ) );
  

  free_buffer( ipv4_buffer );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  UnitTest tests[] = {
    unit_test( test_parse_packet_ether_arp_succeeds ),

    unit_test( test_parse_packet_ether_ipv4_succeeds ),
  };
  stub_logger();
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
