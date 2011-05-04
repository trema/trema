/*
 * Unit tests for ether functions and macros.
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


const char macda[] = {
    ( char ) 0xff, ( char ) 0xff, ( char ) 0xff, ( char ) 0xff, ( char ) 0xff, ( char ) 0xff
};
const char macsa[] = {
    ( char ) 0x00, ( char ) 0xd0, ( char ) 0x09, ( char ) 0x20, ( char ) 0x09, ( char ) 0xF7
};
const char sntp_data[] = {
    ( char ) 0xaa, ( char ) 0xaa, ( char ) 0x03, ( char ) 0x00, ( char ) 0x00, ( char ) 0x00, ( char ) 0x08, ( char ) 0x00
};


/********************************************************************************
 * Mock functions.
 ********************************************************************************/

void
mock_die( char *format, ... ) {
  UNUSED( format );
}

void
mock_debug( const char *format, ... ) {
  UNUSED( format );
}


/********************************************************************************
 * Setup and teardown function.
 ********************************************************************************/

static buffer *
setup_dummy_ether_packet( size_t length ) {
  buffer *ether_buffer = alloc_buffer_with_length( length );
  alloc_packet( ether_buffer );
  append_back_buffer( ether_buffer, length );
  packet_info( ether_buffer )->l2_data.eth = ether_buffer->data;

  memcpy( ( char * ) packet_info( ether_buffer )->l2_data.eth->macda, macda, ETH_ADDRLEN );
  memcpy( ( char * ) packet_info( ether_buffer )->l2_data.eth->macsa, macsa, ETH_ADDRLEN );
  packet_info( ether_buffer )->l2_data.eth->type = htons( ETH_ETHTYPE_ARP );

  return ether_buffer;
}


static buffer *
setup_dummy_snap_packet() {
  buffer *snap_buffer = setup_dummy_ether_packet( sizeof( ether_header_t ) + sizeof( snap_header_t ) );

  packet_info( snap_buffer )->l2_data.eth->type = htons( sizeof( ether_header_t ) + sizeof( snap_header_t ) - ETH_PREPADLEN );
  packet_info( snap_buffer )->vtag = ( void * ) ( packet_info( snap_buffer )->l2_data.eth + 1 );
  memcpy( ( char * ) packet_info( snap_buffer )->vtag, sntp_data, sizeof( snap_header_t ) );

  return snap_buffer;
}


static buffer *
setup_dummy_vlan_packet() {
  buffer *vlan_buffer = setup_dummy_ether_packet( sizeof( ether_header_t ) + sizeof( vlantag_header_t ) );

  packet_info( vlan_buffer )->l2_data.eth->type = htons( sizeof( ether_header_t ) + sizeof( vlantag_header_t ) - ETH_PREPADLEN );
  packet_info( vlan_buffer )->l2_data.eth->type = htons( ETH_ETHTYPE_TPID );
  packet_info( vlan_buffer )->vtag = ( void * ) ( packet_info( vlan_buffer )->l2_data.eth + 1 );
  packet_info( vlan_buffer )->vtag->type = htons( ETH_ETHTYPE_IPV4 );

  return vlan_buffer;
}


/********************************************************************************
 * ether frame(DIX) Tests.
 ********************************************************************************/

void
test_parse_ether_succeeds() {
  buffer *buffer = setup_dummy_ether_packet( sizeof( ether_header_t ) );

  assert_int_equal( parse_ether( buffer ), true );

  free_packet( buffer );
}


void
test_parse_ether_fails_if_packet_size_is_short_ethernet_size() {
  buffer *buffer = setup_dummy_ether_packet( sizeof( ether_header_t ) );
  buffer->length = sizeof( ether_header_t ) - ETH_ADDRLEN;

  assert_int_equal( parse_ether( buffer ), false );

  free_packet( buffer );
}


void
test_parse_ether_fails_if_mac_address_is_wrong_mac() {
  buffer *buffer = setup_dummy_ether_packet( sizeof( ether_header_t ) );
  packet_info( buffer )->l2_data.eth->macsa[ 0 ] = 0xff;

  assert_int_equal( parse_ether( buffer ), false );

  free_packet( buffer );
}


/********************************************************************************
 * ether frame(ieee8023:snap) Tests.
 ********************************************************************************/

void
test_parse_ether_snap_succeeds() {
  buffer *buffer = setup_dummy_snap_packet( );

  assert_int_equal( parse_ether( buffer ), true );

  free_packet( buffer );
}


void
test_parse_ether_netbios_succeeds() {
  buffer *buffer = setup_dummy_snap_packet( );
  ( ( snap_header_t * ) packet_info( buffer )->vtag )->llc[ 0 ] = 0xF0;
  ( ( snap_header_t * ) packet_info( buffer )->vtag )->llc[ 1 ] = 0xF0;

  assert_int_equal( parse_ether( buffer ), true );
  assert_int_equal( packet_info( buffer )->ethtype, ETH_ETHTYPE_UKNOWN );

  free_packet( buffer );
}


