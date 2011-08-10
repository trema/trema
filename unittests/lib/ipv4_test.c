/*
 * Unit tests for ipv4 functions and macros.
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


/*
 * add padding data to ipv4 header size forcibly to be able to adapt
 * itself to the minimum size of the ethernet packet (= 46 byte).
 */
const unsigned int padding_size = 46 - sizeof( ipv4_header_t );


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
 * Setup function.
 ********************************************************************************/

static buffer *
setup_dummy_ipv4_packet() {
  assert_true( sizeof( ipv4_header_t ) < 46 );

  buffer *buf = alloc_buffer_with_length( sizeof( ipv4_header_t ) + padding_size );
  alloc_packet( buf );
  append_back_buffer( buf, sizeof( ipv4_header_t ) + padding_size );
  packet_info( buf )->l3_data.ipv4 = buf->data;

  ipv4_header_t *ipv4 = packet_info( buf )->l3_data.ipv4;
  ipv4->version = IPVERSION;
  ipv4->ihl = sizeof( ipv4_header_t ) / 4;
  ipv4->tot_len = htons( sizeof( ipv4_header_t ) );
  ipv4->ttl = 0;
  ipv4->check = 0;
  ipv4->protocol = IPPROTO_UDP;
  ipv4->saddr = htonl( 0xC0A80067 );
  ipv4->daddr = htonl( 0xC0A80036 );
  ipv4->frag_off = htons( 0 );
  ipv4->check = get_checksum( ( uint16_t * ) ipv4, sizeof( ipv4_header_t ) );

  return buf;
}


/********************************************************************************
 * Tests.
 ********************************************************************************/

static void
test_parse_ipv4_without_ah_succeeds() {
  buffer *buf = setup_dummy_ipv4_packet();

  assert_int_equal( parse_ipv4( buf ), true );

  free_buffer( buf );
}


static void
test_parse_ipv4_fails_if_packet_size_is_too_short() {
  buffer *buf = setup_dummy_ipv4_packet();
  buf->length = sizeof( ipv4_header_t ) - 1;

  assert_int_equal( parse_ipv4( buf ), false );

  free_buffer( buf );
}


static void
test_parse_ipv4_fails_if_version_is_not_ipv4() {
  buffer *buf = setup_dummy_ipv4_packet();
  packet_info( buf )->l3_data.ipv4->version = 6;

  assert_int_equal( parse_ipv4( buf ), false );

  free_buffer( buf );
}


static void
test_parse_ipv4_fails_if_ihl_is_too_small() {
  buffer *buf = setup_dummy_ipv4_packet();
  packet_info( buf )->l3_data.ipv4->ihl = 2;

  assert_int_equal( parse_ipv4( buf ), false );

  free_buffer( buf );
}


static void
test_parse_ipv4_fails_if_checksum_has_incorrect_value() {
  buffer *buf = setup_dummy_ipv4_packet();
  packet_info( buf )->l3_data.ipv4->check = 0;

  assert_int_equal( parse_ipv4( buf ), false );

  free_buffer( buf );
}


static void
test_parse_ipv4_fails_if_fragment_does_not_have_any_data() {
  buffer *buf = setup_dummy_ipv4_packet( );
  ipv4_header_t *ipv4 = packet_info( buf )->l3_data.ipv4;
  ipv4->frag_off = htons( 10000 );
  ipv4->check = 0;
  ipv4->check = get_checksum( ( uint16_t * ) ipv4, sizeof( ipv4_header_t ) );

  assert_int_equal( parse_ipv4( buf ), false );

  free_buffer( buf );
}


static void
test_parse_ipv4_fails_if_packet_size_is_too_big() {
  buffer *buf = setup_dummy_ipv4_packet( );
  ipv4_header_t *ipv4 = packet_info( buf )->l3_data.ipv4;
  ipv4->frag_off = htons( sizeof( ipv4_header_t ) );
  ipv4->tot_len = htons( IP_MAXPACKET );
  ipv4->check = 0;
  ipv4->check = get_checksum( ( uint16_t * ) ipv4, sizeof( ipv4_header_t ) );

  assert_int_equal( parse_ipv4( buf ), false );

  free_buffer( buf );
}


static void
test_parse_ipv4_fails_if_tot_len_has_incorrect_value() {
  buffer *buf = setup_dummy_ipv4_packet( );
  ipv4_header_t *ipv4 = packet_info( buf )->l3_data.ipv4;
  ipv4->tot_len = htons( sizeof( ipv4_header_t ) * 3 );
  ipv4->check = 0;
  ipv4->check = get_checksum( ( uint16_t * ) ipv4, sizeof( ipv4_header_t ) );

  assert_int_equal( parse_ipv4( buf ), false );

  free_buffer( buf );
}


