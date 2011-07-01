/*
 * Unit tests for arp functions and macros.
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
#include "buffer.h"
#include "checks.h"
#include "cmockery_trema.h"
#include "packet_info.h"


/*
 * add padding data to arp header size forcibly to be able to adapt
 * itself to the minimum size of the ethernet packet (= 46 byte).
 */
const unsigned int padding_size = 46 - sizeof( arp_header_t );


/********************************************************************************
 * Setup and teardown function.
 ********************************************************************************/

static buffer *
setup_dummy_arp_packet() {
  assert_true( sizeof( arp_header_t ) < 46 );

  buffer *buffer = alloc_buffer_with_length( sizeof( arp_header_t ) + padding_size );
  alloc_packet( buffer );
  append_back_buffer( buffer, sizeof( arp_header_t ) + padding_size );
  packet_info( buffer )->l3_data.l3 = buffer->data;
  arp_header_t *arp = ( arp_header_t * ) ( packet_info( buffer )->l3_data.arp );

  arp->ar_hrd = htons( ARPHRD_ETHER );
  arp->ar_pro = htons( ETH_ETHTYPE_IPV4 );
  arp->ar_hln = ETH_ADDRLEN;
  arp->ar_pln = IPV4_ADDRLEN;
  arp->ar_op = htons( ARPOP_REPLY );
  arp->sha[ 0 ] = 0x00;
  arp->sha[ 1 ] = 0x01;
  arp->sha[ 2 ] = 0x02;
  arp->sha[ 3 ] = 0x03;
  arp->sha[ 4 ] = 0x04;
  arp->sha[ 5 ] = 0x05;
  arp->sip = htonl( 0x01020304 );
  arp->tha[ 0 ] = 0x06;
  arp->tha[ 1 ] = 0x07;
  arp->tha[ 2 ] = 0x08;
  arp->tha[ 3 ] = 0x09;
  arp->tha[ 4 ] = 0x0a;
  arp->tha[ 5 ] = 0x0b;
  arp->tip = htonl( 0x05060708 );

  return buffer;
}


/********************************************************************************
 * Tests.
 ********************************************************************************/

static void
test_valid_arp_packet_fails_if_packet_size_is_Less_than_arp_data_size() {
  buffer *buffer = setup_dummy_arp_packet( );
  buffer->length = sizeof( arp_header_t ) - 1;

  assert_int_equal( valid_arp_packet( buffer ), false );

  free_packet( buffer );
}


static void
test_valid_arp_packet_fails_if_hw_type_is_no_ethernet_type() {
  buffer *buffer = setup_dummy_arp_packet( );
  packet_info( buffer )->l3_data.arp->ar_hrd = ARPHRD_ETHER;

  assert_int_equal( valid_arp_packet( buffer ), false );

  free_packet( buffer );
}


static void
test_valid_arp_packet_fails_if_protocl_type_is_no_ip_type() {
  buffer *buffer = setup_dummy_arp_packet( );
  packet_info( buffer )->l3_data.arp->ar_pro = ETH_ETHTYPE_IPV4;

  assert_int_equal( valid_arp_packet( buffer ), false );

  free_packet( buffer );
}


static void
test_valid_arp_packet_fails_if_hw_size_is_no_mac_address_size() {
  buffer *buffer = setup_dummy_arp_packet( );
  packet_info( buffer )->l3_data.arp->ar_hln = ( uint8_t ) ( ETH_ADDRLEN + padding_size );

  assert_int_equal( valid_arp_packet( buffer ), false );

  free_packet( buffer );
}


static void
test_valid_arp_packet_fails_if_protocl_size_is_no_ip_address_size() {
  buffer *buffer = setup_dummy_arp_packet( );
  packet_info( buffer )->l3_data.arp->ar_pln = ( uint8_t ) ( IPV4_ADDRLEN + padding_size );

  assert_int_equal( valid_arp_packet( buffer ), false );

  free_packet( buffer );
}


static void
test_valid_arp_packet_fails_if_operation_is_no_request_reply() {
  buffer *buffer = setup_dummy_arp_packet( );
  packet_info( buffer )->l3_data.arp->ar_op = ARPOP_REQUEST;

  assert_int_equal( valid_arp_packet( buffer ), false );

  free_packet( buffer );
}


static void
test_arp_request_succeeds() {
  buffer *buffer = setup_dummy_arp_packet( );
  packet_info( buffer )->l3_data.arp->ar_op = htons( ARPOP_REQUEST );

  assert_int_equal( valid_arp_packet( buffer ), true );

  free_packet( buffer );
}


static void
test_arp_reply_succeeds() {
  buffer *buffer = setup_dummy_arp_packet( );
  packet_info( buffer )->l3_data.arp->ar_op = htons( ARPOP_REPLY );

  assert_int_equal( valid_arp_packet( buffer ), true );

  free_packet( buffer );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  UnitTest tests[] = {
    unit_test( test_arp_request_succeeds ),
    unit_test( test_arp_reply_succeeds ),
    unit_test( test_valid_arp_packet_fails_if_packet_size_is_Less_than_arp_data_size ),
    unit_test( test_valid_arp_packet_fails_if_hw_type_is_no_ethernet_type ),
    unit_test( test_valid_arp_packet_fails_if_protocl_type_is_no_ip_type ),
    unit_test( test_valid_arp_packet_fails_if_hw_size_is_no_mac_address_size ),
    unit_test( test_valid_arp_packet_fails_if_protocl_size_is_no_ip_address_size ),
    unit_test( test_valid_arp_packet_fails_if_operation_is_no_request_reply ),
  };
  stub_logger();
  setup_leak_detector();
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