void
test_parse_ether_not_llc_header_succeeds() {
  buffer *buffer = setup_dummy_snap_packet( );
  ( ( snap_header_t * ) packet_info( buffer )->vtag )->llc[ 0 ] = 0xFF;
  ( ( snap_header_t * ) packet_info( buffer )->vtag )->llc[ 1 ] = 0xFF;

  assert_int_equal( parse_ether( buffer ), true );
  assert_int_equal( packet_info( buffer )->ethtype, ETH_ETHTYPE_UKNOWN );

  free_packet( buffer );
}


void
test_parse_ether_fails_if_type_is_trancated_llc_snap_ieee8023() {
  buffer *buffer = setup_dummy_ether_packet( sizeof( ether_header_t ) );
  packet_info( buffer )->l2_data.eth->type = htons( ETH_ETHTYPE_8023 );

  assert_int_equal( parse_ether( buffer ), false );

  free_packet( buffer );
}


void
test_parse_ether_fails_if_type_is_trancated_payload_ieee8023() {
  buffer *buffer = setup_dummy_snap_packet( );
  packet_info( buffer )->l2_data.eth->type = htons( sizeof( ether_header_t ) + sizeof( snap_header_t ) );

  assert_int_equal( parse_ether( buffer ), false );

  free_packet( buffer );
}


void
test_parse_ether_fails_if_llc_is_unsupported_llc_snap() {
  buffer *buffer = setup_dummy_snap_packet( );
  ( ( snap_header_t * ) packet_info( buffer )->vtag )->llc[ 2 ] = 1;

  assert_int_equal( parse_ether( buffer ), false );

  free_packet( buffer );
}


/********************************************************************************
 * ether frame(ieee8023:vlan) Tests.
 ********************************************************************************/

void
test_parse_ether_vlan_succeeds() {
  buffer *buffer = setup_dummy_vlan_packet( );

  assert_int_equal( parse_ether( buffer ), true );

  free_packet( buffer );
}


void
test_parse_ether_fails_if_type_is_trancated_vlan() {
  buffer *buffer = setup_dummy_ether_packet(sizeof( ether_header_t ) );
  packet_info( buffer )->l2_data.eth->type = htons( ETH_ETHTYPE_TPID );

  assert_int_equal( parse_ether( buffer ), false );

  free_packet( buffer );
}


void
test_parse_ether_fails_if_ether_data_is_NULL() {
  buffer *buffer = setup_dummy_vlan_packet( );
  packet_info( buffer )->l2_data.eth = NULL;

  expect_assert_failure( parse_ether( buffer ) );

  free_packet( buffer );
}


void
test_parse_ether_fails_if_buffer_is_NULL() {
  expect_assert_failure( parse_ether( NULL ) );
}


/********************************************************************************
 * fill_ether_padding Tests.
 ********************************************************************************/

void
test_fill_ether_padding_succeeds_if_length_is_ETH_MINIMUM_LENGTH() {
  buffer *buffer = alloc_buffer_with_length( ( size_t ) ETH_MINIMUM_LENGTH );
  append_back_buffer( buffer, ETH_MINIMUM_LENGTH );

  fill_ether_padding( buffer );

  assert_int_equal ( ( int ) buffer->length, ETH_MINIMUM_LENGTH );

  free_buffer( buffer );
}


void
test_fill_ether_padding_succeeds_if_length_is_less_than_ETH_MINIMUM_LENGTH() {
  buffer *buffer = alloc_buffer_with_length( ( size_t ) ETH_MINIMUM_LENGTH - ETH_FCS_LENGTH - 1 );
  append_back_buffer( buffer, ETH_MINIMUM_LENGTH - ETH_FCS_LENGTH - 1 );

  fill_ether_padding( buffer );

  assert_int_equal ( ( int ) buffer->length, ETH_MINIMUM_LENGTH - ETH_FCS_LENGTH );

  free_buffer( buffer );
}


void
test_fill_ether_padding_fails_if_buffer_is_NULL() {
  expect_assert_failure( fill_ether_padding( NULL ) );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  UnitTest tests[] = {
    unit_test( test_parse_ether_succeeds ),
    unit_test( test_parse_ether_fails_if_packet_size_is_short_ethernet_size ),
    unit_test( test_parse_ether_fails_if_mac_address_is_wrong_mac ),

    unit_test( test_parse_ether_snap_succeeds ),
    unit_test( test_parse_ether_netbios_succeeds ),
    unit_test( test_parse_ether_not_llc_header_succeeds ),
    unit_test( test_parse_ether_fails_if_type_is_trancated_llc_snap_ieee8023 ),
    unit_test( test_parse_ether_fails_if_type_is_trancated_payload_ieee8023 ),
    unit_test( test_parse_ether_fails_if_llc_is_unsupported_llc_snap ),

    unit_test( test_parse_ether_vlan_succeeds ),
    unit_test( test_parse_ether_fails_if_type_is_trancated_vlan ),

    unit_test( test_parse_ether_fails_if_ether_data_is_NULL ),
    unit_test( test_parse_ether_fails_if_buffer_is_NULL ),

    unit_test( test_fill_ether_padding_succeeds_if_length_is_ETH_MINIMUM_LENGTH ),
    unit_test( test_fill_ether_padding_succeeds_if_length_is_less_than_ETH_MINIMUM_LENGTH ),
    unit_test( test_fill_ether_padding_fails_if_buffer_is_NULL ),
  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