static void
test_parse_ipv4_fails_if_source_address_is_illigal() {
  buffer *buf = setup_dummy_ipv4_packet( );
  ipv4_header_t *ipv4 = packet_info( buf )->l3_data.ipv4;
  ipv4->saddr = htonl( 0xFFFFFFFF );
  ipv4->check = 0;
  ipv4->check = get_checksum( ( uint16_t * ) ipv4, sizeof( ipv4_header_t ) );

  assert_int_equal( parse_ipv4( buf ), false );

  free_buffer( buf );
}


static void
test_parse_ipv4_fails_if_destination_address_is_illigal() {
  buffer *buf = setup_dummy_ipv4_packet( );
  ipv4_header_t *ipv4 = packet_info( buf )->l3_data.ipv4;
  ipv4->daddr = htonl( 0x7f000000 );
  ipv4->check = 0;
  ipv4->check = get_checksum( ( uint16_t * ) ipv4, sizeof( ipv4_header_t ) );

  assert_int_equal( parse_ipv4( buf ), false );

  free_buffer( buf );
}


static void
test_parse_ipv4_fails_if_source_and_destination_are_same() {
  buffer *buf = setup_dummy_ipv4_packet( );
  ipv4_header_t *ipv4 = packet_info( buf )->l3_data.ipv4;
  ipv4->saddr = htonl( 0xC0A80037 );
  ipv4->daddr = htonl( 0xC0A80037 );
  ipv4->check = 0;
  ipv4->check = get_checksum( ( uint16_t * ) ipv4, sizeof( ipv4_header_t ) );

  assert_int_equal( parse_ipv4( buf ), false );

  free_buffer( buf );
}


static void
test_parse_ipv4_with_ah_succeeds() {
  buffer *buf = setup_dummy_ipv4_packet( );
  ipv4_header_t *ipv4 = packet_info( buf )->l3_data.ipv4;
  ipv4->protocol = IPPROTO_AH;
  ipv4->tot_len = htons( sizeof( ipv4_header_t ) + 8 );
  ipv4->check = 0;
  ipv4->check = get_checksum( ( uint16_t * ) ipv4, sizeof( ipv4_header_t ) );

  assert_int_equal( parse_ipv4( buf ), true );

  free_buffer( buf );
}


static void
test_parse_ipv4_fails_if_packet_with_ah_is_too_short() {
  buffer *buf = setup_dummy_ipv4_packet( );
  ipv4_header_t *ipv4 = packet_info( buf )->l3_data.ipv4;
  ipv4->protocol = IPPROTO_AH;
  ipv4->ihl = 12;
  ipv4->tot_len = htons( ( uint16_t ) ( ipv4->ihl * 4 + 8 ) );
  ipv4->check = 0;
  ipv4->check = get_checksum( ( uint16_t * ) ipv4, ( uint32_t ) ( ipv4->ihl * 4 ) );

  assert_int_equal( parse_ipv4( buf ), false );

  free_buffer( buf );
}


static void
test_parse_ipv4_fails_if_ipv4_data_is_NULL() {
  buffer *buf = setup_dummy_ipv4_packet( );
  packet_info( buf )->l3_data.ipv4 = NULL;

  expect_assert_failure( parse_ipv4( buf ) );

  free_buffer( buf );
}


static void
test_parse_ipv4_fails_if_buffer_is_NULL() {
  expect_assert_failure( parse_ipv4( NULL ) );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  UnitTest tests[] = {
    unit_test( test_parse_ipv4_without_ah_succeeds ),
    unit_test( test_parse_ipv4_fails_if_packet_size_is_too_short ),
    unit_test( test_parse_ipv4_fails_if_version_is_not_ipv4 ),
    unit_test( test_parse_ipv4_fails_if_ihl_is_too_small ),
    unit_test( test_parse_ipv4_fails_if_checksum_has_incorrect_value ),
    unit_test( test_parse_ipv4_fails_if_fragment_does_not_have_any_data ),
    unit_test( test_parse_ipv4_fails_if_packet_size_is_too_big ),
    unit_test( test_parse_ipv4_fails_if_tot_len_has_incorrect_value ),
    unit_test( test_parse_ipv4_fails_if_source_address_is_illigal ),
    unit_test( test_parse_ipv4_fails_if_destination_address_is_illigal ),
    unit_test( test_parse_ipv4_fails_if_source_and_destination_are_same ),
    unit_test( test_parse_ipv4_with_ah_succeeds ),
    unit_test( test_parse_ipv4_fails_if_packet_with_ah_is_too_short ),
    unit_test( test_parse_ipv4_fails_if_ipv4_data_is_NULL ),
    unit_test( test_parse_ipv4_fails_if_buffer_is_NULL ),
  };
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
