/*
 * Unit tests for utility functions.
 *
 * Copyright (C) 2008-2013 NEC Corporation
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


#include <arpa/inet.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmockery_trema.h"
#include "log.h"
#include "ipv4.h"
#include "trema_wrapper.h"
#include "utility.h"
#include "wrapper.h"

/********************************************************************************
 * Setup and teardown
 ********************************************************************************/

static void ( *original_critical )( const char *format, ... );

static void
mock_critical( const char *format, ... ) {
  char output[ 256 ];
  va_list args;
  va_start( args, format );
  vsprintf( output, format, args );
  va_end( args );
  check_expected( output );
}


static void ( *original_abort )( void );

static void
stub_abort() {
  // Do nothing.
}


static void
setup() {
  original_critical = critical;
  critical = mock_critical;

  original_abort = abort;
  trema_abort = stub_abort;
}


static void
teardown() {
  critical = original_critical;
  trema_abort = original_abort;
}


/********************************************************************************
 * Tests.
 ********************************************************************************/

static void
test_die() {
  expect_string( mock_critical, output, "Bye!" );
  die( "Bye!" );
}


static void
test_hash_core() {
  unsigned char bin1[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
  unsigned char bin2[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

  assert_true( hash_core( bin1, sizeof( bin1 ) ) == hash_core( bin2, sizeof( bin2 ) ) );
}


static void
test_compare_string() {
  char hello1[] = "Hello World";
  char hello2[] = "Hello World";
  char bye[] = "Bye World";

  assert_true( compare_string( hello1, hello2 ) );
  assert_false( compare_string( hello1, bye ) );
}


static void
test_hash_string() {
  char hello1[] = "Hello World";
  char hello2[] = "Hello World";

  assert_true( hash_string( hello1 ) == hash_string( hello2 ) );
}


static void
test_compare_uint32() {
  uint32_t x = 123;
  uint32_t y = 123;
  uint32_t z = 321;

  assert_true( compare_uint32( ( void * ) &x, ( void * ) &y ) );
  assert_false( compare_uint32( ( void * ) &x, ( void * ) &z ) );
}


static void
test_hash_uint32() {
  uint32_t key = 123;

  assert_int_equal( 123, hash_uint32( ( void * ) &key ) );
}


static void
test_compare_datapath_id() {
  uint64_t x = 123;
  uint64_t y = 123;
  uint64_t z = 321;

  assert_true( compare_datapath_id( ( void * ) &x, ( void * ) &y ) );
  assert_false( compare_datapath_id( ( void * ) &x, ( void * ) &z ) );
}


static void
test_hash_datapath_id() {
  uint64_t x = 123;
  uint64_t y = 123;
  uint64_t z = 321;

  assert_true( hash_datapath_id( ( void * ) &x ) == hash_datapath_id( ( void * ) &y ) );
  assert_true( hash_datapath_id( ( void * ) &x ) != hash_datapath_id( ( void * ) &z ) );
}


static void
test_compare_mac() {
  uint8_t mac1[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  uint8_t mac2[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

  assert_true( compare_mac( mac1, mac1 ) );
  assert_false( compare_mac( mac1, mac2 ) );
}


static void
test_hash_mac() {
  uint8_t mac1[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  uint8_t mac2[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

  assert_true( hash_mac( mac1 ) == hash_mac( mac2 ) );
}


static void
test_mac_to_uint64() {
  uint8_t mac1[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  uint8_t mac2[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

  assert_true( mac_to_uint64( mac1 ) == 281474976710655ULL );
  assert_true( mac_to_uint64( mac2 ) == 0 );
}


static void
test_string_to_datapath_id() {
  uint64_t datapath_id;
  uint64_t expected_datapath_id = 18446744073709551615ULL;

  assert_true( string_to_datapath_id( "18446744073709551615", &datapath_id ) );
  assert_memory_equal( &datapath_id, &expected_datapath_id, sizeof( uint64_t ) );

  assert_false( string_to_datapath_id( "INVALID DATAPATH ID", &datapath_id ) );
}


static void
test_wildcards_to_string_with_exact_match() {
  char wildcards_str[ 128 ];
  char expected_wildcards_str[] = "none";
  uint32_t wildcards = 0;
  assert_true( wildcards_to_string( wildcards, wildcards_str, sizeof( wildcards_str ) ) );
  assert_string_equal( wildcards_str, expected_wildcards_str );

  wildcards = UINT32_MAX & ~( ( uint32_t ) OFPFW_ALL );
  assert_true( wildcards_to_string( wildcards, wildcards_str, sizeof( wildcards_str ) ) );
  assert_string_equal( wildcards_str, expected_wildcards_str );
}


static void
test_wildcards_to_string_with_all_wildcards() {
  char wildcards_str[ 128 ];
  char expected_wildcards_str[] = "all";
  uint32_t wildcards = OFPFW_ALL;
  assert_true( wildcards_to_string( wildcards, wildcards_str, sizeof( wildcards_str ) ) );
  assert_string_equal( wildcards_str, expected_wildcards_str );

  wildcards = UINT32_MAX;
  assert_true( wildcards_to_string( wildcards, wildcards_str, sizeof( wildcards_str ) ) );
  assert_string_equal( wildcards_str, expected_wildcards_str );
}


static void
test_wildcards_to_string_with_all_wildcards_except_in_port() {
  char wildcards_str[ 128 ];
  char expected_wildcards_str[] = "dl_src|dl_dst|dl_type|dl_vlan|dl_vlan_pcp|nw_proto|nw_tos|nw_src(63)|nw_dst(63)|tp_src|tp_dst";
  uint32_t wildcards = OFPFW_ALL & ~OFPFW_IN_PORT;
  assert_true( wildcards_to_string( wildcards, wildcards_str, sizeof( wildcards_str ) ) );
  assert_string_equal( wildcards_str, expected_wildcards_str );
}


static void
test_wildcards_to_string_with_all_wildcards_except_dl_addrs() {
  char wildcards_str[ 128 ];
  char expected_wildcards_str[] = "in_port|dl_type|dl_vlan|dl_vlan_pcp|nw_proto|nw_tos|nw_src(63)|nw_dst(63)|tp_src|tp_dst";
  uint32_t wildcards = OFPFW_ALL & ~OFPFW_DL_SRC & ~OFPFW_DL_DST;
  assert_true( wildcards_to_string( wildcards, wildcards_str, sizeof( wildcards_str ) ) );
  assert_string_equal( wildcards_str, expected_wildcards_str );
}


static void
test_wildcards_to_string_fails_with_insufficient_buffer() {
  char wildcards_str[ 1 ];
  uint32_t wildcards = 0;
  assert_false( wildcards_to_string( wildcards, wildcards_str, sizeof( wildcards_str ) ) );
}


static void
test_match_to_string() {
  char match_str[ 256 ];
  char expected_match_str[] = "wildcards = 0(none), in_port = 1, dl_src = 01:02:03:04:05:07, dl_dst = 08:09:0a:0b:0c:0d, dl_vlan = 0x1, dl_vlan_pcp = 0x1, dl_type = 0x800, nw_tos = 1, nw_proto = 6, nw_src = 10.9.8.7/32, nw_dst = 6.5.4.3/32, tp_src = 1024, tp_dst = 2048";
  struct ofp_match match = { 0, 1,
                             { 0x01, 0x02, 0x03, 0x04, 0x05, 0x07 },
                             { 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d },
                             1, 1, { 0 }, 0x800, 1, 0x6, { 0, 0 },
                             0x0a090807, 0x06050403, 1024, 2048 };

  assert_true( match_to_string( &match, match_str, sizeof( match_str ) ) );
  assert_string_equal( match_str, expected_match_str );
}


static void
test_match_to_string_fails_with_insufficient_buffer() {
  char match_str[ 1 ];
  struct ofp_match match;

  assert_false( match_to_string( &match, match_str, sizeof( match_str ) ) );
}


static void
test_phy_port_to_string() {
  char phy_port_str[ 256 ];
  char expected_phy_port_str[] = "port_no = 1, hw_addr = 01:02:03:04:05:06, name = GbE 0/1, config = 0x1, state = 0x1, curr = 0x4a0, advertised = 0x6bf, supported = 0x6bf, peer = 0x6bf";
  uint8_t hw_addr[ OFP_ETH_ALEN ] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
  uint32_t port_features = ( OFPPF_10MB_HD | OFPPF_10MB_FD | OFPPF_100MB_HD |
                             OFPPF_100MB_FD | OFPPF_1GB_HD | OFPPF_1GB_FD |
                             OFPPF_COPPER |  OFPPF_AUTONEG | OFPPF_PAUSE );
  struct ofp_phy_port phy_port;

  phy_port.port_no = 1;
  memcpy( phy_port.hw_addr, hw_addr, sizeof( phy_port.hw_addr ) );
  memset( phy_port.name, '\0', OFP_MAX_PORT_NAME_LEN );
  strncpy( phy_port.name, "GbE 0/1", OFP_MAX_PORT_NAME_LEN );
  phy_port.config = OFPPC_PORT_DOWN;
  phy_port.state = OFPPS_LINK_DOWN;
  phy_port.curr = ( OFPPF_1GB_FD | OFPPF_COPPER | OFPPF_PAUSE );
  phy_port.advertised = port_features;
  phy_port.supported = port_features;
  phy_port.peer = port_features;

  assert_true( phy_port_to_string( &phy_port, phy_port_str, sizeof( phy_port_str ) ) );
  assert_string_equal( phy_port_str, expected_phy_port_str );
}


static void
test_phy_port_to_string_fails_with_insufficient_buffer() {
  char phy_port_str[ 1 ];
  struct ofp_phy_port phy_port;

  assert_false( phy_port_to_string( &phy_port, phy_port_str, sizeof( phy_port_str ) ) );
}


static void
test_actions_to_string_with_action_output() {
  char str[ 128 ];
  char expected_str[] = "output: port=1 max_len=65535";
  struct ofp_action_output action;

  action.type = OFPAT_OUTPUT;
  action.len = sizeof( struct ofp_action_output );
  action.port = 1;
  action.max_len = 65535;

  assert_true( actions_to_string( ( const struct ofp_action_header * ) &action, action.len, str, sizeof( str ) ) );
  assert_string_equal( str, expected_str );
}


static void
test_actions_to_string_with_action_set_vlan_vid() {
  char str[ 128 ];
  char expected_str[] = "set_vlan_vid: vlan_vid=0xbeef";
  struct ofp_action_vlan_vid action;

  action.type = OFPAT_SET_VLAN_VID;
  action.len = sizeof( struct ofp_action_vlan_vid );
  action.vlan_vid = 0xbeef;

  assert_true( actions_to_string( ( const struct ofp_action_header * ) &action, action.len, str, sizeof( str ) ) );
  assert_string_equal( str, expected_str );
}


static void
test_actions_to_string_with_action_set_vlan_pcp() {
  char str[ 128 ];
  char expected_str[] = "set_vlan_pcp: vlan_pcp=0xf0";
  struct ofp_action_vlan_pcp action;

  action.type = OFPAT_SET_VLAN_PCP;
  action.len = sizeof( struct ofp_action_vlan_pcp );
  action.vlan_pcp = 0xf0;

  assert_true( actions_to_string( ( const struct ofp_action_header * ) &action, action.len, str, sizeof( str ) ) );
  assert_string_equal( str, expected_str );
}


static void
test_actions_to_string_with_action_strip_vlan() {
  char str[ 128 ];
  char expected_str[] = "strip_vlan";
  struct ofp_action_header action;

  action.type = OFPAT_STRIP_VLAN;
  action.len = sizeof( struct ofp_action_header );

  assert_true( actions_to_string( ( const struct ofp_action_header * ) &action, action.len, str, sizeof( str ) ) );
  assert_string_equal( str, expected_str );
}


static void
test_actions_to_string_with_action_set_dl_src() {
  char str[ 128 ];
  char expected_str[] = "set_dl_src: dl_addr=01:02:03:04:05:06";
  struct ofp_action_dl_addr action;

  action.type = OFPAT_SET_DL_SRC;
  action.len = sizeof( struct ofp_action_dl_addr );
  action.dl_addr[ 0 ] = 0x01;
  action.dl_addr[ 1 ] = 0x02;
  action.dl_addr[ 2 ] = 0x03;
  action.dl_addr[ 3 ] = 0x04;
  action.dl_addr[ 4 ] = 0x05;
  action.dl_addr[ 5 ] = 0x06;

  assert_true( actions_to_string( ( const struct ofp_action_header * ) &action, action.len, str, sizeof( str ) ) );
  assert_string_equal( str, expected_str );
}


static void
test_actions_to_string_with_action_set_dl_dst() {
  char str[ 128 ];
  char expected_str[] = "set_dl_dst: dl_addr=01:02:03:04:05:06";
  struct ofp_action_dl_addr action;

  action.type = OFPAT_SET_DL_DST;
  action.len = sizeof( struct ofp_action_dl_addr );
  action.dl_addr[ 0 ] = 0x01;
  action.dl_addr[ 1 ] = 0x02;
  action.dl_addr[ 2 ] = 0x03;
  action.dl_addr[ 3 ] = 0x04;
  action.dl_addr[ 4 ] = 0x05;
  action.dl_addr[ 5 ] = 0x06;

  assert_true( actions_to_string( ( const struct ofp_action_header * ) &action, action.len, str, sizeof( str ) ) );
  assert_string_equal( str, expected_str );
}


static void
test_actions_to_string_with_action_set_nw_src() {
  char str[ 128 ];
  char expected_str[] = "set_nw_src: nw_addr=10.0.0.1";
  struct ofp_action_nw_addr action;

  action.type = OFPAT_SET_NW_SRC;
  action.len = sizeof( struct ofp_action_nw_addr );
  action.nw_addr = 0x0a000001;

  assert_true( actions_to_string( ( const struct ofp_action_header * ) &action, action.len, str, sizeof( str ) ) );
  assert_string_equal( str, expected_str );
}


static void
test_actions_to_string_with_action_set_nw_dst() {
  char str[ 128 ];
  char expected_str[] = "set_nw_dst: nw_addr=10.0.0.1";
  struct ofp_action_nw_addr action;

  action.type = OFPAT_SET_NW_DST;
  action.len = sizeof( struct ofp_action_nw_addr );
  action.nw_addr = 0x0a000001;

  assert_true( actions_to_string( ( const struct ofp_action_header * ) &action, action.len, str, sizeof( str ) ) );
  assert_string_equal( str, expected_str );
}


static void
test_actions_to_string_with_action_set_nw_tos() {
  char str[ 128 ];
  char expected_str[] = "set_nw_tos: nw_tos=0x3";
  struct ofp_action_nw_tos action;

  action.type = OFPAT_SET_NW_TOS;
  action.len = sizeof( struct ofp_action_nw_tos );
  action.nw_tos = 0x3;

  assert_true( actions_to_string( ( const struct ofp_action_header * ) &action, action.len, str, sizeof( str ) ) );
  assert_string_equal( str, expected_str );
}


static void
test_actions_to_string_with_action_set_tp_src() {
  char str[ 128 ];
  char expected_str[] = "set_tp_src: tp_port=1024";
  struct ofp_action_tp_port action;

  action.type = OFPAT_SET_TP_SRC;
  action.len = sizeof( struct ofp_action_tp_port );
  action.tp_port = 1024;

  assert_true( actions_to_string( ( const struct ofp_action_header * ) &action, action.len, str, sizeof( str ) ) );
  assert_string_equal( str, expected_str );
}


static void
test_actions_to_string_with_action_set_tp_dst() {
  char str[ 128 ];
  char expected_str[] = "set_tp_dst: tp_port=1024";
  struct ofp_action_tp_port action;

  action.type = OFPAT_SET_TP_DST;
  action.len = sizeof( struct ofp_action_tp_port );
  action.tp_port = 1024;

  assert_true( actions_to_string( ( const struct ofp_action_header * ) &action, action.len, str, sizeof( str ) ) );
  assert_string_equal( str, expected_str );
}


static void
test_actions_to_string_with_action_enqueue() {
  char str[ 128 ];
  char expected_str[] = "enqueue: port=16 queue_id=3";
  struct ofp_action_enqueue action;

  action.type = OFPAT_ENQUEUE;
  action.len = sizeof( struct ofp_action_enqueue );
  action.port = 16;
  action.queue_id = 3;

  assert_true( actions_to_string( ( const struct ofp_action_header * ) &action, action.len, str, sizeof( str ) ) );
  assert_string_equal( str, expected_str );
}


static void
test_actions_to_string_with_action_vendor() {
  char str[ 128 ];
  char expected_str[] = "vendor: vendor=0xdeadbeef";
  struct ofp_action_vendor_header action;

  action.type = OFPAT_VENDOR;
  action.len = sizeof( struct ofp_action_vendor_header );
  action.vendor = 0xdeadbeef;

  assert_true( actions_to_string( ( const struct ofp_action_header * ) &action, action.len, str, sizeof( str ) ) );
  assert_string_equal( str, expected_str );
}


static void
test_actions_to_string_with_undefined_action() {
  char str[ 128 ];
  char expected_str[] = "undefined: type=0xcafe";
  struct ofp_action_header action;

  action.type = 0xcafe;
  action.len = sizeof( struct ofp_action_header );

  assert_true( actions_to_string( ( const struct ofp_action_header * ) &action, action.len, str, sizeof( str ) ) );
  assert_string_equal( str, expected_str );
}


static void
test_actions_to_string_with_multiple_actions() {
  char str[ 128 ];
  char expected_str[] = "output: port=1 max_len=65535, set_vlan_vid: vlan_vid=0xbeef";
  uint16_t actions_length = sizeof( struct ofp_action_output ) + sizeof( struct ofp_action_vlan_vid );
  void *actions = malloc( actions_length );
  memset( actions, 0, actions_length );
  struct ofp_action_output *output = actions;
  struct ofp_action_vlan_vid *vlan_vid = ( struct ofp_action_vlan_vid * ) ( ( char * ) actions + sizeof( struct ofp_action_output ) );

  output->type = OFPAT_OUTPUT;
  output->len = sizeof( struct ofp_action_output );
  output->port = 1;
  output->max_len = 65535;
  vlan_vid->type = OFPAT_SET_VLAN_VID;
  vlan_vid->len = sizeof( struct ofp_action_vlan_vid );
  vlan_vid->vlan_vid = 0xbeef;

  assert_true( actions_to_string( ( const struct ofp_action_header * ) actions, actions_length, str, sizeof( str ) ) );
  assert_string_equal( str, expected_str );
  free( actions );
}


static void
test_actions_to_string_fails_with_insufficient_buffer() {
  char str[ 1 ];
  struct ofp_action_output action;

  action.type = OFPAT_OUTPUT;
  action.len = sizeof( struct ofp_action_output );
  action.port = 1;
  action.max_len = 65535;

  assert_false( actions_to_string( ( const struct ofp_action_header * ) &action, action.len, str, sizeof( str ) ) );
}


static void
test_get_checksum_udp_packet() {
  ipv4_header_t ipv4_header;

  // Create a test packet.
  memset( &ipv4_header, 0, sizeof( ipv4_header ) );
  ipv4_header.version = 4;
  ipv4_header.ihl = 5;
  ipv4_header.tos = 0;
  ipv4_header.tot_len = htons( 0x004c );
  ipv4_header.id = htons( 0x48d8 );
  ipv4_header.frag_off = htons( 0 );
  ipv4_header.ttl = 0x80;
  ipv4_header.protocol = 0x11;
  ipv4_header.csum = 0;
  ipv4_header.saddr = htonl( 0x0a3835af );
  ipv4_header.daddr = htonl( 0x0a3837ff );

  uint16_t checksum = get_checksum( ( uint16_t * ) &ipv4_header,
                                    sizeof( ipv4_header ) );
  assert_int_equal( checksum, 0xab6f );
}


static void
test_get_checksum_icmp_packet() {
  ipv4_header_t ipv4_header;

  // Create a test packet.
  memset( &ipv4_header, 0, sizeof( ipv4_header ) );
  ipv4_header.version = 4;
  ipv4_header.ihl = 5;
  ipv4_header.tos = 0;
  ipv4_header.tot_len = htons( 0x0054 );
  ipv4_header.id = htons( 0xaec3 );
  ipv4_header.frag_off = htons( 0 );
  ipv4_header.ttl = 0x40;
  ipv4_header.protocol = 0x01;
  ipv4_header.csum = 0;
  ipv4_header.saddr = htonl( 0xc0a8642b );
  ipv4_header.daddr = htonl( 0xc0a8642c );

  uint16_t checksum = get_checksum( ( uint16_t * ) &ipv4_header,
                                    sizeof( ipv4_header ) );
  assert_int_equal( checksum, 0x3d82 );
}


static void
test_xfree_data() {
  void *mem = xmalloc( 123 );
  xfree_data( mem, NULL );
}


static void
test_string_equal() {
  char lhs1[] = "abcd";
  char rhs1[] = "abcd";
  assert_true( string_equal( lhs1, rhs1 ) );

  char lhs2[] = "abcde";
  char rhs2[] = "abcdef";
  assert_false( string_equal( lhs2, rhs2 ) );
}


/********************************************************************************
 * Run tests.
 ********************************************************************************/

int
main() {
  const UnitTest tests[] = {
    unit_test_setup_teardown( test_die, setup, teardown ),

    unit_test( test_hash_core ),

    unit_test( test_compare_string ),
    unit_test( test_hash_string ),

    unit_test( test_compare_uint32 ),
    unit_test( test_hash_uint32 ),

    unit_test( test_compare_datapath_id ),
    unit_test( test_hash_datapath_id ),

    unit_test( test_compare_mac ),
    unit_test( test_hash_mac ),
    unit_test( test_mac_to_uint64 ),

    unit_test( test_string_to_datapath_id ),

    unit_test( test_wildcards_to_string_with_exact_match ),
    unit_test( test_wildcards_to_string_with_all_wildcards ),
    unit_test( test_wildcards_to_string_with_all_wildcards_except_in_port ),
    unit_test( test_wildcards_to_string_with_all_wildcards_except_dl_addrs ),
    unit_test( test_wildcards_to_string_fails_with_insufficient_buffer ),

    unit_test( test_match_to_string ),
    unit_test( test_match_to_string_fails_with_insufficient_buffer ),

    unit_test( test_phy_port_to_string ),
    unit_test( test_phy_port_to_string_fails_with_insufficient_buffer ),

    unit_test( test_actions_to_string_with_action_output ),
    unit_test( test_actions_to_string_with_action_set_vlan_vid ),
    unit_test( test_actions_to_string_with_action_set_vlan_pcp ),
    unit_test( test_actions_to_string_with_action_strip_vlan ),
    unit_test( test_actions_to_string_with_action_set_dl_src ),
    unit_test( test_actions_to_string_with_action_set_dl_dst ),
    unit_test( test_actions_to_string_with_action_set_nw_src ),
    unit_test( test_actions_to_string_with_action_set_nw_dst ),
    unit_test( test_actions_to_string_with_action_set_nw_tos ),
    unit_test( test_actions_to_string_with_action_set_tp_src ),
    unit_test( test_actions_to_string_with_action_set_tp_dst ),
    unit_test( test_actions_to_string_with_action_enqueue ),
    unit_test( test_actions_to_string_with_action_vendor ),
    unit_test( test_actions_to_string_with_undefined_action ),
    unit_test( test_actions_to_string_with_multiple_actions ),
    unit_test( test_actions_to_string_fails_with_insufficient_buffer ),

    unit_test( test_get_checksum_udp_packet ),
    unit_test( test_get_checksum_icmp_packet ),

    unit_test( test_xfree_data ),
    unit_test( test_string_equal ),
  };
  setup_leak_detector();
  return run_tests( tests );
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
